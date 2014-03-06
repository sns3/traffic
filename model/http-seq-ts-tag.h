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

#ifndef HTTP_SEQ_TS_TAG_H
#define HTTP_SEQ_TS_TAG_H

#include <ns3/tag.h>
#include <ns3/nstime.h>


namespace ns3 {

/**
 * \brief Packet tag for HttpClient and HttpServer applications.
 *
 * The serialized tag is 12 bytes in length. The first 32-bit field is a
 * sequence number, then followed by a 64-bit time stamp (automatically
 * filled with the current simulation time).
 *
 * The tag is used for detecting packet loss and calculating packet delay
 * in HTTP traffic.
 */
class HttpSeqTsTag : public Tag
{
public:
  /// Creates an HttpSeqTsTag instance with a zero sequence number.
  HttpSeqTsTag ();

  /**
   * \brief Creates an HttpSeqTsTag instance with the given sequence number.
   * \param seq the sequence number.
   */
  HttpSeqTsTag (uint32_t seq);

  // Inherited from ObjectBase base class.
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  // Inherited from Tag base class.
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer buf) const;
  virtual void Deserialize (TagBuffer buf);
  virtual void Print (std::ostream &os) const;

  /**
   * \param seq the sequence number.
   */
  void SetSeq (uint32_t seq);

  /**
   * \return the sequence number.
   */
  uint32_t GetSeq () const;

  /**
   * \return the time stamp.
   */
  Time GetTs () const;

private:
  uint32_t m_seq;  ///< Sequence number.
  uint64_t m_ts;   ///< Timestamp.

};


} // namespace ns3


#endif /* HTTP_SEQ_TS_TAG_H */
