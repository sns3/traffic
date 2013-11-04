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
#include <ns3/config.h>
#include <ns3/pointer.h>
#include <ns3/uinteger.h>
#include <ns3/http-variables.h>
#include <ns3/packet.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
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

  m_mtuSize = m_httpVariables->GetMtuSize ();
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
                   "The type of protocol to use (only ns3::TcpSocketFactory is valid for now)",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&HttpServer::m_protocol),
                   MakeTypeIdChecker ())
    .AddAttribute ("Mtu",
                   "Maximum transmission unit (in bytes) of the TCP sockets "
                   "used in this application, excluding the compulsory 40 "
                   "bytes TCP header. Typical values are 1460 and 536 bytes. "
                   "The attribute is read-only because the value is set randomly.",
                   TypeId::ATTR_GET,
                   UintegerValue (),
                   MakeUintegerAccessor (&HttpServer::m_mtuSize),
                   MakeUintegerChecker<uint32_t> ())
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


uint32_t
HttpServer::GetMtuSize () const
{
  return m_mtuSize;
}


Address
HttpServer::GetAddress () const
{
  return m_localAddress;
}


uint16_t
HttpServer::GetPort () const
{
  return m_localPort;
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
    case STOPPED:
      ret = "STOPPED";
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

  if (!Simulator::IsFinished ()) // guard against canceling out-of-bound events
    {
      StopApplication ();
    }

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
          if (m_protocol != TcpSocketFactory::GetTypeId ())
            {
              NS_FATAL_ERROR ("Socket other than "
                << TcpSocketFactory::GetTypeId ().GetName ()
                << " are not supported at the moment");
            }

          // find the current default MTU value of TCP sockets
          Ptr<const ns3::AttributeValue> previousSocketMtu;
          TypeId tcpSocketTid = TypeId::LookupByName ("ns3::TcpSocket");
          for (uint32_t i = 0; i < tcpSocketTid.GetAttributeN (); i++)
            {
              struct TypeId::AttributeInformation attrInfo = tcpSocketTid.GetAttribute (i);
              if (attrInfo.name == "SegmentSize")
                {
                  previousSocketMtu = attrInfo.initialValue;
                }
            }

          // change the default MTU value for all TCP sockets
          Config::SetDefault ("ns3::TcpSocket::SegmentSize",
                              UintegerValue (m_mtuSize));

          // creating a TCP socket to connect to the server
          m_initialSocket = Socket::CreateSocket (GetNode (), m_protocol);
#ifdef NS3_ASSERT_ENABLE
          UintegerValue mtu;
          m_initialSocket->GetAttribute ("SegmentSize", mtu);
          NS_LOG_INFO (this << " created socket " << m_initialSocket
                            << " of " << m_protocol.GetName ()
                            << " with MTU of " << mtu.Get () << " bytes");
          NS_UNUSED (mtu);
#endif /* NS3_ASSERT_ENABLE */

          Config::SetDefault ("ns3::TcpSocket::SegmentSize",
                              *previousSocketMtu); // reset it back

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

  SwitchToState (STOPPED);

  // close all accepted sockets
  std::map<Ptr<Socket>, SocketInfo_t>::iterator it;
  for (it = m_acceptedSockets.begin ();
       it != m_acceptedSockets.end (); it++)
    {
      if (!Simulator::IsExpired (it->second.nextServe))
        {
          NS_LOG_INFO (this << " canceling a serving event which is due in "
                            << Simulator::GetDelayLeft (it->second.nextServe).GetSeconds ()
                            << " seconds");
          Simulator::Cancel (it->second.nextServe);
        }

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

#ifdef NS3_ASSERT_ENABLE
  UintegerValue mtu;
  socket->GetAttribute ("SegmentSize", mtu);
  NS_LOG_INFO (this << " new connection from socket " << socket
                    << " with MTU of " << mtu.Get () << " bytes");
  NS_UNUSED (mtu);
#endif /* NS3_ASSERT_ENABLE */

  socket->SetCloseCallbacks (MakeCallback (&HttpServer::NormalCloseCallback,
                                           this),
                             MakeCallback (&HttpServer::ErrorCloseCallback,
                                           this));
  socket->SetRecvCallback (MakeCallback (&HttpServer::ReceivedDataCallback,
                                         this));
  socket->SetSendCallback (MakeCallback (&HttpServer::SendCallback,
                                         this));

  std::map<Ptr<Socket>, SocketInfo_t>::iterator it;
  it = m_acceptedSockets.find (socket);

  if (it == m_acceptedSockets.end ())
    {
      // this is a new socket, keep it
      SocketInfo_t socketInfo;
      socketInfo.txBufferContentType = HttpEntityHeader::MAIN_OBJECT;
      socketInfo.txBufferContentLength = 0;
      m_acceptedSockets.insert (
        std::pair<Ptr<Socket>, SocketInfo_t> (socket, socketInfo));
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
      if (m_state == STARTED)
        {
          NS_FATAL_ERROR ("Initial listener socket shall not be closed when server is still running");
        }
    }
  else
    {
      std::map<Ptr<Socket>, SocketInfo_t>::iterator it;
      it = m_acceptedSockets.find (socket);

      if (it == m_acceptedSockets.end ())
        {
          NS_LOG_WARN (this << " Invalid socket");
        }
      else
        {
          if (!Simulator::IsExpired (it->second.nextServe))
            {
              NS_LOG_INFO (this << " canceling a serving event which is due in "
                                << Simulator::GetDelayLeft (it->second.nextServe).GetSeconds ()
                                << " seconds");
              Simulator::Cancel (it->second.nextServe);
            }
          m_acceptedSockets.erase (it);
        }
    }

}


void
HttpServer::ErrorCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (socket == m_initialSocket)
    {
      if (m_state == STARTED)
        {
          NS_FATAL_ERROR ("Initial listener socket shall not be closed when server is still running");
        }
    }
  else
    {
      std::map<Ptr<Socket>, SocketInfo_t>::iterator it;
      it = m_acceptedSockets.find (socket);

      if (it == m_acceptedSockets.end ())
        {
          NS_LOG_WARN (this << " Invalid socket");
        }
      else
        {
          if (!Simulator::IsExpired (it->second.nextServe))
            {
              NS_LOG_INFO (this << " canceling a serving event which is due in "
                                << Simulator::GetDelayLeft (it->second.nextServe).GetSeconds ()
                                << " seconds");
              Simulator::Cancel (it->second.nextServe);
            }
          m_acceptedSockets.erase (it);
        }
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

      m_rxTrace (packet, from);

      std::map<Ptr<Socket>, SocketInfo_t>::iterator it;
      it = m_acceptedSockets.find (socket);

      if (it == m_acceptedSockets.end ())
        {
          NS_LOG_WARN (this << " Invalid socket");
        }
      else
        {
          HttpEntityHeader httpEntity;
          packet->RemoveHeader (httpEntity);
          Time delay;

          switch (httpEntity.GetContentType ())
          {
            case HttpEntityHeader::MAIN_OBJECT:
              delay = m_httpVariables->GetMainObjectGenerationDelay ();
              NS_LOG_INFO (this << " will finish generating a main object in "
                                << delay.GetSeconds () << " seconds");
              it->second.nextServe = Simulator::Schedule (
                delay, &HttpServer::ServeMainObject, this, socket);
              break;

            case HttpEntityHeader::EMBEDDED_OBJECT:
              delay = m_httpVariables->GetEmbeddedObjectGenerationDelay ();
              NS_LOG_INFO (this << " will finish generating an embedded object in "
                                << delay.GetSeconds () << " seconds");
              it->second.nextServe = Simulator::Schedule (
                delay, &HttpServer::ServeEmbeddedObject, this, socket);
              break;

            default:
              NS_LOG_WARN (this << " Invalid packet header");
              break;
          }

        } // end of else of `if (it == m_acceptedSockets.end ())`

    } // end of `while ((packet = socket->RecvFrom (from)))`

} // end of `void ReceivedDataCallback (Ptr<Socket> socket)`


