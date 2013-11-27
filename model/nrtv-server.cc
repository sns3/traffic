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

#include "nrtv-server.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/config.h>
#include <ns3/pointer.h>
#include <ns3/uinteger.h>
#include <ns3/nrtv-variables.h>
#include <ns3/packet.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/udp-socket.h>
#include <ns3/address-utils.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/unused.h>


NS_LOG_COMPONENT_DEFINE ("NrtvServer");


namespace ns3 {


// NRTV SERVER ////////////////////////////////////////////////////////////////


NS_OBJECT_ENSURE_REGISTERED (NrtvServer);


NrtvServer::NrtvServer ()
  : m_state (NOT_STARTED),
    m_initialSocket (0),
    m_nrtvVariables (CreateObject<NrtvVariables> ())
{
  NS_LOG_FUNCTION (this);
}


NrtvServer::~NrtvServer ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
NrtvServer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrtvServer")
    .SetParent<Application> ()
    .AddConstructor<NrtvServer> ()
    .AddAttribute ("Variables",
                   "Pointer to random number generator",
                   PointerValue (),
                   MakePointerAccessor (&NrtvServer::m_nrtvVariables),
                   MakePointerChecker<NrtvVariables> ())
    .AddAttribute ("LocalAddress",
                   "The local address of the server, "
                   "i.e., the address on which to bind the Rx socket",
                   AddressValue (),
                   MakeAddressAccessor (&NrtvServer::m_localAddress),
                   MakeAddressChecker ())
    .AddAttribute ("LocalPort",
                   "Port on which the application listen for incoming packets",
                   UintegerValue (1935), // the default port for Adobe Flash video
                   MakeUintegerAccessor (&NrtvServer::m_localPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Protocol",
                   "The type of protocol to use.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&NrtvServer::m_protocol),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx",
                     "A packet has been sent",
                     MakeTraceSourceAccessor (&NrtvServer::m_txTrace))
    .AddTraceSource ("StateTransition",
                     "Trace fired upon every HTTP client state transition",
                     MakeTraceSourceAccessor (&NrtvServer::m_stateTransitionTrace))
  ;
  return tid;
}


Address
NrtvServer::GetLocalAddress () const
{
  return m_localAddress;
}


uint16_t
NrtvServer::GetLocalPort () const
{
  return m_localPort;
}


NrtvServer::State_t
NrtvServer::GetState () const
{
  return m_state;
}


std::string
NrtvServer::GetStateString () const
{
  return GetStateString (m_state);
}


std::string
NrtvServer::GetStateString (NrtvServer::State_t state)
{
  switch (state)
  {
    case NOT_STARTED:
      return "NOT_STARTED";
      break;
    case STARTED:
      return "STARTED";
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
NrtvServer::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  if (!Simulator::IsFinished ()) // guard against canceling out-of-bound events
    {
      StopApplication ();
    }

  Application::DoDispose (); // chain up
}


void
NrtvServer::StartApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_state == NOT_STARTED)
    {
      if (m_initialSocket == 0)
        {
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

              if (addressUtils::IsMulticast (inetSocket))
                {
                  Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_initialSocket);
                  if (udpSocket != 0)
                    {
                      // equivalent to setsockopt (MCAST_JOIN_GROUP)
                      udpSocket->MulticastJoinGroup (0, inetSocket);
                    }
                  else
                    {
                      NS_FATAL_ERROR ("Error: Failed to join multicast group");
                    }
                }
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

              if (addressUtils::IsMulticast (inet6Socket))
                {
                  Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_initialSocket);
                  if (udpSocket != 0)
                    {
                      // equivalent to setsockopt (MCAST_JOIN_GROUP)
                      udpSocket->MulticastJoinGroup (0, inet6Socket);
                    }
                  else
                    {
                      NS_FATAL_ERROR ("Error: Failed to join multicast group");
                    }
                }
            }

          /// \todo UDP doesn't need this, returns ERROR_OPNOTSUPP
          ret = m_initialSocket->Listen ();
          NS_LOG_DEBUG (this << " Listen () return value= " << ret
                             << " GetErrNo= " << m_initialSocket->GetErrno ());

          NS_UNUSED (ret);

        } // end of `if (m_initialSocket == 0)`

      NS_ASSERT_MSG (m_initialSocket != 0, "Failed creating socket");
      m_initialSocket->ShutdownRecv ();
      m_initialSocket->SetAcceptCallback (MakeCallback (&NrtvServer::ConnectionRequestCallback,
                                                        this),
                                          MakeCallback (&NrtvServer::NewConnectionCreatedCallback,
                                                        this));
      m_initialSocket->SetCloseCallbacks (MakeCallback (&NrtvServer::NormalCloseCallback,
                                                        this),
                                          MakeCallback (&NrtvServer::ErrorCloseCallback,
                                                        this));
      SwitchToState (STARTED);

    } // end of `if (m_state == NOT_STARTED)`
  else
    {
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for StartApplication");
    }

} // end of `void StartApplication ()`


