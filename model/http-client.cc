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

#include "http-client.h"
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


NS_LOG_COMPONENT_DEFINE ("HttpClient");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HttpClient);


HttpClient::HttpClient ()
  : m_state (NOT_STARTED),
    m_socket (0),
    m_embeddedObjectsToBeRequested (0),
    m_httpVariables (CreateObject<HttpVariables> ())
{
  NS_LOG_FUNCTION (this);
}


HttpClient::~HttpClient ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
HttpClient::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::HttpClient")
    .SetParent<Application> ()
    .AddConstructor<HttpClient> ()
    .AddAttribute ("Variables",
                   "Pointer to random number generator",
                   PointerValue (),
                   MakePointerAccessor (&HttpClient::m_httpVariables),
                   MakePointerChecker<HttpVariables> ())
    .AddAttribute ("RemoteServerAddress",
                   "The address of the destination server",
                   AddressValue (),
                   MakeAddressAccessor (&HttpClient::m_remoteServerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort",
                   "The destination port of the outbound packets",
                   UintegerValue (80), // the default HTTP port
                   MakeUintegerAccessor (&HttpClient::m_remoteServerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Protocol",
                   "The type of protocol to use",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&HttpClient::m_protocol),
                   MakeTypeIdChecker ())
    .AddTraceSource ("TxMainObjectRequest",
                     "Sent a request for a main object",
                     MakeTraceSourceAccessor (&HttpClient::m_txMainObjectRequestTrace))
    .AddTraceSource ("TxEmbeddedObjectRequest",
                     "Sent a request for an embedded object",
                     MakeTraceSourceAccessor (&HttpClient::m_txEmbeddedObjectRequestTrace))
    .AddTraceSource ("RxMainObjectPacket",
                     "A packet of main object has been received",
                     MakeTraceSourceAccessor (&HttpClient::m_rxMainObjectPacketTrace))
    .AddTraceSource ("RxMainObject",
                     "Received a whole main object",
                     MakeTraceSourceAccessor (&HttpClient::m_rxMainObjectTrace))
    .AddTraceSource ("RxEmbeddedObjectPacket",
                     "A packet of embedded object has been received",
                     MakeTraceSourceAccessor (&HttpClient::m_rxEmbeddedObjectPacketTrace))
    .AddTraceSource ("RxEmbeddedObject",
                     "Received a whole embedded object",
                     MakeTraceSourceAccessor (&HttpClient::m_rxEmbeddedObjectTrace))
    .AddTraceSource ("StateTransition",
                     "Trace fired upon every HTTP client state transition",
                     MakeTraceSourceAccessor (&HttpClient::m_stateTransitionTrace))
  ;
  return tid;
}


HttpClient::State_t
HttpClient::GetState () const
{
  return m_state;
}


std::string
HttpClient::GetStateString () const
{
  return GetStateString (m_state);
}


