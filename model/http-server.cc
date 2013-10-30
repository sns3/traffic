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

#include "http-server.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/pointer.h>
#include <ns3/uinteger.h>
#include <ns3/http-variables.h>
#include <ns3/packet.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/http-entity-header.h>
#include <ns3/unused.h>


NS_LOG_COMPONENT_DEFINE ("HttpServer");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HttpServer);


HttpServer::HttpServer ()
  : m_state (NOT_STARTED),
    m_initialSocket (0),
    m_httpVariables (CreateObject<HttpVariables> ())
{
  NS_LOG_FUNCTION (this);

  //m_mtuSize = m_httpVariables->GetMtuSize ();
  m_mtuSize = 536; /// \todo Find out why 1460 doesn't work
  NS_LOG_INFO (this << " MTU size for this server application is "
                    << m_mtuSize << " bytes");
}


HttpServer::~HttpServer ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
HttpServer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::HttpServer")
    .SetParent<Application> ()
    .AddConstructor<HttpServer> ()
    .AddAttribute ("Variables",
                   "Pointer to random number generator",
                   PointerValue (),
                   MakePointerAccessor (&HttpServer::m_httpVariables),
                   MakePointerChecker<HttpVariables> ())
    .AddAttribute ("LocalAddress",
                   "The local address of the server, "
                   "i.e., the address on which to bind the Rx socket",
                   AddressValue (),
                   MakeAddressAccessor (&HttpServer::m_localAddress),
                   MakeAddressChecker ())
    .AddAttribute ("Port",
                   "Port on which the application listen for incoming packets",
                   UintegerValue (80), // the default HTTP port
                   MakeUintegerAccessor (&HttpServer::m_localPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Protocol",
                   "The type of protocol to use",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&HttpServer::m_protocol),
                   MakeTypeIdChecker ())
    .AddAttribute ("ResponseDelay",
                   "The constant delay time between a reception of an HTTP request "
                   "and the send out of the HTTP response by this application "
                   "(not applicable at the moment)",
                   TimeValue (MilliSeconds (10)),
                   MakeTimeAccessor (&HttpServer::m_responseDelay),
                   MakeTimeChecker ())
    .AddTraceSource ("Tx",
                     "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&HttpServer::m_txTrace))
    .AddTraceSource ("Rx",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&HttpServer::m_rxTrace))
    .AddTraceSource ("StateTransition",
                     "Trace fired upon every HTTP client state transition",
                     MakeTraceSourceAccessor (&HttpServer::m_stateTransitionTrace))
  ;
  return tid;
}


HttpServer::State_t
HttpServer::GetState () const
{
  return m_state;
}


std::string
HttpServer::GetStateString () const
{
  return GetStateString (m_state);
}


std::string
HttpServer::GetStateString (HttpServer::State_t state)
{
  std::string ret = "";
  switch (state)
  {
    case NOT_STARTED:
      ret = "NOT_STARTED";
      break;
    case STARTED:
      ret = "STARTED";
      break;
    default:
      NS_FATAL_ERROR ("Unknown state");
      break;
  }
  return ret;
}


void
HttpServer::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose (); // chain up
}


