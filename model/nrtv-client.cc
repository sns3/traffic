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

#include "nrtv-client.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/config.h>
#include <ns3/pointer.h>
#include <ns3/uinteger.h>
#include <ns3/nrtv-variables.h>
#include <ns3/nrtv-header.h>
#include <ns3/seq-ts-header.h>
#include <ns3/packet.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/udp-socket-factory.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/unused.h>


NS_LOG_COMPONENT_DEFINE ("NrtvClient");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrtvClient);


NrtvClient::NrtvClient ()
  : m_state (NOT_STARTED),
    m_socket (0),
    m_nrtvVariables (CreateObject<NrtvVariables> ())
{
  NS_LOG_FUNCTION (this);

  m_dejitterBufferWindowSize = m_nrtvVariables->GetDejitterBufferWindowSize ();
  NS_LOG_INFO (this << " this client application uses"
                    << " a de-jitter buffer window size of "
                    << m_dejitterBufferWindowSize.GetSeconds () << " seconds");
}


NrtvClient::~NrtvClient ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
NrtvClient::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrtvClient")
    .SetParent<Application> ()
    .AddConstructor<NrtvClient> ()
    .AddAttribute ("Variables",
                   "Pointer to random number generator",
                   PointerValue (),
                   MakePointerAccessor (&NrtvClient::m_nrtvVariables),
                   MakePointerChecker<NrtvVariables> ())
    .AddAttribute ("RemoteServerAddress",
                   "The address of the destination server",
                   AddressValue (),
                   MakeAddressAccessor (&NrtvClient::m_remoteServerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemoteServerPort",
                   "The destination port of the outbound packets",
                   UintegerValue (1935), // the default port for Adobe Flash video
                   MakeUintegerAccessor (&NrtvClient::m_remoteServerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Protocol",
                   "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&NrtvClient::m_protocol),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Rx",
                     "A packet of has been received",
                     MakeTraceSourceAccessor (&NrtvClient::m_rxTrace))
    .AddTraceSource ("RxSlice",
                     "Received a whole slice",
                     MakeTraceSourceAccessor (&NrtvClient::m_rxSliceTrace))
    .AddTraceSource ("RxFrame",
                     "Received a whole frame",
                     MakeTraceSourceAccessor (&NrtvClient::m_rxFrameTrace))
    .AddTraceSource ("StateTransition",
                     "Trace fired upon every NRTV client state transition",
                     MakeTraceSourceAccessor (&NrtvClient::m_stateTransitionTrace))
  ;
  return tid;
}


Address
NrtvClient::GetRemoteServerAddress () const
{
  return m_remoteServerAddress;
}


uint16_t
NrtvClient::GetRemoteServerPort () const
{
  return m_remoteServerPort;
}


NrtvClient::State_t
NrtvClient::GetState () const
{
  return m_state;
}


std::string
NrtvClient::GetStateString () const
{
  return GetStateString (m_state);
}


std::string
NrtvClient::GetStateString (NrtvClient::State_t state)
{
  switch (state)
  {
    case NOT_STARTED:
      return "NOT_STARTED";
      break;
    case CONNECTING:
      return "CONNECTING";
      break;
    case RECEIVING:
      return "RECEIVING";
      break;
    case IDLE:
      return "IDLE";
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
NrtvClient::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  if (!Simulator::IsFinished ()) // guard against canceling out-of-bound events
    {
      StopApplication ();
    }

  Application::DoDispose (); // chain up
}


void
NrtvClient::StartApplication ()
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
NrtvClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  SwitchToState (STOPPED);
  CancelAllPendingEvents ();
  CloseConnection ();
}


void
NrtvClient::ConnectionSucceededCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (m_state == CONNECTING)
    {
      NS_ASSERT_MSG (m_socket == socket, "Invalid socket");
      socket->SetRecvCallback (MakeCallback (&NrtvClient::ReceivedDataCallback,
                                             this));
      SwitchToState (RECEIVING);
    }
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for ConnectionSucceeded");
    }
}


void
NrtvClient::ConnectionFailedCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (m_state == CONNECTING)
    {
      if (socket->GetErrno () != Socket::ERROR_NOTERROR)
        {
          m_eventRetryConnection = Simulator::ScheduleNow (
            &NrtvClient::RetryConnection, this);
        }
    }
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for ConnectionFailed");
    }
}


void
NrtvClient::NormalCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  CancelAllPendingEvents ();
  SwitchToState (IDLE);
  Time idleTime = m_nrtvVariables->GetIdleTime ();
  NS_LOG_INFO (this << " a video has just completed, now waiting for "
                    << idleTime.GetSeconds () << " seconds before the next video");
  Simulator::Schedule (idleTime, &NrtvClient::OpenConnection, this);
}


void
NrtvClient::ErrorCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  CancelAllPendingEvents ();
  m_eventRetryConnection = Simulator::ScheduleNow (
    &NrtvClient::RetryConnection, this);
  /// \todo This won't work because the socket is already closed
}