void
NrtvServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  SwitchToState (STOPPED);

  // close all accepted sockets
  for (std::map<Ptr<Socket>, Ptr<NrtvServerVideoWorker> >::iterator it = m_workers.begin ();
       it != m_workers.end (); ++it)
    {
      it->first->Close ();
      it->first->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
    }

  m_workers.clear ();

  if (m_initialSocket != 0)
    {
      m_initialSocket->Close ();
      m_initialSocket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
    }

}


bool
NrtvServer::ConnectionRequestCallback (Ptr<Socket> socket,
                                       const Address & address)
{
  NS_LOG_FUNCTION (this << socket << address);
  return true; // unconditionally accept the connection request
}


void
NrtvServer::NewConnectionCreatedCallback (Ptr<Socket> socket,
                                          const Address & address)
{
  NS_LOG_FUNCTION (this << socket << address);

  Ptr<NrtvServerVideoWorker> worker = Create<NrtvServerVideoWorker> (this,
                                                                     socket);
  m_workers[socket] = worker;
}


void
NrtvServer::NormalCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (socket == m_initialSocket)
    {
      if (m_state == STARTED)
        {
          NS_FATAL_ERROR ("Initial listener socket shall not be closed when server is still running");
        }
    }
}


void
NrtvServer::ErrorCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (socket == m_initialSocket)
    {
      if (m_state == STARTED)
        {
          NS_FATAL_ERROR ("Initial listener socket shall not be closed when server is still running");
        }
    }
}


void
NrtvServer::SwitchToState (NrtvServer::State_t state)
{
  std::string oldState = GetStateString ();
  std::string newState = GetStateString (state);
  NS_LOG_FUNCTION (this << oldState << newState);
  m_state = state;
  NS_LOG_INFO (this << " NrtvServer " << oldState << " --> " << newState);
  m_stateTransitionTrace (oldState, newState);
}


// NRTV SERVER VIDEO WORKER ///////////////////////////////////////////////////


NrtvServerVideoWorker::NrtvServerVideoWorker (NrtvServer* server,
                                              Ptr<Socket> socket)
  : m_server (server),
    m_socket (socket),
    m_txBufferSize (0),
    m_numOfFramesServed (0),
    m_numOfSlicesServed (0),
    m_isLastFrame (false)
{
  NS_LOG_FUNCTION (this << socket);

  PointerValue p;
  server->GetAttribute ("Variables", p);
  m_nrtvVariables = p.Get<NrtvVariables> ();
  m_frameInterval = m_nrtvVariables->GetFrameInterval ();
  m_numOfFrames = m_nrtvVariables->GetNumOfFrames ();
  m_numOfSlices = m_nrtvVariables->GetNumOfSlices ();
  NS_ASSERT (m_numOfFrames > 0);
  NS_ASSERT (m_numOfSlices > 0);
  NS_LOG_INFO (this << " this video is " << m_numOfFrames << " frames long"
                    << " (each frame is " << m_frameInterval.GetMilliSeconds ()
                    << " ms long and made of " << m_numOfSlices << " slices)");

  socket->SetCloseCallbacks (MakeCallback (&NrtvServerVideoWorker::NormalCloseCallback,
                                           this),
                             MakeCallback (&NrtvServerVideoWorker::ErrorCloseCallback,
                                           this));
  socket->SetSendCallback (MakeCallback (&NrtvServerVideoWorker::SendCallback,
                                         this));

  Simulator::ScheduleNow (&NrtvServerVideoWorker::NewFrame, this);
}


void
NrtvServerVideoWorker::NormalCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT_MSG (m_socket == socket,
                 "Socket " << m_socket << " is expected, "
                           << "but socket " << socket << " is received");
  m_socket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
}


void
NrtvServerVideoWorker::ErrorCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT_MSG (m_socket == socket,
                 "Socket " << m_socket << " is expected, "
                           << "but socket " << socket << " is received");
  m_socket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
}


void
NrtvServerVideoWorker::SendCallback (Ptr<Socket> socket,
                                     uint32_t availableBufferSize)
{
  NS_LOG_FUNCTION (this << socket << availableBufferSize);
  NS_ASSERT_MSG (m_socket == socket,
                 "Socket " << m_socket << " is expected, "
                           << "but socket " << socket << " is received");

  if (availableBufferSize > 0)
    {
      // this must be an unfinished transmission
      ServeFromTxBuffer ();
    }
}


