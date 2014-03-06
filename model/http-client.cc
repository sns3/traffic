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
#include <ns3/config.h>
#include <ns3/pointer.h>
#include <ns3/uinteger.h>
#include <ns3/http-variables.h>
#include <ns3/http-seq-ts-tag.h>
#include <ns3/packet.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/unused.h>


NS_LOG_COMPONENT_DEFINE ("HttpClient");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HttpClient);


HttpClient::HttpClient ()
  : m_state (NOT_STARTED),
    m_socket (0),
    m_objectBytesToBeReceived (0),
    m_objectArrivalTime (MilliSeconds (0)),
    m_embeddedObjectsToBeRequested (0),
    m_httpVariables (CreateObject<HttpVariables> ())
{
  NS_LOG_FUNCTION (this);

  m_isBurstMode = m_httpVariables->IsBurstMode ();

  if (m_isBurstMode)
    {
      NS_LOG_INFO (this << " this client application uses HTTP 1.0 (burst mode)");
    }
  else
    {
      NS_LOG_INFO (this << " this client application uses HTTP 1.1 (persistent mode)");
    }
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
    .AddAttribute ("RemoteServerPort",
                   "The destination port of the outbound packets",
                   UintegerValue (80), // the default HTTP port
                   MakeUintegerAccessor (&HttpClient::m_remoteServerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Protocol",
                   "The type of protocol to use. The attribute is here to "
                   "accommodate different protocols in the future. At the "
                   "moment, only ns3::TcpSocketFactory is supported.",
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
    .AddTraceSource ("Rx",
                     "General trace for receiving a packet of any kind",
                     MakeTraceSourceAccessor (&HttpClient::m_rxTrace))
    .AddTraceSource ("RxDelay",
                     "General trace of delay for receiving a complete object",
                     MakeTraceSourceAccessor (&HttpClient::m_rxDelayTrace))
  ;
  return tid;
}


Time
HttpClient::GetStartTime () const
{
  return m_startTime;
}


Time
HttpClient::GetStopTime () const
{
  return m_stopTime;
}


bool
HttpClient::IsScheduledToStop () const
{
  return (m_stopTime != TimeStep (0));
}


Address
HttpClient::GetRemoteServerAddress () const
{
  return m_remoteServerAddress;
}


uint16_t
HttpClient::GetRemoteServerPort () const
{
  return m_remoteServerPort;
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
  switch (state)
    {
    case NOT_STARTED:
      return "NOT_STARTED";
      break;
    case CONNECTING:
      return "CONNECTING";
      break;
    case EXPECTING_MAIN_OBJECT:
      return "EXPECTING_MAIN_OBJECT";
      break;
    case PARSING_MAIN_OBJECT:
      return "PARSING_MAIN_OBJECT";
      break;
    case EXPECTING_EMBEDDED_OBJECT:
      return "EXPECTING_EMBEDDED_OBJECT";
      break;
    case READING:
      return "READING";
      break;
    case STOPPED:
      return "STOPPED";
      break;
    default:
      NS_FATAL_ERROR ("Unknown state");
      break;
    }
}


void
HttpClient::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  if (!Simulator::IsFinished ()) // guard against canceling out-of-bound events
    {
      StopApplication ();
    }

  Application::DoDispose (); // chain up
}


void
HttpClient::StartApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == NOT_STARTED)
    {
      OpenConnection ();
    }
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for StartApplication");
    }
}


void
HttpClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  SwitchToState (STOPPED);
  CancelAllPendingEvents ();
  CloseConnection ();
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

      if (m_embeddedObjectsToBeRequested > 0)
        {
          /*
           * This case is for burst mode: after parsing a main object or after
           * receiving an object but need to request more.
           */
          m_eventRequestEmbeddedObject = Simulator::ScheduleNow (
              &HttpClient::RequestEmbeddedObject, this);
        }
      else
        {
          /*
           * This case is for the first connection attempt or for burst mode
           * after reading.
           */
          m_eventRequestMainObject = Simulator::ScheduleNow (
              &HttpClient::RequestMainObject, this);
        }
    }
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for ConnectionSucceeded");
    }
}


