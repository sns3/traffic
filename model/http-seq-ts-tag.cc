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

#include "http-seq-ts-tag.h"
#include <ns3/log.h>
#include <ns3/simulator.h>

NS_LOG_COMPONENT_DEFINE ("HttpSeqTsTag");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HttpSeqTsTag);

HttpSeqTsTag::HttpSeqTsTag ()
  : Tag (),
    m_seq (0),
    m_ts (Simulator::Now ().GetTimeStep ())
{
  NS_LOG_FUNCTION (this << Simulator::Now ().GetSeconds ());
}


HttpSeqTsTag::HttpSeqTsTag (uint32_t seq)
  : Tag (),
    m_seq (seq),
    m_ts (Simulator::Now ().GetTimeStep ())
{
  NS_LOG_FUNCTION (this << seq << Simulator::Now ().GetSeconds ());
}


TypeId
HttpSeqTsTag::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::HttpSeqTsTag")
    .SetParent<Tag> ()
    .AddConstructor<HttpSeqTsTag> ()
  ;
  return tid;
}


TypeId
HttpSeqTsTag::GetInstanceTypeId () const
{
  return GetTypeId ();
}


uint32_t
HttpSeqTsTag::GetSerializedSize () const
{
  return 4 + 8;
}


void
HttpSeqTsTag::Serialize (TagBuffer buf) const
{
  NS_LOG_FUNCTION (this << &buf);
  buf.WriteU32 (m_seq);
  buf.WriteU64 (m_ts);
}


void
HttpSeqTsTag::Deserialize (TagBuffer buf)
{
  NS_LOG_FUNCTION (this << &buf);
  m_seq = buf.ReadU32 ();
  m_ts = buf.ReadU64 ();
}


void
HttpSeqTsTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(seq=" << m_seq << " time=" << TimeStep (m_ts).GetSeconds () << ")";
}


void
HttpSeqTsTag::SetSeq (uint32_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}


uint32_t
HttpSeqTsTag::GetSeq () const
{
  return m_seq;
}


Time
HttpSeqTsTag::GetTs () const
{
  return TimeStep (m_ts);
}


} // namespace ns3