std::string
HttpClient::GetStateString (HttpClient::State_t state)
{
  std::string ret = "";
  switch (state)
  {
    case NOT_STARTED:
      ret = "NOT_STARTED";
      break;
    case CONNECTING:
      ret = "CONNECTING";
      break;
    case EXPECTING_MAIN_OBJECT:
      ret = "EXPECTING_MAIN_OBJECT";
      break;
    case PARSING_MAIN_OBJECT:
      ret = "PARSING_MAIN_OBJECT";
      break;
    case EXPECTING_EMBEDDED_OBJECT:
      ret = "EXPECTING_EMBEDDED_OBJECT";
      break;
    case READING:
      ret = "READING";
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
HttpClient::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose (); // chain up
}


void
HttpClient::StartApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == NOT_STARTED)
    {
      if (m_socket == 0)
        {
          NS_LOG_INFO (this << " creating a socket of " << m_protocol.GetName ());
          m_socket = Socket::CreateSocket (GetNode (), m_protocol);
          int ret;

          if (Ipv4Address::IsMatchingType (m_remoteServerAddress))
            {
              ret = m_socket->Bind ();
              NS_LOG_DEBUG (this << " Bind() return value= " << ret
                                 << " GetErrNo= " << m_socket->GetErrno ());

              Ipv4Address ipv4 = Ipv4Address::ConvertFrom (m_remoteServerAddress);
              InetSocketAddress inetSocket = InetSocketAddress (ipv4,
                                                                m_remoteServerPort);
              NS_LOG_INFO (this << " connecting to " << ipv4
                                << " port " << m_remoteServerPort
                                << " / " << inetSocket);
              ret = m_socket->Connect (inetSocket);
              NS_LOG_DEBUG (this << " Connect() return value= " << ret
                                 << " GetErrNo= " << m_socket->GetErrno ());
            }
          else if (Ipv6Address::IsMatchingType (m_remoteServerAddress))
            {
              ret = m_socket->Bind6 ();
              NS_LOG_DEBUG (this << " Bind6() return value= " << ret
                                 << " GetErrNo= " << m_socket->GetErrno ());

              Ipv6Address ipv6 = Ipv6Address::ConvertFrom (m_remoteServerAddress);
              Inet6SocketAddress inet6Socket = Inet6SocketAddress (ipv6,
                                                                   m_remoteServerPort);
              NS_LOG_INFO (this << " connecting to " << ipv6
                                << " port " << m_remoteServerPort
                                << " / " << inet6Socket);
              ret = m_socket->Connect (inet6Socket);
              NS_LOG_DEBUG (this << " Connect() return value= " << ret
                                 << " GetErrNo= " << m_socket->GetErrno ());
            }

          NS_UNUSED (ret);

        } // end of `if (m_socket == 0)`

      NS_ASSERT_MSG (m_socket != 0, "Failed creating socket");
      SwitchToState (CONNECTING);
      m_socket->SetConnectCallback (MakeCallback (&HttpClient::ConnectionSucceededCallback,
                                                  this),
                                    MakeCallback (&HttpClient::ConnectionFailedCallback,
                                                  this));
      m_socket->SetCloseCallbacks (MakeCallback (&HttpClient::NormalCloseCallback,
                                                 this),
                                   MakeCallback (&HttpClient::ErrorCloseCallback,
                                                 this));
      m_socket->SetRecvCallback (MakeCallback (&HttpClient::ReceivedDataCallback,
                                               this));
      m_socket->SetSendCallback (MakeCallback (&HttpClient::SendCallback,
                                               this));
    } // end of `if (m_state == NOT_STARTED)`
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                        << " for StartApplication");
    }

} // end of `void StartApplication ()`


void
HttpClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);
  /// \todo Cancel any remaining events?
  /// \todo Close socket?
}


void
HttpClient::ConnectionSucceededCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (m_state == CONNECTING)
    {
      NS_ASSERT_MSG (m_socket == socket, "Invalid socket");
      socket->SetRecvCallback (MakeCallback (&HttpClient::ReceivedDataCallback,
                                             this));
      socket->SetSendCallback (MakeCallback (&HttpClient::SendCallback,
                                             this));
      Simulator::ScheduleNow (&HttpClient::RequestMainObject, this);
    }
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                        << " for ConnectionSucceeded");
    }
}


void
HttpClient::ConnectionFailedCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (m_state == CONNECTING)
    {
      /// \todo Add delay before retrying?
      Simulator::ScheduleNow (&HttpClient::RetryConnection, this);
    }
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                        << " for ConnectionFailed");
    }
}


void
HttpClient::NormalCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Simulator::ScheduleNow (&HttpClient::RetryConnection, this);
}


void
HttpClient::ErrorCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Simulator::ScheduleNow (&HttpClient::RetryConnection, this);
}


void
HttpClient::ReceivedDataCallback (Ptr<Socket> socket)
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

      switch (m_state)
      {
        case EXPECTING_MAIN_OBJECT:
          ReceiveMainObject (packet);
          break;
        case EXPECTING_EMBEDDED_OBJECT:
          ReceiveEmbeddedObject (packet);
          break;
        default:
          NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                            << " for ReceivedData");
          break;
      }

    } // end of `while ((packet = socket->RecvFrom (from)))`

} // end of `void ReceivedDataCallback (Ptr<Socket> socket)`


void
HttpClient::SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize)
{
  NS_LOG_FUNCTION (this << socket << availableBufferSize);
}


