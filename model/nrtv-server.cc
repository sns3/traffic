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
#include <ns3/nrtv-header.h>
#include <ns3/seq-ts-header.h>
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
          NS_ASSERT_MSG (m_protocol == TcpSocketFactory::GetTypeId (),
                         "Protocols other than TCP are not supported");
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

  // destroy all workers
  m_workers.clear ();

  // stop listening
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
NrtvServer::NotifyVideoCompleted (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  // remove the worker entry
  std::map<Ptr<Socket>, Ptr<NrtvServerVideoWorker> >::iterator it
    = m_workers.find (socket);
  NS_ASSERT (it != m_workers.end ());
  m_workers.erase (it); // this will destroy the worker and close its socket
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
    m_packetSeq (0),
    m_numOfFramesServed (0),
    m_numOfSlicesServed (0)
{
  NS_LOG_FUNCTION (this << socket);

  // accessing the parent's server random variable
  PointerValue p;
  server->GetAttribute ("Variables", p);
  m_nrtvVariables = p.Get<NrtvVariables> ();
  m_frameInterval = m_nrtvVariables->GetFrameInterval ();  // frame rate
  m_numOfFrames = m_nrtvVariables->GetNumOfFrames ();      // length of video
  NS_ASSERT (m_numOfFrames > 0);
  m_numOfSlices = m_nrtvVariables->GetNumOfSlices ();      // slices per frame
  NS_ASSERT (m_numOfSlices > 0);
  NS_LOG_INFO (this << " this video is " << m_numOfFrames << " frames long"
                    << " (each frame is " << m_frameInterval.GetMilliSeconds ()
                    << " ms long and made of " << m_numOfSlices << " slices)");

  socket->SetCloseCallbacks (MakeCallback (&NrtvServerVideoWorker::NormalCloseCallback,
                                           this),
                             MakeCallback (&NrtvServerVideoWorker::ErrorCloseCallback,
                                           this));

  // start the first frame now
  Simulator::ScheduleNow (&NrtvServerVideoWorker::NewFrame, this);
}


NrtvServerVideoWorker::~NrtvServerVideoWorker ()
{
  NS_LOG_FUNCTION (this);

  // close the socket
  m_socket->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (),
                               MakeNullCallback<void, Ptr<Socket> > ());
  m_socket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
  m_socket->Close ();
}


void
NrtvServerVideoWorker::NormalCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT_MSG (m_socket == socket,
                 "Socket " << m_socket << " is expected, "
                           << "but socket " << socket << " is received");
  m_socket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
  CancelAllPendingEvents (); // cancel any scheduled transmission
  Simulator::ScheduleNow (&NrtvServer::NotifyVideoCompleted,
                          m_server, m_socket); // tell the server I'm done
}


void
NrtvServerVideoWorker::ErrorCloseCallback (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT_MSG (m_socket == socket,
                 "Socket " << m_socket << " is expected, "
                           << "but socket " << socket << " is received");
  m_socket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
  CancelAllPendingEvents (); // cancel any scheduled transmission
  Simulator::ScheduleNow (&NrtvServer::NotifyVideoCompleted,
                          m_server, m_socket); // tell the server I'm done
}


void
NrtvServerVideoWorker::SendCallback (Ptr<Socket> socket,
                                     uint32_t availableBufferSize)
{
  NS_LOG_FUNCTION (this << socket << availableBufferSize);
  NS_ASSERT_MSG (m_socket == socket,
                 "Socket " << m_socket << " is expected, "
                           << "but socket " << socket << " is received");
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
      // inform the server instance
      NS_LOG_INFO (this << " no more frame after this");
      m_eventNewFrame = Simulator::Schedule (m_frameInterval,
                                             &NrtvServer::NotifyVideoCompleted,
                                             m_server, m_socket);
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

  if (encodingDelay < Simulator::GetDelayLeft (m_eventNewFrame))
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

  uint32_t socketSize = m_socket->GetTxAvailable ();
  NS_LOG_DEBUG (this << " socket has " << socketSize
                     << " bytes available for Tx");

  uint32_t sliceSize = m_nrtvVariables->GetSliceSize ();
  NS_LOG_INFO (this << " video slice " << m_numOfSlicesServed
                    << " is " << sliceSize << " bytes");

  /*
   * The headers of the slice (packet) consist of SeqTsHeader and NrtvHeader.
   * Unfortunately we can't avoid hard-coding the size of SeqTsHeader (12 bytes)
   * here.
   */
  uint32_t headerSize = 12 + NrtvHeader::GetStaticSerializedSize ();
  uint32_t contentSize = std::min (sliceSize,
                                   socketSize - headerSize);
  /*
   * We simply assume that our packets are rather small and the socket will
   * always has space to fit these packets.
   */
  NS_ASSERT_MSG (contentSize == sliceSize, "Socket size is too small");

  m_packetSeq++;
  SeqTsHeader seqTsHeader;
  seqTsHeader.SetSeq (m_packetSeq);

  NrtvHeader nrtvHeader;
  nrtvHeader.SetFrameNumber (m_numOfFramesServed);
  nrtvHeader.SetNumOfFrames (m_numOfFrames);
  nrtvHeader.SetSliceNumber (m_numOfSlicesServed);
  nrtvHeader.SetNumOfSlices (m_numOfSlices);

  Ptr<Packet> packet = Create<Packet> (contentSize);
  /*
   * Adding the headers in reverse order, so that when the client receives the
   * packet, the SeqTs header will be the first to be read.
   */
  packet->AddHeader (nrtvHeader);
  packet->AddHeader (seqTsHeader);

  uint32_t packetSize = packet->GetSize ();
  NS_ASSERT (packetSize == (contentSize + headerSize));
  NS_ASSERT (packetSize <= socketSize);
  NS_ASSERT_MSG (packetSize <= 536, // hard-coded MTU size
                 "Packet size shall not be larger than MTU size");

  NS_LOG_INFO (this << " created packet " << packet << " of "
                    << packetSize << " bytes");

#ifdef NS3_LOG_ENABLE
  int actualBytes = m_socket->Send (packet);
  NS_LOG_DEBUG (this << " Send() packet " << packet
                     << " of " << packetSize << " bytes,"
                     << " return value= " << actualBytes);

  if ((unsigned) actualBytes == packetSize)
    {
      // nothing
    }
  else
    {
      /// \todo We don't do retry at the moment, so we just do nothing in this case
      NS_LOG_ERROR (this << " failure in sending packet");
    }
#endif /* NS3_LOG_ENABLE */

#ifndef NS3_LOG_ENABLE
  m_socket->Send (packet);
#endif /* NS3_LOG_ENABLE */

  m_server->m_txTrace (packet);

  // make way for the next slice
  if (m_numOfSlicesServed < m_numOfSlices)
    {
      ScheduleNewSlice ();
    }

} // end of `void NewSlice ()`


void
NrtvServerVideoWorker::CancelAllPendingEvents ()
{
  NS_LOG_FUNCTION (this);

  if (!Simulator::IsExpired (m_eventNewFrame))
    {
      NS_LOG_INFO (this << " canceling NewFrame which is due in "
                        << Simulator::GetDelayLeft (m_eventNewFrame).GetSeconds ()
                        << " seconds");
      Simulator::Cancel (m_eventNewFrame);
    }

  if (!Simulator::IsExpired (m_eventNewSlice))
    {
      NS_LOG_INFO (this << " canceling NewSlice which is due in "
                        << Simulator::GetDelayLeft (m_eventNewSlice).GetSeconds ()
                        << " seconds");
      Simulator::Cancel (m_eventNewSlice);
    }
}


} // end of `namespace ns3`