void
HttpServer::StartApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == NOT_STARTED)
    {
      if (m_initialSocket == 0)
        {
          NS_LOG_INFO (this << " creating the initial socket of "
                            << m_protocol.GetName ());
          m_initialSocket = Socket::CreateSocket (GetNode (), m_protocol);
          int ret;

          if (Ipv4Address::IsMatchingType (m_localAddress))
            {
              Ipv4Address ipv4 = Ipv4Address::ConvertFrom (m_localAddress);
              InetSocketAddress inetSocket = InetSocketAddress (ipv4,
                                                                m_localPort);
              NS_LOG_INFO (this << " binding on " << ipv4
                                << " port " << m_localPort
                                << " / " << inetSocket);
              ret = m_initialSocket->Bind (inetSocket);
              NS_LOG_DEBUG (this << " Bind() return value= " << ret
                                 << " GetErrNo= " << m_initialSocket->GetErrno ());
            }
          else if (Ipv6Address::IsMatchingType (m_localAddress))
            {
              Ipv6Address ipv6 = Ipv6Address::ConvertFrom (m_localAddress);
              Inet6SocketAddress inet6Socket = Inet6SocketAddress (ipv6,
                                                                   m_localPort);
              NS_LOG_INFO (this << " binding on " << ipv6
                                << " port " << m_localPort
                                << " / " << inet6Socket);
              ret = m_initialSocket->Bind (inet6Socket);
              NS_LOG_DEBUG (this << " Bind() return value= " << ret
                                 << " GetErrNo= " << m_initialSocket->GetErrno ());
            }

          ret = m_initialSocket->Listen ();
          NS_LOG_DEBUG (this << " Listen () return value= " << ret
                             << " GetErrNo= " << m_initialSocket->GetErrno ());

          NS_UNUSED (ret);

        } // end of `if (m_initialSocket == 0)`

      NS_ASSERT_MSG (m_initialSocket != 0, "Failed creating socket");
      m_initialSocket->SetAcceptCallback (MakeCallback (&HttpServer::ConnectionRequestCallback,
                                                        this),
                                          MakeCallback (&HttpServer::NewConnectionCreatedCallback,
                                                        this));
      m_initialSocket->SetCloseCallbacks (MakeCallback (&HttpServer::NormalCloseCallback,
                                                        this),
                                          MakeCallback (&HttpServer::ErrorCloseCallback,
                                                        this));
      m_initialSocket->SetRecvCallback (MakeCallback (&HttpServer::ReceivedDataCallback,
                                                      this));
      m_initialSocket->SetSendCallback (MakeCallback (&HttpServer::SendCallback,
                                                      this));
      SwitchToState (STARTED);

    } // end of `if (m_state == NOT_STARTED)`
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                        << " for StartApplication");
    }

} // end of `void StartApplication ()`


void
HttpServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  // close all accepted sockets
  std::map<Ptr<Socket>, HttpEntityHeader>::iterator it;
  for (it = m_acceptedSocketTxBuffer.begin ();
       it != m_acceptedSocketTxBuffer.end (); it++)
    {
      it->first->Close ();
      it->first->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
      it->first->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }

  if (m_initialSocket != 0)
    {
      m_initialSocket->Close ();
      m_initialSocket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
      m_initialSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }

  /// \todo Cancel any remaining events?
}


bool
HttpServer::ConnectionRequestCallback (Ptr<Socket> socket,
                                       const Address & address)
{
  NS_LOG_FUNCTION (this << socket << address);
  return true; // unconditionally accept the connection request
}


void
HttpServer::NewConnectionCreatedCallback (Ptr<Socket> socket,
                                          const Address & address)
{
  NS_LOG_FUNCTION (this << socket << address);

  socket->SetCloseCallbacks (MakeCallback (&HttpServer::NormalCloseCallback,
                                           this),
                             MakeCallback (&HttpServer::ErrorCloseCallback,
                                           this));
  socket->SetRecvCallback (MakeCallback (&HttpServer::ReceivedDataCallback,
                                         this));
  socket->SetSendCallback (MakeCallback (&HttpServer::SendCallback,
                                         this));

  std::map<Ptr<Socket>, HttpEntityHeader>::iterator it;
  it = m_acceptedSocketTxBuffer.find (socket);

  if (it == m_acceptedSocketTxBuffer.end ())
    {
      // this is a new socket, keep it
      HttpEntityHeader httpEntity; // vanilla
      m_acceptedSocketTxBuffer.insert (
        std::pair<Ptr<Socket>, HttpEntityHeader> (socket, httpEntity));
    }
  else
    {
      NS_LOG_WARN (this << " socket " << socket << " already exists");
    }
}


void
HttpServer::NormalCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (socket == m_initialSocket)
    {
      NS_FATAL_ERROR ("Initial listener socket shall not be closed");
    }

  std::map<Ptr<Socket>, HttpEntityHeader>::iterator it;
  it = m_acceptedSocketTxBuffer.find (socket);

  if (it == m_acceptedSocketTxBuffer.end ())
    {
      NS_LOG_WARN (this << " Invalid socket");
    }
  else
    {
      m_acceptedSocketTxBuffer.erase (it);
    }
}


void
HttpServer::ErrorCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (socket == m_initialSocket)
    {
      NS_FATAL_ERROR ("Initial listener socket shall not be closed");
    }

  std::map<Ptr<Socket>, HttpEntityHeader>::iterator it;
  it = m_acceptedSocketTxBuffer.find (socket);

  if (it == m_acceptedSocketTxBuffer.end ())
    {
      NS_LOG_WARN (this << " Invalid socket");
    }
  else
    {
      m_acceptedSocketTxBuffer.erase (it);
    }
}