void
NrtvServerVideoWorker::ScheduleNewFrame ()
{
  uint32_t frameNumber = m_numOfFramesServed + 1;
  NS_LOG_FUNCTION (this << frameNumber << m_numOfFrames);
  NS_ASSERT (frameNumber <= m_numOfFrames);

  m_eventNewFrame = Simulator::Schedule (m_frameInterval,
                                         &NrtvServerVideoWorker::NewFrame, this);
  NS_LOG_INFO (this << " video frame " << frameNumber << " will be generated in "
                    << m_frameInterval.GetSeconds () << " seconds");
  NS_UNUSED (frameNumber);
}


void
NrtvServerVideoWorker::NewFrame ()
{
  m_numOfFramesServed++;
  NS_LOG_FUNCTION (this << m_numOfFramesServed << m_numOfFrames);

  if (m_numOfFramesServed < m_numOfFrames)
    {
      ScheduleNewFrame (); // schedule the next frame
    }
  else
    {
      m_isLastFrame = true;
      NS_LOG_INFO ("No more frame after this");
    }

  m_numOfSlicesServed = 0;
  ScheduleNewSlice (); // the first slice of this frame
}


void
NrtvServerVideoWorker::ScheduleNewSlice ()
{
  uint16_t sliceNumber = m_numOfSlicesServed + 1;
  NS_LOG_FUNCTION (this << sliceNumber << m_numOfSlices);
  NS_ASSERT (sliceNumber <= m_numOfSlices);

  Time encodingDelay = m_nrtvVariables->GetSliceEncodingDelay ();
  NS_LOG_DEBUG (this << " encoding the slice needs "
                     << encodingDelay.GetMilliSeconds () << " ms,"
                     << " while new frame is coming in "
                     << Simulator::GetDelayLeft (m_eventNewFrame).GetMilliSeconds () << " ms");

  if (m_isLastFrame
      || (encodingDelay < Simulator::GetDelayLeft (m_eventNewFrame)))
    {
      // still time for a new slice
      NS_LOG_INFO (this << " video slice " << sliceNumber << " will be generated in "
                        << encodingDelay.GetMilliSeconds () << " ms");
      m_eventNewSlice = Simulator::Schedule (encodingDelay,
                                             &NrtvServerVideoWorker::NewSlice,
                                             this);
    }
  else
    {
      // not enough time for another slice
      NS_LOG_LOGIC (this << " " << (m_numOfSlices - m_numOfSlicesServed)
                         << " slices are skipped");
    }

  NS_UNUSED (sliceNumber);
}


void
NrtvServerVideoWorker::NewSlice ()
{
  m_numOfSlicesServed++;
  NS_LOG_FUNCTION (this << m_numOfSlicesServed << m_numOfSlices);

  uint32_t sliceSize = m_nrtvVariables->GetSliceSize ();
  NS_LOG_INFO (this << " video slice " << m_numOfSlicesServed
                    << " is " << sliceSize << " bytes");
  m_txBufferSize += sliceSize;
  ServeFromTxBuffer (); // this will deplete m_txBufferSize

  if (m_numOfSlicesServed < m_numOfSlices)
    {
      ScheduleNewSlice ();
    }
}


uint32_t
NrtvServerVideoWorker::ServeFromTxBuffer ()
{
  NS_LOG_FUNCTION (this);

  uint32_t packetSize = std::min (m_txBufferSize,
                                  m_socket->GetTxAvailable ());
  if (packetSize > 0)
    {
      Ptr<Packet> packet = Create<Packet> (packetSize);
      NS_LOG_INFO (this << " created packet " << packet << " of "
                   << packetSize << " bytes");
      int actualBytes = m_socket->Send (packet);
      NS_LOG_DEBUG (this << " Send() packet " << packet
                    << " of " << packetSize << " bytes,"
                    << " return value= " << actualBytes);
      m_server->m_txTrace (packet);

      if ((unsigned) actualBytes == packetSize)
        {
          // the packet go through successfully
          NS_ASSERT (m_txBufferSize >= packetSize);
          m_txBufferSize -= packetSize;
          NS_LOG_INFO (this << " remaining buffer to be transmitted "
                       << m_txBufferSize << " bytes");
          return packetSize;
        }
      else
        {
          NS_LOG_INFO (this << " failed to send packet,"
                       << " GetErrNo= " << m_socket->GetErrno () << ","
                       << " suspending transmission"
                       << " and waiting for another Tx opportunity");
          return 0;
        }

    } // end of `if (packetSize > 0)`
  else
    {
      return 0;
    }

} // end of `uint32_t ServeFromTxBuffer ()`


} // end of `namespace ns3`
