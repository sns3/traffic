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
 * \brief Simplified HTTP entity header used by HTTP Client and Server
 *        applications.
 *
 * The header is 6 bytes in length. The first 2 bytes are for the Content-Type
 * header, while the remaining 4 bytes are for the Content-Length header.
 */
class HttpEntityHeader : public Header
{
public:
  HttpEntityHeader ();
  static TypeId GetTypeId (void);

  enum ContentType_t
  {
    NOT_SET,         ///< Integer equivalent = 0
    MAIN_OBJECT,     ///< Integer equivalent = 1
    EMBEDDED_OBJECT  ///< Integer equivalent = 2
  };

  void SetContentType (ContentType_t contentType);
  ContentType_t GetContentType () const;

  void SetContentLength (uint32_t contentLength);
  uint32_t GetContentLength () const;

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