void
HttpClient::ConnectionFailedCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (m_state == CONNECTING)
    {
      if (socket->GetErrno () != Socket::ERROR_NOTERROR)
        {
          m_eventRetryConnection = Simulator::ScheduleNow (
              &HttpClient::RetryConnection, this);
        }
    }
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for ConnectionFailed");
    }
}


void
HttpClient::NormalCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  CancelAllPendingEvents ();
  if (socket->GetErrno () != Socket::ERROR_NOTERROR)
    {
      m_eventRetryConnection = Simulator::ScheduleNow (
          &HttpClient::RetryConnection, this);
      /// \todo This won't work because the socket is already closed
    }
}


void
HttpClient::ErrorCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  CancelAllPendingEvents ();
  if (socket->GetErrno () != Socket::ERROR_NOTERROR)
    {
      m_eventRetryConnection = Simulator::ScheduleNow (
          &HttpClient::RetryConnection, this);
      /// \todo This won't work because the socket is already closed
    }
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

      m_rxTrace (packet, from);

      switch (m_state)
        {
        case EXPECTING_MAIN_OBJECT:
          ReceiveMainObject (packet, from);
          break;
        case EXPECTING_EMBEDDED_OBJECT:
          ReceiveEmbeddedObject (packet, from);
          break;
        default:
          NS_LOG_WARN (this << " invalid state " << GetStateString ()
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
HttpClient::OpenConnection ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == NOT_STARTED || m_state == EXPECTING_EMBEDDED_OBJECT
      || m_state == PARSING_MAIN_OBJECT || m_state == READING)
    {
      NS_ASSERT_MSG (m_protocol == TcpSocketFactory::GetTypeId (),
                     "Protocols other than TCP are not supported");
      m_socket = Socket::CreateSocket (GetNode (), m_protocol);

#ifdef NS3_LOG_ENABLE
      UintegerValue mtu;
      m_socket->GetAttribute ("SegmentSize", mtu);
      NS_LOG_INFO (this << " created socket " << m_socket
                        << " of " << m_protocol.GetName ()
                        << " with MTU of " << mtu.Get () << " bytes");
      NS_UNUSED (mtu);
#endif /* NS3_LOG_ENABLE */

      int ret;

      if (Ipv4Address::IsMatchingType (m_remoteServerAddress))
        {
          ret = m_socket->Bind ();
          NS_LOG_DEBUG (this << " Bind() return value= " << ret
                             << " GetErrNo= " << m_socket->GetErrno ());

          const Ipv4Address ipv4 = Ipv4Address::ConvertFrom (m_remoteServerAddress);
          const InetSocketAddress inetSocket = InetSocketAddress (ipv4,
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

          const Ipv6Address ipv6 = Ipv6Address::ConvertFrom (m_remoteServerAddress);
          const Inet6SocketAddress inet6Socket = Inet6SocketAddress (ipv6,
                                                                     m_remoteServerPort);
          NS_LOG_INFO (this << " connecting to " << ipv6
                            << " port " << m_remoteServerPort
                            << " / " << inet6Socket);
          ret = m_socket->Connect (inet6Socket);
          NS_LOG_DEBUG (this << " Connect() return value= " << ret
                             << " GetErrNo= " << m_socket->GetErrno ());
        }

      NS_UNUSED (ret); // mute compiler warning
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

    } // end of `if (m_state == {NOT_STARTED, EXPECTING_EMBEDDED_OBJECT, PARSING_MAIN_OBJECT, READING})`
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for OpenConnection");
    }

} // end of `void OpenConnection ()`