void
HttpServer::ReceivedDataCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0)
        {
          break; // EOF
        }

      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO (this << " a packet of " << packet->GetSize () << " bytes"
                            << " received from " << InetSocketAddress::ConvertFrom (from).GetIpv4 ()
                            << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                            << " / " << InetSocketAddress::ConvertFrom (from));
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO (this << " a packet of " << packet->GetSize () << " bytes"
                            << " received from " << Inet6SocketAddress::ConvertFrom (from).GetIpv6 ()
                            << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort ()
                            << " / " << InetSocketAddress::ConvertFrom (from));
        }

      HttpEntityHeader httpEntity;
      packet->RemoveHeader (httpEntity);

      switch (httpEntity.GetContentType ())
      {
        case HttpEntityHeader::MAIN_OBJECT:
          /// \todo Add a response delay here
          Simulator::ScheduleNow (&HttpServer::ServeMainObject, this, socket);
          break;
        case HttpEntityHeader::EMBEDDED_OBJECT:
          /// \todo Add a response delay here
          Simulator::ScheduleNow (&HttpServer::ServeEmbeddedObject, this, socket);
          break;
        default:
          NS_LOG_WARN (this << " Invalid packet header");
          break;
      }

      m_rxTrace (packet, from);

    } // end of `while ((packet = socket->RecvFrom (from)))`

} // end of `void ReceivedDataCallback (Ptr<Socket> socket)`


void
HttpServer::SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize)
{
  NS_LOG_FUNCTION (this << socket << availableBufferSize);

  std::map<Ptr<Socket>, HttpEntityHeader>::iterator it;
  it = m_acceptedSocketTxBuffer.find (socket);

  if (it == m_acceptedSocketTxBuffer.end ())
    {
      NS_LOG_WARN (this << " Invalid socket");
    }
  else
    {
      uint32_t txBufferSize = it->second.GetContentLength ();

      if (txBufferSize > 0)
        {
          /*
           * Pending transmission of this socket exists in the Tx buffer. We
           * fetch it here by first clearing the Tx buffer, and then followed by
           * actual transmission.
           */
          HttpEntityHeader txInfo (it->second); // copy
          it->second.SetContentLength (0);

          switch (it->second.GetContentType ())
          {
            uint32_t actualSent;

            case HttpEntityHeader::MAIN_OBJECT:
              NS_LOG_INFO (this << " continue transmitting " << txBufferSize
                                << " bytes of main object from Tx buffer");
              actualSent = Serve (socket, txInfo);
              if (actualSent < txBufferSize)
                {
                  NS_LOG_INFO (this << " transmission of main object is suspended"
                                    << " after " << actualSent << " bytes");
                }
              else
                {
                  NS_LOG_INFO (this << " finished sending a whole main object");
                }
              break;

            case HttpEntityHeader::EMBEDDED_OBJECT:
              NS_LOG_INFO (this << " continue transmitting " << txBufferSize
                                << " bytes of embedded object from Tx buffer");
              actualSent = Serve (socket, txInfo);
              if (actualSent < txBufferSize)
                {
                  NS_LOG_INFO (this << " transmission of embedded object is suspended"
                                    << " after " << actualSent << " bytes");
                }
              else
                {
                  NS_LOG_INFO (this << " finished sending a whole embedded object");
                }
              break;

            default:
              NS_FATAL_ERROR ("Unknown Content-Type: " << it->second.GetContentType ());
              break;
          }

        } // end of `if (bufferSize > 0)`

    } // end of else of `if (it == m_acceptedSocketTxBuffer.end ())`

} // end of `void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize)`


void
HttpServer::ServeMainObject (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  uint32_t objectSize = m_httpVariables->GetEmbeddedObjectSize ();
  NS_LOG_INFO (this << " embedded object to be served is "
                    << objectSize << " bytes");

  HttpEntityHeader txInfo;
  txInfo.SetContentLength (objectSize);
  txInfo.SetContentType (HttpEntityHeader::MAIN_OBJECT);

  uint32_t actualSent = Serve (socket, txInfo);

  if (actualSent < objectSize)
    {
      NS_LOG_INFO (this << " transmission of main object is suspended"
                        << " after " << actualSent << " bytes");
    }
  else
    {
      NS_LOG_INFO (this << " finished sending a whole main object");
    }
}


