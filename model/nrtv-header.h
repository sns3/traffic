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

#ifndef NRTV_HEADER_H
#define NRTV_HEADER_H

#include <ns3/header.h>


namespace ns3 {


/**
 * \ingroup traffic
 * \brief Simple packet header for use in NRTV traffic models.
 *
 * The header is 12 bytes in length. There are 4 fields in the header:
 * - frame number (4 bytes, an index starting from 0);
 * - number of frames in the current video session (4 bytes);
 * - slice number (2 bytes, an index starting from 0); and
 * - number of slices in the current frame (2 bytes).
 *
 * The following is the usage example in the case of transmitting a packet.
 * First create a plain header:
 *
 *     NrtvHeader nrtvHeader;
 *
 * Then, specify the underlying values, for example:
 *
 *     nrtvHeader.SetFrameNumber (1);
 *     nrtvHeader.SetNumOfFrames (3000);
 *     nrtvHeader.SetSliceNumber (1);
 *     nrtvHeader.SetNumOfSlices (8);
 *
 * Finally, we can append the header to a packet, e.g.:
 *
 *     Ptr<Packet> packet = Create<Packet> (250);
 *     packet->AddHeader (nrtvHeader);
 *
 * The header is 12 bytes long, so the resulting packet in the above example
 * will become 262 bytes long.
 *
 * Another use case is upon receiving a packet and reading the header content.
 * First of all, make sure the received packet is at least 12 bytes long (we
 * may use GetStaticSerializedSize() to avoid hard-coding a bare figure).
 * Then strip the header from the packet to read its content, for example:
 *
 *     if (packet->GetSize () < NrtvHeader::GetStaticSerializedSize ())
 *       {
 *         // there is definitely no NRTV header in this packet
 *       }
 *     else
 *       {
 *         NrtvHeader nrtvEntity;
 *         packet->RemoveHeader (nrtvEntity);
 *         uint32_t frameNumber = nrtvHeader.GetFrameNumber ();
 *         uint32_t numOfFrames = nrtvHeader.GetNumOfFrames ();
 *         uint32_t sliceNumber = nrtvHeader.GetSliceNumber ();
 *         uint32_t numOfSlices = nrtvHeader.GetNumOfSlices ();
 *       }
 *
 * Instead of Packet::RemoveHeader(), we may use Packet::PeekHeader() if we
 * want to keep the header in the packet.
 *
 * \warning You will get an error if you invoke Packet::RemoveHeader() or
 *          Packet::PeekHeader() on a packet smaller than 12 bytes,
 *
 */
class NrtvHeader : public Header
{
public:
  /// Create a plain new instance of NRTV header.
  NrtvHeader ();

  // Inherited from ObjectBase base class
  static TypeId GetTypeId (void);

  /**
   * \param frameNumber the value for the "frame number" field of this header
   *                    instance
   */
  void SetFrameNumber (uint32_t frameNumber);

  /**
   * \return the current value of the "frame number" field of this header
   *         instance
   */
  uint32_t GetFrameNumber () const;

  /**
   * \param numOfFrames the value for the "number of frames" field of this
   *                    header instance
   */
  void SetNumOfFrames (uint32_t numOfFrames);

  /**
   * \return the current value of the "number of frames" field of this header
   *         instance
   */
  uint32_t GetNumOfFrames () const;

  /**
   * \param sliceNumber the value for the "slice number" field of this header
   *                    instance
   */
  void SetSliceNumber (uint16_t sliceNumber);

  /**
   * \return the current value of the "slice number" field of this header
   *         instance
   */
  uint16_t GetSliceNumber () const;

  /**
   * \param numOfSlice the value for the "number of slices" field of this
   *                   header instance
   */
  void SetNumOfSlices (uint16_t numOfSlices);

  /**
   * \return the current value of the "number of slices" field of this header
   *         instance
   */
  uint16_t GetNumOfSlices () const;

  /**
   * \return the constant length of any instances of this header (6 bytes)
   */
  static uint32_t GetStaticSerializedSize ();

  // Inherited from Header base class
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  // Inherited from ObjectBase base class
  virtual TypeId GetInstanceTypeId () const;

private:
  uint32_t m_frameNumber;
  uint32_t m_numOfFrames;
  uint16_t m_sliceNumber;
  uint16_t m_numOfSlices;

}; // end of `class NrtvHeader`


}  // end of `namespace ns3`


#endif /* NRTV_HEADER_H */