void
HttpClient::RetryConnection ()
{
  NS_LOG_FUNCTION (this);

  if (Ipv4Address::IsMatchingType (m_remoteServerAddress))
    {
      const Ipv4Address ipv4 = Ipv4Address::ConvertFrom (m_remoteServerAddress);
      const InetSocketAddress inetSocket = InetSocketAddress (ipv4,
                                                              m_remoteServerPort);
      NS_LOG_INFO (this << " retrying connecting to " << ipv4
                        << " port " << m_remoteServerPort
                        << " / " << inetSocket);
      int ret = m_socket->Connect (inetSocket);
      NS_LOG_DEBUG (this << " Connect() return value= " << ret
                         << " GetErrNo= " << m_socket->GetErrno ());
      NS_UNUSED (ret);
    }
  else if (Ipv6Address::IsMatchingType (m_remoteServerAddress))
    {
      const Ipv6Address ipv6 = Ipv6Address::ConvertFrom (m_remoteServerAddress);
      const Inet6SocketAddress inet6Socket = Inet6SocketAddress (ipv6,
                                                                 m_remoteServerPort);
      NS_LOG_INFO (this << " retrying connection to " << ipv6
                        << " port " << m_remoteServerPort
                        << " / " << inet6Socket);
      int ret = m_socket->Connect (inet6Socket);
      NS_LOG_DEBUG (this << " Connect() return value= " << ret
                         << " GetErrNo= " << m_socket->GetErrno ());
      NS_UNUSED (ret);
    }

  SwitchToState (CONNECTING);

} // end of `void RetryConnection ()`


void
HttpClient::CloseConnection ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetConnectCallback (MakeNullCallback<void, Ptr<Socket> > (),
                                    MakeNullCallback<void, Ptr<Socket> > ());
      m_socket->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (),
                                   MakeNullCallback<void, Ptr<Socket> > ());
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
    }
}


void
HttpClient::RequestMainObject ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == CONNECTING || m_state == READING)
    {
      HttpEntityHeader httpEntity;
      httpEntity.SetContentLength (0); // request does not need content length
      httpEntity.SetContentType (HttpEntityHeader::MAIN_OBJECT);
      const uint32_t requestSize = m_httpVariables->GetRequestSize ();
      Ptr<Packet> packet = Create<Packet> (requestSize);
      packet->AddHeader (httpEntity);
      const uint32_t packetSize = packet->GetSize ();
      NS_ASSERT_MSG (packetSize <= 536, // hard-coded MTU size
                     "Packet size shall not be larger than MTU size");
      m_txMainObjectRequestTrace (packet);
      const int actualBytes = m_socket->Send (packet);
      NS_LOG_DEBUG (this << " Send() packet " << packet
                         << " of " << packet->GetSize () << " bytes,"
                         << " return value= " << actualBytes);

      if ((unsigned) actualBytes != packetSize)
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
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for RequestMainObject");
    }

} // end of `void RequestMainObject ()`


void
HttpClient::RequestEmbeddedObject ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == CONNECTING || m_state == PARSING_MAIN_OBJECT
      || m_state == EXPECTING_EMBEDDED_OBJECT)
    {
      if (m_embeddedObjectsToBeRequested > 0)
        {
          HttpEntityHeader httpEntity;
          httpEntity.SetContentLength (0); // request does not need content length
          httpEntity.SetContentType (HttpEntityHeader::EMBEDDED_OBJECT);
          const uint32_t requestSize = m_httpVariables->GetRequestSize ();
          Ptr<Packet> packet = Create<Packet> (requestSize);
          packet->AddHeader (httpEntity);
          const uint32_t packetSize = packet->GetSize ();
          NS_ASSERT_MSG (packetSize <= 536, // hard-coded MTU size
                         "Packet size shall not be larger than MTU size");
          m_txEmbeddedObjectRequestTrace (packet);
          const int actualBytes = m_socket->Send (packet);
          NS_LOG_DEBUG (this << " Send() packet " << packet
                             << " of " << packet->GetSize () << " bytes,"
                             << " return value= " << actualBytes);

          if ((unsigned) actualBytes != packetSize)
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
          NS_LOG_WARN (this << " no embedded object to be requested");
        }
    }
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for RequestEmbeddedObject");
    }

} // end of `void RequestEmbeddedObject ()`