void
HttpServer::SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize)
{
  NS_LOG_FUNCTION (this << socket << availableBufferSize);

  std::map<Ptr<Socket>, SocketInfo_t>::iterator it;
  it = m_acceptedSockets.find (socket);

  if (it == m_acceptedSockets.end ())
    {
      NS_LOG_WARN (this << " Invalid socket");
    }
  else
    {
      uint32_t txBufferSize = it->second.txBufferContentLength;

      if (txBufferSize > 0)
        {
          /*
           * Pending transmission of this socket exists in the Tx buffer. We
           * fetch it here by first clearing the Tx buffer, and then followed by
           * actual transmission.
           */
          it->second.txBufferContentLength = 0;

          switch (it->second.txBufferContentType)
          {
            uint32_t actualSent;

            case HttpEntityHeader::MAIN_OBJECT:
              NS_LOG_INFO (this << " continue transmitting " << txBufferSize
                                << " bytes of main object from Tx buffer");
              actualSent = Serve (socket, it->second.txBufferContentType,
                                  txBufferSize);
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
              actualSent = Serve (socket, it->second.txBufferContentType,
                                  txBufferSize);
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
              NS_FATAL_ERROR ("Unknown Content-Type: " << it->second.txBufferContentType);
              break;
          }

        } // end of `if (bufferSize > 0)`

    } // end of else of `if (it == m_acceptedSockets.end ())`

} // end of `void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize)`