void
HttpClient::RetryConnection ()
{
  NS_LOG_FUNCTION (this);

  if (Ipv4Address::IsMatchingType (m_remoteServerAddress))
    {
      Ipv4Address ipv4 = Ipv4Address::ConvertFrom (m_remoteServerAddress);
      InetSocketAddress inetSocket = InetSocketAddress (ipv4,
                                                        m_remoteServerPort);
      NS_LOG_INFO (this << " connecting to " << ipv4
                        << " port " << m_remoteServerPort
                        << " / " << inetSocket);
      int ret = m_socket->Connect (inetSocket);
      NS_LOG_DEBUG (this << " Connect() return value= " << ret
                         << " GetErrNo= " << m_socket->GetErrno ());
      NS_UNUSED (ret);
    }
  else if (Ipv6Address::IsMatchingType (m_remoteServerAddress))
    {
      Ipv6Address ipv6 = Ipv6Address::ConvertFrom (m_remoteServerAddress);
      Inet6SocketAddress inet6Socket = Inet6SocketAddress (ipv6,
                                                           m_remoteServerPort);
      NS_LOG_INFO (this << " retrying connection to " << ipv6
                        << " port " << m_remoteServerPort
                        << " / " << inet6Socket);
      int ret = m_socket->Connect (inet6Socket);
      NS_LOG_DEBUG (this << " Connect() return value= " << ret
                         << " GetErrNo= " << m_socket->GetErrno ());
      NS_UNUSED (ret);
    }

} // end of `void RetryConnection ()`


void
HttpClient::RequestMainObject ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == CONNECTING || m_state == READING)
    {
      HttpEntityHeader httpEntity;
      httpEntity.SetContentLength (0); // request does not need content length
      httpEntity.SetContentType (HttpEntityHeader::MAIN_OBJECT);
      uint32_t requestSize = m_httpVariables->GetRequestSize ();
      Ptr<Packet> packet = Create<Packet> (requestSize - httpEntity.GetSerializedSize ());
      packet->AddHeader (httpEntity);
      m_txMainObjectRequestTrace (packet);
      int actualBytes = m_socket->Send (packet);
      NS_LOG_DEBUG (this << " Send() packet " << packet
                         << " of " << packet->GetSize () << " bytes,"
                         << " return value= " << actualBytes);

      if ((unsigned) actualBytes != requestSize)
        {
          NS_LOG_INFO (this << " failed to send request for embedded object,"
                            << " GetErrNo= " << m_socket->GetErrno () << ","
                            << " waiting for another Tx opportunity");
          /// \todo What to do if it fails here?
        }
      else
        {
          SwitchToState (EXPECTING_MAIN_OBJECT);
        }
    }
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                        << " for RequestMainObject");
    }

} // end of `void RequestMainObject ()`


void
HttpClient::RequestEmbeddedObject ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == PARSING_MAIN_OBJECT || m_state == EXPECTING_EMBEDDED_OBJECT)
    {
      HttpEntityHeader httpEntity;
      httpEntity.SetContentLength (0); // request does not need content length
      httpEntity.SetContentType (HttpEntityHeader::EMBEDDED_OBJECT);
      uint32_t requestSize = m_httpVariables->GetRequestSize ();
      Ptr<Packet> packet = Create<Packet> (requestSize - httpEntity.GetSerializedSize ());
      packet->AddHeader (httpEntity);
      m_txEmbeddedObjectRequestTrace (packet);
      int actualBytes = m_socket->Send (packet);
      NS_LOG_DEBUG (this << " Send() packet " << packet
                         << " of " << packet->GetSize () << " bytes,"
                         << " return value= " << actualBytes);

      if ((unsigned) actualBytes != requestSize)
        {
          NS_LOG_INFO (this << " failed to send request for embedded object,"
                            << " GetErrNo= " << m_socket->GetErrno () << ","
                            << " waiting for another Tx opportunity");
          /// \todo What to do if it fails here?
        }
      else
        {
          m_embeddedObjectsToBeRequested--;
          SwitchToState (EXPECTING_EMBEDDED_OBJECT);
        }
    }
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                        << " for RequestEmbeddedObject");
    }

} // end of `void RequestEmbeddedObject ()`


void
HttpClient::ReceiveMainObject (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  if (m_state == EXPECTING_MAIN_OBJECT)
    {
      Ptr<Packet> packetCopy = packet->Copy ();
      HttpEntityHeader httpEntity;
      packetCopy->RemoveHeader (httpEntity);

      if (httpEntity.GetContentType () == HttpEntityHeader::MAIN_OBJECT)
        {
          m_rxMainObjectPacketTrace (packet);
          uint32_t contentLength = httpEntity.GetContentLength ();
          NS_LOG_DEBUG (this << " received a main object packet"
                             << " with Content-Length= " << contentLength);

          if (contentLength > packetCopy->GetSize ())
            {
              /*
               * There are more packets of this main object, so just stay still
               * and wait until they arrive.
               */
            }
          else
            {
              // this is the last packet of this main object
              // acknowledge the reception of a whole main object
              NS_LOG_INFO (this << " finished receiving a main object");
              m_rxMainObjectTrace (packet);
              EnterParsingTime ();
            }
        }
      else
        {
          NS_LOG_WARN (this << " Invalid packet header");
        }

    } // end of `if (m_state == EXPECTING_MAIN_OBJECT)`
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                        << " for ReceiveMainObject");
    }

} // end of `void ReceiveMainObject (Ptr<Packet> packet)`


