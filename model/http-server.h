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

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <ns3/simple-ref-count.h>
#include <ns3/nstime.h>
#include <ns3/http-entity-header.h>
#include <ns3/application.h>
#include <ns3/address.h>
#include <ns3/traced-callback.h>
#include <map>


namespace ns3 {


class Socket;


class HttpServerTxBuffer : public SimpleRefCount<HttpServerTxBuffer>
{
public:
  HttpServerTxBuffer ();
  virtual ~HttpServerTxBuffer ();
  bool IsSocketAvailable (Ptr<Socket> socket) const;
  void AddSocket (Ptr<Socket> socket);
  void RemoveSocket (Ptr<Socket> socket);
  void CloseSocket (Ptr<Socket> socket);
  void CloseAllSockets ();

  bool IsBufferEmpty (Ptr<Socket> socket) const;
  HttpEntityHeader::ContentType_t GetBufferContentType (Ptr<Socket> socket) const;
  uint32_t GetBufferSize (Ptr<Socket> socket) const;
  bool HasTxedPartOfObject (Ptr<Socket> socket) const;

  void WriteNewObject (Ptr<Socket> socket,
                       HttpEntityHeader::ContentType_t contentType,
                       uint32_t objectSize);
  void RecordNextServe (Ptr<Socket> socket, EventId eventId);
  void DepleteBufferSize (Ptr<Socket> socket, uint32_t amount);

private:
  struct TxBuffer_t
  {
    EventId                          nextServe;
    HttpEntityHeader::ContentType_t  txBufferContentType;
    uint32_t                         txBufferSize;
    bool                             hasTxedPartOfObject;
  };

  std::map<Ptr<Socket>, TxBuffer_t> m_txBuffer;
};


class Packet;
class HttpVariables;


class HttpServer : public Application
{
public:
  HttpServer ();
  virtual ~HttpServer ();
  static TypeId GetTypeId ();

  uint32_t GetMtuSize () const;
  Address GetAddress () const;
  uint16_t GetPort () const;

  enum State_t
  {
    NOT_STARTED = 0,
    STARTED,
    STOPPED
  };

  State_t GetState () const;
  std::string GetStateString () const;
  static std::string GetStateString (State_t state);

protected:
  // Inherited from Object base class
  virtual void DoDispose ();

  // Inherited from Application base class
  virtual void StartApplication ();
  virtual void StopApplication ();

private:
  virtual bool ConnectionRequestCallback (Ptr<Socket> socket,
                                          const Address & address);
  virtual void NewConnectionCreatedCallback (Ptr<Socket> socket,
                                             const Address & address);
  virtual void NormalCloseCallback (Ptr<Socket> socket);
  virtual void ErrorCloseCallback (Ptr<Socket> socket);
  virtual void ReceivedDataCallback (Ptr<Socket> socket);
  virtual void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize);

  void ServeNewMainObject (Ptr<Socket> socket);
  void ServeNewEmbeddedObject (Ptr<Socket> socket);
  uint32_t ServeFromTxBuffer (Ptr<Socket> socket);
  void SwitchToState (State_t state);

  State_t                  m_state;
  uint32_t                 m_mtuSize;
  Ptr<Socket>              m_initialSocket;
  Ptr<HttpServerTxBuffer>  m_txBuffer;

  // ATTRIBUTES

  Ptr<HttpVariables>  m_httpVariables;
  Address             m_localAddress;
  uint16_t            m_localPort;
  TypeId              m_protocol;

  // TRACE SOURCES

  TracedCallback<Ptr<const Packet> >                   m_txTrace;
  TracedCallback<Ptr<const Packet>, const Address &>   m_rxTrace;
  TracedCallback<std::string, std::string>             m_stateTransitionTrace;

}; // end of `class HttpServer`


}  // end of `namespace ns3`


#endif /* HTTP_SERVER_H */