void
HttpServer::ServeMainObject (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  uint32_t objectSize = m_httpVariables->GetMainObjectSize ();
  NS_LOG_INFO (this << " main object to be served is "
                    << objectSize << " bytes");
  uint32_t actualSent = Serve (socket, HttpEntityHeader::MAIN_OBJECT,
                               objectSize);

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
  uint32_t actualSent = Serve (socket, HttpEntityHeader::EMBEDDED_OBJECT,
                               objectSize);

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
HttpServer::Serve (Ptr<Socket> socket,
                   HttpEntityHeader::ContentType_t contentType,
                   uint32_t objectSize)
{
  NS_LOG_FUNCTION (this << socket);

  std::map<Ptr<Socket>, SocketInfo_t>::iterator it;
  it = m_acceptedSockets.find (socket);
  NS_ASSERT_MSG (it != m_acceptedSockets.end (),
                 "Invalid socket");

  uint32_t bytesTxed = 0;
  uint32_t contentSize; // size of actual content
  uint32_t headerSize = HttpEntityHeader::GetStaticSerializedSize ();
  uint32_t packetSize; // contentSize + headerSize

  NS_LOG_DEBUG (this << " socket has " << socket->GetTxAvailable ()
                     << " bytes available for Tx");

  while ((objectSize > 0) && (socket->GetTxAvailable () > headerSize))
    {
      contentSize = std::min (objectSize,
                              socket->GetTxAvailable () - headerSize);
      NS_ASSERT_MSG (contentSize > 0,
                     "Invalid size of packet content: " << contentSize);
      HttpEntityHeader httpEntityHeader;
      httpEntityHeader.SetContentType (contentType);
      httpEntityHeader.SetContentLength (objectSize); // contentSize is not used here
      /*
       * Content-Length above is the size of the whole object that remains.
       * That's why objectSize is used here instead of contentSize. Thus it is
       * always larger than the size of the packet that will be transmitted
       * below.
       */
      Ptr<Packet> packet = Create<Packet> (contentSize);
      packet->AddHeader (httpEntityHeader);
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
          objectSize -= contentSize;
          NS_LOG_INFO (this << " remaining object to be transmitted "
                            << objectSize << " bytes");
        }
      else
        {
          NS_LOG_INFO (this << " failed to send object,"
                            << " GetErrNo= " << socket->GetErrno () << ","
                            << " suspending transmission"
                            << " and waiting for another Tx opportunity");
          break; // this exits the `while ((objectSize > 0) && (socket->GetTxAvailable () > headerSize))`
        }

      NS_LOG_DEBUG (this << " socket has " << socket->GetTxAvailable ()
                         << " bytes available for Tx");

    } // end of `while ((objectSize > 0) && (socket->GetTxAvailable () > headerSize))`

  if (objectSize > 0)
    {
      /*
       * The whole object has only partially transmitted. Save a record of the
       * remains to Tx buffer. Transmission shall continue later in SendCallback
       * when the socket becomes ready.
       */
      SaveTxBuffer (socket, contentType, objectSize);
    }

  return bytesTxed;

} // end of `uint32_t Serve (Ptr<Socket> socket, HttpEntityHeader txInfo)`


void
HttpServer::SaveTxBuffer (const Ptr<Socket> socket,
                          HttpEntityHeader::ContentType_t contentType,
                          uint32_t objectSize)
{
  NS_LOG_FUNCTION (this << socket << contentType << objectSize);

  std::map<Ptr<Socket>, SocketInfo_t>::iterator it;
  it = m_acceptedSockets.find (socket);

  if (it == m_acceptedSockets.end ())
    {
      NS_FATAL_ERROR ("Invalid socket " << socket);
    }
  else
    {
      if (it->second.txBufferContentLength == 0)
        {
          // existing Tx buffer is empty
          it->second.txBufferContentType = contentType;
          it->second.txBufferContentLength = objectSize;
        }
      else if (it->second.txBufferContentLength == contentType)
        {
          // pre-existing Tx buffer of same Content-Type, increment the buffer
          it->second.txBufferContentLength += objectSize;
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