void
HttpClient::ReceiveMainObject (Ptr<Packet> packet, const Address &from)
{
  NS_LOG_FUNCTION (this << packet << from);

  if (m_state == EXPECTING_MAIN_OBJECT)
    {
      // m_objectBytesToBeReceived will be updated below
      // m_objectArrivalTime may be updated below
      Receive (packet, HttpEntityHeader::MAIN_OBJECT);
      m_rxMainObjectPacketTrace (packet);

      if (m_objectBytesToBeReceived > 0)
        {
          /*
           * There are more packets of this main object, so just stay still
           * and wait until they arrive.
           */
          NS_LOG_INFO (this << " " << m_objectBytesToBeReceived << " byte(s)"
                            << " remains from this chunk of main object");
        }
      else
        {
          /*
           * This is the last packet of this main object. Acknowledge the
           * reception of a whole main object
           */
          NS_LOG_INFO (this << " finished receiving a main object");
          m_rxMainObjectTrace ();

          if (!m_objectArrivalTime.IsZero ())
            {
              m_rxDelayTrace (Simulator::Now () - m_objectArrivalTime, from);
              m_objectArrivalTime = MilliSeconds (0); // reset back to zero
            }

          if (m_isBurstMode)
            {
              CloseConnection ();
            }

          EnterParsingTime ();
        }

    } // end of `if (m_state == EXPECTING_MAIN_OBJECT)`
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for ReceiveMainObject");
    }

} // end of `void ReceiveMainObject (Ptr<Packet> packet)`


void
HttpClient::ReceiveEmbeddedObject (Ptr<Packet> packet, const Address &from)
{
  NS_LOG_FUNCTION (this << packet << from);

  if (m_state == EXPECTING_EMBEDDED_OBJECT)
    {
      // m_objectBytesToBeReceived will be updated below
      // m_objectArrivalTime may be updated below
      Receive (packet, HttpEntityHeader::EMBEDDED_OBJECT);
      m_rxEmbeddedObjectPacketTrace (packet);

      if (m_objectBytesToBeReceived > 0)
        {
          /*
           * There are more packets of this embedded object, so just stay
           * still and wait until they arrive.
           */
          NS_LOG_INFO (this << " " << m_objectBytesToBeReceived << " byte(s)"
                            << " remains from this chunk of embedded object");
        }
      else
        {
          /*
           * This is the last packet of this embedded object. Acknowledge
           * the reception of a whole embedded object
           */
          NS_LOG_INFO (this << " finished receiving an embedded object");
          m_rxEmbeddedObjectTrace ();

          if (!m_objectArrivalTime.IsZero ())
            {
              m_rxDelayTrace (Simulator::Now () - m_objectArrivalTime, from);
              m_objectArrivalTime = MilliSeconds (0); // reset back to zero
            }

          if (m_isBurstMode)
            {
              CloseConnection ();
            }

          if (m_embeddedObjectsToBeRequested > 0)
            {
              NS_LOG_INFO (this << " " << m_embeddedObjectsToBeRequested
                                << " more embedded object(s) to be requested");

              if (m_isBurstMode)
                {
                  // open a new connection
                  m_eventRequestEmbeddedObject = Simulator::ScheduleNow (
                      &HttpClient::OpenConnection, this);
                  // RequestEmbeddedObject will follow after connection is established
                }
              else
                {
                  // immediately request another using the existing connection
                  m_eventRequestEmbeddedObject = Simulator::ScheduleNow (
                      &HttpClient::RequestEmbeddedObject, this);
                }
            }
          else
            {
              /*
               * There is no more embedded object, the web page has been
               * downloaded completely. Now is the time to read it.
               */
              EnterReadingTime ();
            }

        } // end of else of `if (m_objectBytesToBeReceived > 0)`

    } // end of `if (m_state == EXPECTING_EMBEDDED_OBJECT)`
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for ReceiveEmbeddedObject");
    }

} // end of `void ReceiveEmbeddedObject (Ptr<Packet> packet)`


