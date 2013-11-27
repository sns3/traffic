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

#ifndef HTTP_ENTITY_HEADER_H
#define HTTP_ENTITY_HEADER_H

#include <ns3/header.h>


namespace ns3 {


/**
 * \brief Simplified HTTP entity header, used by HTTP Client and Server
 *        applications.
 *
 * The header is 6 bytes in length. The first 2 bytes are for the Content-Type
 * header, while the remaining 4 bytes are for the Content-Length header.
 *
 * The following is the usage example in the case of transmitting a packet.
 * First create a plain header:
 *
 *     HttpEntityHeader httpEntity;
 *
 * Then, specify the underlying values, for example:
 *
 *     httpEntity.SetContentType (HttpEntityHeader::MAIN_OBJECT);
 *     httpEntity.SetContentLength (350); // in bytes
 *
 * Finally, we can append the header to a packet, e.g.:
 *
 *     Ptr<Packet> packet = Create<Packet> (530);
 *     packet->AddHeader (httpEntityHeader);
 *
 * The header is 6 bytes long, so the resulting packet in the above example
 * will become 536 bytes long.
 *
 * Another use case is upon receiving a packet and reading the header content.
 * First of all, make sure the received packet is at least 6 bytes long (we
 * may use GetStaticSerializedSize() to avoid hard-coding a bare figure).
 * Then strip the header from the packet to read its content, for example:
 *
 *     if (packet->GetSize () < HttpEntityHeader::GetStaticSerializedSize ())
 *       {
 *         // there is definitely no HTTP entity header in this packet
 *       }
 *     else
 *       {
 *         HttpEntityHeader httpEntity;
 *         packet->RemoveHeader (httpEntity);
 *         HttpEntityHeader::ContentType_t contentType = httpEntity.GetContentType ();
 *         uint32_t contentLength = httpEntity.GetContentLength ();
 *       }
 *
 * Instead of Packet::RemoveHeader(), we may use Packet::PeekHeader() if we
 * want to keep the header in the packet.
 *
 * \warning You will get an error if you invoke Packet::RemoveHeader() or
 *          Packet::PeekHeader() on a packet smaller than 6 bytes,
 *
 */
class HttpEntityHeader : public Header
{
public:
  /// Create a plain new instance of HTTP entity header.
  HttpEntityHeader ();

  // Inherited from ObjectBase base class
  static TypeId GetTypeId (void);

  /// The possible types of content (default = NOT_SET).
  enum ContentType_t
  {
    NOT_SET,         ///< Integer equivalent = 0
    MAIN_OBJECT,     ///< Integer equivalent = 1
    EMBEDDED_OBJECT  ///< Integer equivalent = 2
  };

  /**
   * \param contentType the value for the Content-Type field of this header
   *                    instance
   */
  void SetContentType (ContentType_t contentType);

  /**
   * \return the current value of the ContentType-field of this header instance
   */
  ContentType_t GetContentType () const;

  /**
   * \param contentLength the value for the Content-Length field of this header
   *                      instance
   */
  void SetContentLength (uint32_t contentLength);

  /**
   * \return the current value of the ContentType-field of this header instance
   */
  uint32_t GetContentLength () const;

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
  uint16_t m_contentType;
  uint32_t m_contentLength;

}; // end of `class HttpEntityHeader`


}  // end of `namespace ns3`


#endif /* HTTP_ENTITY_HEADER_H */
