/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include "nrtv-tcp-client.h"

#include "nrtv-header.h"
#include "nrtv-variables.h"

namespace ns3
{
namespace traffic_extensions
{

// NRTV CLIENT TX BUFFER //////////////////////////////////////////////////////

NS_LOG_COMPONENT_DEFINE("NrtvTcpClientRxBuffer");

NrtvTcpClientRxBuffer::NrtvTcpClientRxBuffer()
    : m_totalBytes(0),
      m_sizeOfVideoSlice(0)
{
    NS_LOG_FUNCTION(this);
}

bool
NrtvTcpClientRxBuffer::IsEmpty() const
{
    if (m_totalBytes == 0)
    {
        NS_ASSERT(m_rxBuffer.size() == 0);
        return true;
    }
    else
    {
        NS_ASSERT(m_rxBuffer.size() > 0);
        return false;
    }
}

bool
NrtvTcpClientRxBuffer::HasVideoSlice() const
{
    NrtvHeader header;
    return m_totalBytes >= (m_sizeOfVideoSlice + header.GetSerializedSize());
}

void
NrtvTcpClientRxBuffer::PushPacket(Ptr<const Packet> packet)
{
    const uint32_t packetSize = packet->GetSize();
    NS_LOG_FUNCTION(this << packet << packetSize);

    if (m_sizeOfVideoSlice == 0)
    {
        // we don't know the size of slice yet
        NrtvHeader header;

        if (IsEmpty())
        {
            m_rxBuffer.push_back(packet->Copy());
        }
        else
        {
            /*
             * The existing packet in the buffer must be a very small packet that
             * contains part of the header. So we combine it together with this
             * packet.
             */
            NS_ASSERT(m_rxBuffer.size() == 1);
            const uint32_t priorRemain = m_rxBuffer.back()->GetSize();
            NS_ASSERT(priorRemain < header.GetSerializedSize());
            NS_LOG_LOGIC(this << " combining a " << priorRemain << "-byte"
                              << " left over from previous slice with " << packetSize
                              << " bytes of packet");
            m_rxBuffer.back()->AddAtEnd(packet);
        }

        NS_ASSERT(m_rxBuffer.size() == 1);
        if (m_rxBuffer.back()->GetSize() >= header.GetSerializedSize())
        {
            m_sizeOfVideoSlice = PeekSliceSize(m_rxBuffer.back());
            NS_LOG_INFO(this << " now expecting a video slice of " << m_sizeOfVideoSlice
                             << " bytes");
        }
        else
        {
            /*
             * Still not enough packets to constitute a 24-byte header (sigh).
             * m_sizeOfVideoSlice stays at zero.
             */
        }
    }
    else
    {
        m_rxBuffer.push_back(packet->Copy());
    }

    // increase the buffer size counter
    m_totalBytes += packetSize;
    NS_LOG_DEBUG(this << " Rx buffer now contains " << m_rxBuffer.size() << " packet(s)" << " ("
                      << m_totalBytes << " bytes)");
}

Ptr<Packet>
NrtvTcpClientRxBuffer::PopVideoSlice()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(!IsEmpty(), "Unable to pop from an empty Rx buffer");
    NS_ASSERT_MSG(HasVideoSlice(), "Not enough packets to constitute a complete video slice");
    NS_ASSERT(PeekSliceSize(m_rxBuffer.front()) == m_sizeOfVideoSlice);

    Ptr<Packet> slice = Create<Packet>();
    NrtvHeader header;
    const uint32_t expectedPacketSize = m_sizeOfVideoSlice + header.GetSerializedSize();
    uint32_t bytesToFetch = expectedPacketSize;

    while (bytesToFetch > 0)
    {
        NS_ASSERT(!m_rxBuffer.empty()); // ensure that front() is not undefined
        const uint32_t packetSize = m_rxBuffer.front()->GetSize();
        NS_LOG_INFO(this << " using a " << packetSize << "-byte packet"
                         << " to compose a video slice" << " (" << bytesToFetch << " bytes to go)");

        if (packetSize <= bytesToFetch)
        {
            // absorb the whole packet
            slice->AddAtEnd(m_rxBuffer.front());
            bytesToFetch -= packetSize;
            m_rxBuffer.pop_front();
        }
        else
        {
            // absorb only the first part of the packet
            slice->AddAtEnd(m_rxBuffer.front()->CreateFragment(0, bytesToFetch));

            // leave the second part in the buffer
            const uint32_t residueBytes = packetSize - bytesToFetch;
            NS_LOG_LOGIC(this << " setting aside " << residueBytes << " bytes"
                              << " for the next video slice");
            m_rxBuffer.front()->RemoveAtStart(bytesToFetch);
            NS_ASSERT(m_rxBuffer.front()->GetSize() == residueBytes);
            bytesToFetch = 0; // this exits the loop
        }

    } // end of `while (bytesToFetch > 0)`

    const uint32_t packetSize = slice->GetSize();
    NS_ASSERT(packetSize == expectedPacketSize);

    // deplete the buffer size counter
    NS_ASSERT(m_totalBytes >= packetSize);
    m_totalBytes -= packetSize;
    NS_LOG_DEBUG(this << " Rx buffer now contains " << m_rxBuffer.size() << " packet(s)" << " ("
                      << m_totalBytes << " bytes)");

    // determine the size of next slice to receive
    if (m_rxBuffer.empty())
    {
        /*
         * The buffer is empty, so we can only tell about the next slice later
         * after the next packet is received.
         */
        m_sizeOfVideoSlice = 0;
    }
    else if (m_rxBuffer.front()->GetSize() >= header.GetSerializedSize())
    {
        // the buffer is not empty and we can read an NRTV header from it
        m_sizeOfVideoSlice = PeekSliceSize(m_rxBuffer.front());
        NS_LOG_INFO(this << " now expecting a video slice of " << m_sizeOfVideoSlice << " bytes");
    }
    else
    {
        // the buffer is not empty, but we cannot read an NRTV header
        m_sizeOfVideoSlice = 0;
        NS_LOG_INFO(this << " cannot read the header yet," << " it must have been split,"
                         << " so the rest will come in the next packet");
    }

    return slice;

} // end of `Ptr<Packet> PopVideoSlice ()`

uint32_t // static
NrtvTcpClientRxBuffer::PeekSliceSize(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(packet << packet->GetSize());
    NrtvHeader nrtvHeader;
    NS_ASSERT_MSG(packet->GetSize() >= nrtvHeader.GetSerializedSize(),
                  "The packet contains no NRTV header");
    packet->PeekHeader(nrtvHeader);
    return nrtvHeader.GetSliceSize();
}

}
} // namespace ns3