void
HttpServer::ServeEmbeddedObject (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  uint32_t objectSize = m_httpVariables->GetEmbeddedObjectSize ();
  NS_LOG_INFO (this << " embedded object to be served is "
                    << objectSize << " bytes");

  HttpEntityHeader txInfo;
  txInfo.SetContentLength (objectSize);
  txInfo.SetContentType (HttpEntityHeader::EMBEDDED_OBJECT);

  uint32_t actualSent = Serve (socket, txInfo);

  if (actualSent < objectSize)
    {
      NS_LOG_INFO (this << " transmission of embedded object is suspended"
                        << " after " << actualSent << " bytes");
    }
  else
    {
      NS_LOG_INFO (this << " finished sending a whole embedded object");
    }
}


uint32_t
HttpServer::Serve (Ptr<Socket> socket, HttpEntityHeader txInfo)
{
  NS_LOG_FUNCTION (this << socket);

  std::map<Ptr<Socket>, HttpEntityHeader>::iterator it;
  it = m_acceptedSocketTxBuffer.find (socket);
  NS_ASSERT_MSG (it != m_acceptedSocketTxBuffer.end (),
                 "Invalid socket");

  uint32_t bytesTxed = 0;
  uint32_t remainingBytes = txInfo.GetContentLength ();
  uint32_t headerSize = txInfo.GetSerializedSize ();
  uint32_t contentSize;
  uint32_t packetSize;

  while (remainingBytes > 0)
    {
      contentSize = std::min (remainingBytes, m_mtuSize - headerSize);
      NS_ASSERT_MSG (contentSize > 0,
                     "Invalid size of packet content: " << contentSize);

      Ptr<Packet> packet = Create<Packet> (contentSize);
      txInfo.SetContentLength (remainingBytes);
      packet->AddHeader (txInfo);
      packetSize = packet->GetSize ();
      NS_LOG_INFO (this << " created packet " << packet << " of "
                        << packetSize << " bytes");
      m_txTrace (packet);
      int actualBytes = socket->Send (packet);
      NS_LOG_DEBUG (this << " Send() packet " << packet
                         << " of " << packet->GetSize () << " bytes,"
                         << " return value= " << actualBytes);

      if ((unsigned) actualBytes == packetSize)
        {
          bytesTxed += contentSize;
          remainingBytes -= contentSize;
          NS_LOG_INFO (this << " remaining object to be transmitted "
                            << remainingBytes << " bytes");
        }
      else
        {
          NS_LOG_INFO (this << " failed to send object,"
                            << " GetErrNo= " << socket->GetErrno () << ","
                            << " waiting for another Tx opportunity");
          /*
           * Save a record of the remains and suspend transmission. Transmission
           * shall continue later in SendCallback when the socket becomes ready.
           */
          SaveTxBuffer (socket, txInfo);
          remainingBytes = 0; // this exits the `while (remainingBytes > 0)`
        }

    } // end of `while (remainingBytes > 0)`

  return bytesTxed;

} // end of `uint32_t Serve (Ptr<Socket> socket, HttpEntityHeader txInfo)`


void
HttpServer::SaveTxBuffer (const Ptr<Socket> socket,
                          const HttpEntityHeader txInfo)
{
  NS_LOG_FUNCTION (this << socket << txInfo.GetContentType ()
                        << txInfo.GetContentLength ());

  std::map<Ptr<Socket>, HttpEntityHeader>::iterator it;
  it = m_acceptedSocketTxBuffer.find (socket);

  if (it == m_acceptedSocketTxBuffer.end ())
    {
      NS_FATAL_ERROR ("Invalid socket " << socket);
    }
  else
    {
      if (it->second.GetContentLength () == 0)
        {
          // existing Tx buffer is empty
          it->second.SetContentLength (txInfo.GetContentLength ());
          it->second.SetContentType (txInfo.GetContentType ());
        }
      else if (it->second.GetContentType () == txInfo.GetContentType ())
        {
          // pre-existing Tx buffer of same Content-Type, increment the buffer
          it->second.SetContentLength (it->second.GetContentLength ()
                                       + txInfo.GetContentLength ());
        }
      else
        {
          NS_FATAL_ERROR ("Unable to append Tx buffer with more packets "
            << "on top of a pending transmission of another Content-Type");
        }
    }

} // end of `void SaveTxBuffer (const Ptr<Socket> socket, const HttpEntityHeader txBuffer)`


void
HttpServer::SwitchToState (HttpServer::State_t state)
{
  std::string oldState = GetStateString ();
  std::string newState = GetStateString (state);
  NS_LOG_FUNCTION (this << oldState << newState);
  m_state = state;
  NS_LOG_INFO (this << " HttpServer " << oldState << " --> " << newState);
  m_stateTransitionTrace (oldState, newState);
}


} // end of `namespace ns3`
