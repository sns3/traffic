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

#include "http-entity-header.h"
#include <ns3/log.h>
#include <ns3/simulator.h>


NS_LOG_COMPONENT_DEFINE ("HttpEntityHeader");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HttpEntityHeader);


HttpEntityHeader::HttpEntityHeader ()
  : m_contentType (NOT_SET),
    m_contentLength (0),
    m_arrivalTime (Simulator::Now ().GetTimeStep ())
{
  NS_LOG_FUNCTION (this);
}


TypeId
HttpEntityHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::HttpEntityHeader")
    .SetParent<Header> ()
    .AddConstructor<HttpEntityHeader> ()
  ;
  return tid;
}


void
HttpEntityHeader::SetContentType (HttpEntityHeader::ContentType_t contentType)
{
  NS_LOG_FUNCTION (this << static_cast<uint16_t> (contentType));
  switch (contentType)
    {
    case NOT_SET:
      m_contentType = 0;
      break;
    case MAIN_OBJECT:
      m_contentType = 1;
      break;
    case EMBEDDED_OBJECT:
      m_contentType = 2;
      break;
    default:
      NS_FATAL_ERROR ("Unknown Content-Type: " << contentType);
      break;
    }
}


HttpEntityHeader::ContentType_t
HttpEntityHeader::GetContentType () const
{
  ContentType_t ret;
  switch (m_contentType)
    {
    case 0:
      ret = NOT_SET;
      break;
    case 1:
      ret = MAIN_OBJECT;
      break;
    case 2:
      ret = EMBEDDED_OBJECT;
      break;
    default:
      NS_FATAL_ERROR ("Unknown Content-Type: " << m_contentType);
      break;
    }
  return ret;
}


void
HttpEntityHeader::SetContentLength (uint32_t contentLength)
{
  NS_LOG_FUNCTION (this << contentLength);
  m_contentLength = contentLength;
}


uint32_t
HttpEntityHeader::GetContentLength () const
{
  return m_contentLength;
}


Time
HttpEntityHeader::GetArrivalTime () const
{
  return TimeStep (m_arrivalTime);
}


uint32_t
HttpEntityHeader::GetStaticSerializedSize ()
{
  return 14;
}


uint32_t
HttpEntityHeader::GetSerializedSize () const
{
  return GetStaticSerializedSize ();
}


void
HttpEntityHeader::Print (std::ostream &os) const
{
  os << "(Content-Type: " << m_contentType
     << " Content-Length: " << m_contentLength
     << " arrivalTime: " << m_arrivalTime << ")";
}


void
HttpEntityHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU16 (m_contentType);
  i.WriteHtonU32 (m_contentLength);
  i.WriteHtonU64 (m_arrivalTime);
}


uint32_t
HttpEntityHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_contentType = i.ReadNtohU16 ();
  m_contentLength = i.ReadNtohU32 ();
  m_arrivalTime = i.ReadNtohU64 ();
  return GetSerializedSize ();
}


TypeId
HttpEntityHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}


} // end of `namespace ns3`

