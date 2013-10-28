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
    case WAITING_CONNECTION_REQUEST:
      ret = "WAITING_CONNECTION_REQUEST";
      break;
    case CONNECTED:
      ret = "CONNECTED";
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
      m_initialSocket->SetRecvCallback (MakeCallback (&HttpServer::ReceivedDataCallback,
                                                      this));
      SwitchToState (WAITING_CONNECTION_REQUEST);

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

  while (!m_acceptedSockets.empty ()) // these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_acceptedSockets.front ();
      m_acceptedSockets.pop_front ();
      acceptedSocket->Close ();
    }

  if (m_initialSocket)
    {
      m_initialSocket->Close ();
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
  socket->SetRecvCallback (MakeCallback (&HttpServer::ReceivedDataCallback,
                                         this));
  m_acceptedSockets.push_back (socket);
  SwitchToState (CONNECTED);
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
HttpServer::SwitchToState (HttpServer::State_t state)
{
  std::string oldState = GetStateString ();
  std::string newState = GetStateString (state);
  NS_LOG_FUNCTION (this << oldState << newState);
  m_state = state;
  NS_LOG_INFO (this << " HttpServer " << oldState << " --> " << newState);
  m_stateTransitionTrace (oldState, newState);
}


void
HttpServer::ServeMainObject (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);
  uint32_t remainingObjectSize = m_httpVariables->GetMainObjectSize ();
  NS_LOG_INFO (this << " main object to be served is "
                    << remainingObjectSize << " bytes");
  uint32_t packetSize;

  while (remainingObjectSize > 0)
    {
      HttpEntityHeader httpEntity;
      httpEntity.SetContentLength (remainingObjectSize);
      httpEntity.SetContentType (HttpEntityHeader::MAIN_OBJECT);

      packetSize = (remainingObjectSize < m_mtuSize) ? remainingObjectSize
                                                     : m_mtuSize;
      Ptr<Packet> packet = Create<Packet> (packetSize - httpEntity.GetSerializedSize ());
      packet->AddHeader (httpEntity);
      NS_LOG_INFO (this << " created packet " << packet << " of "
                        << packetSize << " bytes");
      m_txTrace (packet);
      int actualSent = socket->Send (packet);
      NS_LOG_DEBUG (this << " Send() return value= " << actualSent);

      if ((unsigned) actualSent == packetSize)
        {
          remainingObjectSize = remainingObjectSize - actualSent;
          NS_LOG_INFO (this << " remaining main object to be transmitted "
                            << remainingObjectSize << " bytes");
        }
      else
        {
          NS_LOG_INFO (this << " failed to send request for main object,"
                            << " waiting for another Tx opportunity");
          remainingObjectSize = 0; // this exits the `while (objectSize > 0)`
        }

    }

  NS_LOG_INFO (this << " finished sending main object");

} // end of `void ServeMainObject (Ptr<Socket> socket)`


void
HttpServer::ServeEmbeddedObject (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);
}


} // end of `namespace ns3`