uint32_t
HttpClient::Receive (Ptr<Packet> packet,
                     HttpEntityHeader::ContentType_t expectedContentType)
{
  NS_LOG_FUNCTION (this << packet << expectedContentType);

  uint32_t rxSize; // the number of bytes to be received from this packet

  if (packet->GetSize () < HttpEntityHeader::GetStaticSerializedSize ())
    {
      /*
       * Which means that the packet does not contain any header. It is then
       * regarded as a continuation of a previous packet. The whole packet is
       * relevant to be received.
       */
      rxSize = packet->GetSize ();
    }
  else
    {
      // some header might exists in the packet, let's take a peek...
      HttpEntityHeader httpEntity;
      packet->PeekHeader (httpEntity);

      if (httpEntity.GetContentType () == expectedContentType)
        {
          NS_LOG_DEBUG (this << " received a packet with Content-Length= "
                             << httpEntity.GetContentLength ());

          if (m_objectBytesToBeReceived > 0)
            {
              NS_LOG_WARN (this << " new chunk of object is received,"
                                << " although we are still expecting "
                                << m_objectBytesToBeReceived << " bytes"
                                << " of the previous object");
            }

          m_objectBytesToBeReceived += httpEntity.GetContentLength ();
          NS_ASSERT_MSG (packet->GetSize () >= httpEntity.GetSerializedSize (),
                         "Received an invalid packet");
          rxSize = packet->GetSize () - httpEntity.GetSerializedSize ();

          // see if the packet also contains an HttpSeqTsTag
          HttpSeqTsTag tag;
          bool isTagged = false;
          ByteTagIterator it = packet->GetByteTagIterator ();
          while (!isTagged && it.HasNext ())
            {
              ByteTagIterator::Item item = it.Next ();
              if (item.GetTypeId () == HttpSeqTsTag::GetTypeId ())
                {
                  NS_LOG_DEBUG (this << " contains a SeqTs tag:"
                                     << " start=" << item.GetStart ()
                                     << " end=" << item.GetEnd ());
                  item.GetTag (tag);
                  m_objectArrivalTime = tag.GetTs ();
                  isTagged = true;
                }
            }

          if (!isTagged)
            {
              NS_LOG_WARN (this << " expected an SeqTs tag, but not found");
            }

        }
      else if (httpEntity.GetContentType () == HttpEntityHeader::NOT_SET)
        {
          /*
           * This packet is a continuation of a previous packet. The packet
           * does not contain any header, so the whole packet is relevant to be
           * received.
           */
          rxSize = packet->GetSize ();
        }
      else
        {
          NS_LOG_WARN (this << " invalid packet header");
          rxSize = 0; // don't receive anything from this packet
        }

    } // end of else of `if (packet->GetSize () < HttpEntityHeader::GetStaticSerializedSize ())`

  if (m_objectBytesToBeReceived < rxSize)
    {
      NS_LOG_WARN (this << " the received packet is larger"
                        << " (" << rxSize << " bytes of content)"
                        << " than it is supposed to be"
                        << " (" << m_objectBytesToBeReceived << " bytes)");
      rxSize = m_objectBytesToBeReceived; // only receive as much as we had expected
      m_objectBytesToBeReceived = 0; // stop expecting any more packet of this object
    }
  else
    {
      m_objectBytesToBeReceived -= rxSize;
    }

  return rxSize;

} // end of `uint32_t Receive (packet, expectedHeader)`