void
HttpClient::ReceiveEmbeddedObject (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  if (m_state == EXPECTING_EMBEDDED_OBJECT)
    {
      Ptr<Packet> packetCopy = packet->Copy ();
      HttpEntityHeader httpEntity;
      packetCopy->RemoveHeader (httpEntity);

      if (httpEntity.GetContentType () == HttpEntityHeader::EMBEDDED_OBJECT)
        {
          m_rxEmbeddedObjectPacketTrace (packet);
          uint32_t contentLength = httpEntity.GetContentLength ();
          NS_LOG_DEBUG (this << " received an embedded object packet"
                             << " with Content-Length= " << contentLength);

          if (contentLength > packetCopy->GetSize ())
            {
              /*
               * There are more packets of this embedded object, so just stay
               * still and wait until they arrive.
               */
            }
          else
            {
              // this is the last packet of this embedded object
              // acknowledge the reception of a whole embedded object
              NS_LOG_INFO (this << " finished receiving an embedded object");
              m_rxEmbeddedObjectTrace (packet);

              if (m_embeddedObjectsToBeRequested > 0)
                {
                  NS_LOG_INFO (this << " " << m_embeddedObjectsToBeRequested
                                    << " more embedded object(s) to be requested");
                  Simulator::ScheduleNow (&HttpClient::RequestEmbeddedObject,
                                          this);
                }
              else
                {
                  /*
                   * There is no more embedded object, the web page has been
                   * downloaded completely. Now is the time to read it.
                   */
                  EnterReadingTime ();
                }
            }
        }
      else
        {
          NS_LOG_WARN (this << " Invalid packet header");
        }

    } // end of `if (m_state == EXPECTING_EMBEDDED_OBJECT)`
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                   << " for ReceiveEmbeddedObject");
    }

} // end of `void ReceiveEmbeddedObject (Ptr<Packet> packet)`


void
HttpClient::EnterParsingTime ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == EXPECTING_MAIN_OBJECT)
    {
      Time parsingTime = m_httpVariables->GetParsingTime ();
      NS_LOG_INFO (this << " the parsing of this main object"
                        << " will complete in "
                        << parsingTime.GetSeconds () << " seconds");
      Simulator::Schedule (parsingTime, &HttpClient::ParseMainObject, this);
      SwitchToState (PARSING_MAIN_OBJECT);
    }
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                   << " for EnterParsingTime");
    }

}


void
HttpClient::ParseMainObject ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == PARSING_MAIN_OBJECT)
    {
      m_embeddedObjectsToBeRequested = m_httpVariables->GetNumOfEmbeddedObjects ();
      NS_LOG_INFO (this << " parsing has determined "
                        << m_embeddedObjectsToBeRequested << " embedded object(s)"
                        << " in the main object");

      if (m_embeddedObjectsToBeRequested > 0)
        {
          Simulator::ScheduleNow (&HttpClient::RequestEmbeddedObject, this);
        }
      else
        {
          /*
           * There is no embedded object in the main object. So sit back and
           * enjoy the plain web page.
           */
          EnterReadingTime ();
        }

    }
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                   << " for ParseMainObject");
    }
}


void
HttpClient::EnterReadingTime ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == EXPECTING_EMBEDDED_OBJECT || m_state == PARSING_MAIN_OBJECT)
    {
      Time readingTime = m_httpVariables->GetReadingTime ();
      NS_LOG_INFO (this << " will finish reading this web page in "
                        << readingTime.GetSeconds () << " seconds");
      // schedule a request of another main object once the reading time expires
      Simulator::Schedule (readingTime, &HttpClient::RequestMainObject, this);
      SwitchToState (READING);
    }
  else
    {
      NS_LOG_WARN (this << " Invalid state " << GetStateString ()
                   << " for EnterReadingTime");
    }

}


void
HttpClient::SwitchToState (HttpClient::State_t state)
{
  std::string oldState = GetStateString ();
  std::string newState = GetStateString (state);
  NS_LOG_FUNCTION (this << oldState << newState);
  m_state = state;
  NS_LOG_INFO (this << " HttpClient " << oldState << " --> " << newState);
  m_stateTransitionTrace (oldState, newState);
}


} // end of `namespace ns3`