void
NrtvClient::ReceivedDataCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (m_state == RECEIVING)
    {
      Ptr<Packet> packet;
      Address from;

      while ((packet = socket->RecvFrom (from)))
        {
          if (packet->GetSize () == 0)
            {
              break; // EOF
            }

#ifdef NS3_LOG_ENABLE
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
#endif /* NS3_LOG_ENABLE */

          m_rxTrace (packet);
          Receive (packet);

        } // end of `while ((packet = socket->RecvFrom (from)))`

    } // end of  `if (m_state == RECEIVING)`
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for ReceivedData");
    }

} // end of `void ReceivedDataCallback (Ptr<Socket> socket)`


void
NrtvClient::OpenConnection ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == NOT_STARTED || m_state == IDLE)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_protocol);
      NS_LOG_INFO (this << " created socket " << m_socket
                        << " of " << m_protocol.GetName ());
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

      NS_UNUSED (ret); // mute compiler warning
      NS_ASSERT_MSG (m_socket != 0, "Failed creating socket");
      m_socket->ShutdownSend ();
      m_socket->SetConnectCallback (MakeCallback (&NrtvClient::ConnectionSucceededCallback,
                                                  this),
                                    MakeCallback (&NrtvClient::ConnectionFailedCallback,
                                                  this));
      m_socket->SetCloseCallbacks (MakeCallback (&NrtvClient::NormalCloseCallback,
                                                 this),
                                   MakeCallback (&NrtvClient::ErrorCloseCallback,
                                                 this));
      m_socket->SetRecvCallback (MakeCallback (&NrtvClient::ReceivedDataCallback,
                                               this));
      SwitchToState (CONNECTING);

    } // end of `if (m_state == NOT_STARTED)`
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for OpenConnection");
    }

} // end of `void OpenConnection ()`


void
NrtvClient::RetryConnection ()
{
  NS_LOG_FUNCTION (this);

  if (Ipv4Address::IsMatchingType (m_remoteServerAddress))
    {
      Ipv4Address ipv4 = Ipv4Address::ConvertFrom (m_remoteServerAddress);
      InetSocketAddress inetSocket = InetSocketAddress (ipv4,
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

  SwitchToState (CONNECTING);

} // end of `void RetryConnection ()`


void
NrtvClient::CloseConnection ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}


uint32_t
NrtvClient::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  /*
   * The headers of the slice (packet) consist of SeqTsHeader and NrtvHeader.
   * Unfortunately we can't avoid hard-coding the size of SeqTsHeader (12 bytes)
   * here.
   */
  uint32_t headerSize = 12 + NrtvHeader::GetStaticSerializedSize ();
  NS_ASSERT_MSG (packet->GetSize () >= headerSize,
                 "Invalid packet, it is too small");

  SeqTsHeader seqTsHeader;
  packet->RemoveHeader (seqTsHeader);
  NS_ASSERT_MSG (seqTsHeader.GetSeq () > 0,
                 "Invalid SeqTs header, seq= " << seqTsHeader.GetSeq ());

  NrtvHeader nrtvHeader;
  packet->RemoveHeader (nrtvHeader);
  NS_ASSERT_MSG (nrtvHeader.GetFrameNumber () > 0,
                 "Invalid NRTV header, frameNumber= " << nrtvHeader.GetFrameNumber ());

  uint16_t sliceNumber = nrtvHeader.GetSliceNumber ();
  uint16_t numOfSlices = nrtvHeader.GetNumOfSlices ();
  NS_LOG_INFO (this << " received slice " << sliceNumber
                    << " out of " << numOfSlices << " slices");
  m_rxSliceTrace (sliceNumber, numOfSlices);

  if (sliceNumber == numOfSlices)
    {
      // this is the last slice, hence we just received a complete frame
      uint32_t frameNumber = nrtvHeader.GetFrameNumber ();
      uint32_t numOfFrames = nrtvHeader.GetNumOfFrames ();
      NS_LOG_INFO (this << " received frame " << frameNumber
                        << " out of " << numOfFrames << " frames");
      m_rxFrameTrace (frameNumber, numOfFrames);

      if (frameNumber == numOfFrames)
        {
          // this is the last frame
        }
    }

  return packet->GetSize ();

} // end of `uint32_t Receive (packet)`


void
NrtvClient::CancelAllPendingEvents ()
{
  NS_LOG_FUNCTION (this);

  if (!Simulator::IsExpired (m_eventRetryConnection))
    {
      NS_LOG_INFO (this << " canceling RetryConnection which is due in "
                        << Simulator::GetDelayLeft (m_eventRetryConnection).GetSeconds ()
                        << " seconds");
      Simulator::Cancel (m_eventRetryConnection);
    }
}


void
NrtvClient::SwitchToState (NrtvClient::State_t state)
{
  std::string oldState = GetStateString ();
  std::string newState = GetStateString (state);
  NS_LOG_FUNCTION (this << oldState << newState);

  m_state = state;
  NS_LOG_INFO (this << " NrtvClient " << oldState << " --> " << newState);
  m_stateTransitionTrace (oldState, newState);
}


} // end of `namespace ns3`