void
HttpClient::EnterParsingTime ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == EXPECTING_MAIN_OBJECT)
    {
      const Time parsingTime = m_httpVariables->GetParsingTime ();
      NS_LOG_INFO (this << " the parsing of this main object"
                        << " will complete in "
                        << parsingTime.GetSeconds () << " seconds");
      m_eventParseMainObject = Simulator::Schedule (
          parsingTime, &HttpClient::ParseMainObject, this);
      SwitchToState (PARSING_MAIN_OBJECT);
    }
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
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
          if (m_isBurstMode)
            {
              // open a new connection
              m_eventRequestEmbeddedObject = Simulator::ScheduleNow (
                  &HttpClient::OpenConnection, this);
              // RequestEmbeddedObject will follow after connection is established
            }
          else
            {
              // immediately request an embedded object using the existing connection
              m_eventRequestEmbeddedObject = Simulator::ScheduleNow (
                  &HttpClient::RequestEmbeddedObject, this);
            }
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
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for ParseMainObject");
    }
}


void
HttpClient::EnterReadingTime ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == EXPECTING_EMBEDDED_OBJECT || m_state == PARSING_MAIN_OBJECT)
    {
      const Time readingTime = m_httpVariables->GetReadingTime ();
      NS_LOG_INFO (this << " will finish reading this web page in "
                        << readingTime.GetSeconds () << " seconds");

      // schedule a request of another main object once the reading time expires
      if (m_isBurstMode)
        {
          // open a new connection
          NS_ASSERT (m_embeddedObjectsToBeRequested == 0);
          m_eventRequestMainObject = Simulator::Schedule (
              readingTime, &HttpClient::OpenConnection, this);
          // RequestMainObject will follow after connection is established
        }
      else
        {
          // reuse the existing connection
          m_eventRequestMainObject = Simulator::Schedule (
              readingTime, &HttpClient::RequestMainObject, this);
        }

      SwitchToState (READING);
    }
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for EnterReadingTime");
    }

}


void
HttpClient::CancelAllPendingEvents ()
{
  NS_LOG_FUNCTION (this);

  if (!Simulator::IsExpired (m_eventRequestMainObject))
    {
      NS_LOG_INFO (this << " canceling RequestMainObject which is due in "
                        << Simulator::GetDelayLeft (m_eventRequestMainObject).GetSeconds ()
                        << " seconds");
      Simulator::Cancel (m_eventRequestMainObject);
    }

  if (!Simulator::IsExpired (m_eventRequestEmbeddedObject))
    {
      NS_LOG_INFO (this << " canceling RequestEmbeddedObject which is due in "
                        << Simulator::GetDelayLeft (m_eventRequestEmbeddedObject).GetSeconds ()
                        << " seconds");
      Simulator::Cancel (m_eventRequestEmbeddedObject);
    }

  if (!Simulator::IsExpired (m_eventRetryConnection))
    {
      NS_LOG_INFO (this << " canceling RetryConnection which is due in "
                        << Simulator::GetDelayLeft (m_eventRetryConnection).GetSeconds ()
                        << " seconds");
      Simulator::Cancel (m_eventRetryConnection);
    }

  if (!Simulator::IsExpired (m_eventParseMainObject))
    {
      NS_LOG_INFO (this << " canceling ParseMainObject which is due in "
                        << Simulator::GetDelayLeft (m_eventParseMainObject).GetSeconds ()
                        << " seconds");
      Simulator::Cancel (m_eventParseMainObject);
    }

}


void
HttpClient::SwitchToState (HttpClient::State_t state)
{
  const std::string oldState = GetStateString ();
  const std::string newState = GetStateString (state);
  NS_LOG_FUNCTION (this << oldState << newState);

  if ((state == EXPECTING_MAIN_OBJECT) || (state == EXPECTING_EMBEDDED_OBJECT))
    {
      if (m_objectBytesToBeReceived > 0)
        {
          NS_FATAL_ERROR ("Cannot start a new receiving session"
                          << " if the previous object"
                          << " (" << m_objectBytesToBeReceived << " bytes)"
                          << " is not completely received yet");
        }
    }

  m_state = state;
  NS_LOG_INFO (this << " HttpClient " << oldState << " --> " << newState);
  m_stateTransitionTrace (oldState, newState);
}


} // end of `namespace ns3`

