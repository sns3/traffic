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
#include <ns3/http-seq-ts-tag.h>
#include <ns3/packet.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/unused.h>
#include <ns3/application-delay-probe.h>
#include <ns3/traffic.h>


NS_LOG_COMPONENT_DEFINE ("HttpServer");


namespace ns3 {


// HTTP SERVER ////////////////////////////////////////////////////////////////


NS_OBJECT_ENSURE_REGISTERED (HttpServer);


HttpServer::HttpServer ()
  : m_state (NOT_STARTED),
    m_initialSocket (0),
    m_txBuffer (Create<HttpServerTxBuffer> ()),
    m_httpVariables (CreateObject<HttpVariables> ())
{
  NS_LOG_FUNCTION (this);

  m_mtuSize = m_httpVariables->GetMtuSize ();
  NS_LOG_INFO (this << " MTU size for this server application is "
                    << m_mtuSize << " bytes");
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
    .AddAttribute ("LocalPort",
                   "Port on which the application listen for incoming packets",
                   UintegerValue (80), // the default HTTP port
                   MakeUintegerAccessor (&HttpServer::m_localPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Protocol",
                   "The type of protocol to use. The attribute is here to "
                   "accommodate different protocols in the future. At the "
                   "moment, only ns3::TcpSocketFactory is supported.",
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
                     "A packet has been sent",
                     MakeTraceSourceAccessor (&HttpServer::m_txTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Rx",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&HttpServer::m_rxTrace),
                     "ns3::Packet::PacketAddressTracedCallback")
    .AddTraceSource ("RxDelay",
                     "A packet has been received with delay information",
                     MakeTraceSourceAccessor (&HttpServer::m_rxDelayTrace),
                     "ns3::ApplicationDelayProbe::PacketDelayAddressCallback")
    .AddTraceSource ("StateTransition",
                     "Trace fired upon every HTTP client state transition",
                     MakeTraceSourceAccessor (&HttpServer::m_stateTransitionTrace),
                     "ns3::StateTransitionCallback")
  ;
  return tid;
}


uint32_t
HttpServer::GetMtuSize () const
{
  return m_mtuSize;
}


Address
HttpServer::GetLocalAddress () const
{
  return m_localAddress;
}


uint16_t
HttpServer::GetLocalPort () const
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
          NS_ASSERT_MSG (m_protocol == TcpSocketFactory::GetTypeId (),
                         "Protocols other than TCP are not supported");

          // find the current default MTU value of TCP sockets
          Ptr<const ns3::AttributeValue> previousSocketMtu;
          const TypeId tcpSocketTid = TypeId::LookupByName ("ns3::TcpSocket");
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
#ifdef NS3_LOG_ENABLE
          UintegerValue mtu;
          m_initialSocket->GetAttribute ("SegmentSize", mtu);
          NS_LOG_INFO (this << " created socket " << m_initialSocket
                            << " of " << m_protocol.GetName ()
                            << " with MTU of " << mtu.Get () << " bytes");
          NS_UNUSED (mtu);
#endif /* NS3_LOG_ENABLE */

          Config::SetDefault ("ns3::TcpSocket::SegmentSize",
                              *previousSocketMtu); // reset it back

          int ret;

          if (Ipv4Address::IsMatchingType (m_localAddress))
            {
              const Ipv4Address ipv4 = Ipv4Address::ConvertFrom (m_localAddress);
              const InetSocketAddress inetSocket = InetSocketAddress (ipv4,
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
              const Ipv6Address ipv6 = Ipv6Address::ConvertFrom (m_localAddress);
              const Inet6SocketAddress inet6Socket = Inet6SocketAddress (ipv6,
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
      NS_LOG_WARN (this << " invalid state " << GetStateString ()
                        << " for StartApplication");
    }

} // end of `void StartApplication ()`


void
HttpServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  SwitchToState (STOPPED);

  // close all accepted sockets
  m_txBuffer->CloseAllSockets ();

  // stop listening
  if (m_initialSocket != 0)
    {
      m_initialSocket->Close ();
      m_initialSocket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                                          MakeNullCallback<void, Ptr<Socket>, const Address &> ());
      m_initialSocket->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (),
                                          MakeNullCallback<void, Ptr<Socket> > ());
      m_initialSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_initialSocket->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
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

#ifdef NS3_LOG_ENABLE
  UintegerValue mtu;
  socket->GetAttribute ("SegmentSize", mtu);
  NS_LOG_INFO (this << " new connection from socket " << socket
                    << " with MTU of " << mtu.Get () << " bytes");
  NS_UNUSED (mtu);
#endif /* NS3_LOG_ENABLE */

  socket->SetCloseCallbacks (MakeCallback (&HttpServer::NormalCloseCallback,
                                           this),
                             MakeCallback (&HttpServer::ErrorCloseCallback,
                                           this));
  socket->SetRecvCallback (MakeCallback (&HttpServer::ReceivedDataCallback,
                                         this));
  socket->SetSendCallback (MakeCallback (&HttpServer::SendCallback,
                                         this));

  m_txBuffer->AddSocket (socket);
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
  else if (m_txBuffer->IsSocketAvailable (socket))
    {
      // socket is already closed, so only remove it from the Tx buffer
      m_txBuffer->RemoveSocket (socket);
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
  else if (m_txBuffer->IsSocketAvailable (socket))
    {
      // socket is already closed, so only remove it from the Tx buffer
      m_txBuffer->RemoveSocket (socket);
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

      if (packet->GetSize () < HttpEntityHeader::GetStaticSerializedSize ())
        {
          NS_FATAL_ERROR ("The received packet does not contain HTTP entity header");
        }

      m_rxTrace (packet, from);

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
              m_rxDelayTrace (Simulator::Now () - tag.GetTs (), from);
              isTagged = true;
            }
        }

      if (!isTagged)
        {
          NS_LOG_WARN (this << " SeqTs tag is not found");
        }

      HttpEntityHeader httpEntity;
      packet->RemoveHeader (httpEntity);

      Time delay;
      switch (httpEntity.GetContentType ())
        {
        case HttpEntityHeader::MAIN_OBJECT:
          delay = m_httpVariables->GetMainObjectGenerationDelay ();
          NS_LOG_INFO (this << " will finish generating a main object"
                            << " in " << delay.GetSeconds () << " seconds");
          m_txBuffer->RecordNextServe (socket,
                                       Simulator::Schedule (delay,
                                                            &HttpServer::ServeNewMainObject,
                                                            this, socket));
          break;

        case HttpEntityHeader::EMBEDDED_OBJECT:
          delay = m_httpVariables->GetEmbeddedObjectGenerationDelay ();
          NS_LOG_INFO (this << " will finish generating an embedded object"
                            << " in " << delay.GetSeconds () << " seconds");
          m_txBuffer->RecordNextServe (socket,
                                       Simulator::Schedule (delay,
                                                            &HttpServer::ServeNewEmbeddedObject,
                                                            this, socket));
          break;

        default:
          NS_FATAL_ERROR ("Invalid packet header");
          break;
        }

    } // end of `while ((packet = socket->RecvFrom (from)))`

} // end of `void ReceivedDataCallback (Ptr<Socket> socket)`


void
HttpServer::SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize)
{
  NS_LOG_FUNCTION (this << socket << availableBufferSize);

  if (!m_txBuffer->IsBufferEmpty (socket))
    {
      const uint32_t txBufferSize = m_txBuffer->GetBufferSize (socket);
      const uint32_t actualSent = ServeFromTxBuffer (socket);

#ifdef NS3_LOG_ENABLE
      if (actualSent < txBufferSize)
        {
          switch (m_txBuffer->GetBufferContentType (socket))
            {
            case HttpEntityHeader::MAIN_OBJECT:
              NS_LOG_INFO (this << " transmission of main object is suspended"
                                << " after " << actualSent << " bytes");
              break;
            case HttpEntityHeader::EMBEDDED_OBJECT:
              NS_LOG_INFO (this << " transmission of embedded object is suspended"
                                << " after " << actualSent << " bytes");
              break;
            default:
              NS_FATAL_ERROR ("Invalid Tx buffer content type");
              break;
            }
        }
      else
        {
          switch (m_txBuffer->GetBufferContentType (socket))
            {
            case HttpEntityHeader::MAIN_OBJECT:
              NS_LOG_INFO (this << " finished sending a whole main object");
              break;
            case HttpEntityHeader::EMBEDDED_OBJECT:
              NS_LOG_INFO (this << " finished sending a whole embedded object");
              break;
            default:
              NS_FATAL_ERROR ("Invalid Tx buffer content type");
              break;
            }
        }
#endif /* NS3_LOG_ENABLE */

      // mute compiler warning
      NS_UNUSED (txBufferSize);
      NS_UNUSED (actualSent);

    } // end of `if (m_txBuffer->IsBufferEmpty (socket))`

} // end of `void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize)`


void
HttpServer::ServeNewMainObject (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  const uint32_t objectSize = m_httpVariables->GetMainObjectSize ();
  NS_LOG_INFO (this << " main object to be served is "
                    << objectSize << " bytes");
  m_txBuffer->WriteNewObject (socket, HttpEntityHeader::MAIN_OBJECT,
                              objectSize);
  const uint32_t actualSent = ServeFromTxBuffer (socket);

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
HttpServer::ServeNewEmbeddedObject (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  const uint32_t objectSize = m_httpVariables->GetEmbeddedObjectSize ();
  NS_LOG_INFO (this << " embedded object to be served is "
                    << objectSize << " bytes");
  m_txBuffer->WriteNewObject (socket, HttpEntityHeader::EMBEDDED_OBJECT,
                              objectSize);
  const uint32_t actualSent = ServeFromTxBuffer (socket);

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
HttpServer::ServeFromTxBuffer (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if (m_txBuffer->IsBufferEmpty (socket))
    {
      return 0;
    }
  else
    {
      const bool hasTxedPartOfObject = m_txBuffer->HasTxedPartOfObject (socket);

      uint32_t headerSize; // how many bytes needed for header
      if (hasTxedPartOfObject)
        {
          // subsequent packet of the same object does not require header
          headerSize = 0;
        }
      else
        {
          // the first packet of the object always begins with a header
          headerSize = HttpEntityHeader::GetStaticSerializedSize ();
        }

      const uint32_t socketSize = socket->GetTxAvailable ();
      NS_LOG_DEBUG (this << " socket has " << socketSize
                         << " bytes available for Tx");

      if (socketSize > headerSize)
        {
          HttpEntityHeader::ContentType_t txBufferContentType
            = m_txBuffer->GetBufferContentType (socket);
          const uint32_t txBufferSize = m_txBuffer->GetBufferSize (socket);

          // size of actual content to be sent now, has to fit into the socket
          const uint32_t contentSize = std::min (txBufferSize,
                                                 socketSize - headerSize);
          //contentSize = std::min (contentSize, m_mtuSize - headerSize);
          Ptr<Packet> packet = Create<Packet> (contentSize);

          if (headerSize > 0)
            {
              NS_ASSERT (!hasTxedPartOfObject);
              HttpEntityHeader httpEntityHeader;
              httpEntityHeader.SetContentType (txBufferContentType);
              httpEntityHeader.SetContentLength (txBufferSize);
              packet->AddHeader (httpEntityHeader);

              /*
               * Packet tag would be a better option than byte tag. But we
               * tested packet tag and we observed that it apparently disappear
               * somewhere at lower layers, even in a simple P2P scenario.
               * Thereby we use byte tag.
               */
              packet->AddByteTag (HttpSeqTsTag ());
            }

          const uint32_t packetSize = packet->GetSize ();
          NS_ASSERT (packetSize == (contentSize + headerSize));
          NS_ASSERT (packetSize <= socketSize);

          NS_LOG_INFO (this << " created packet " << packet << " of "
                            << packetSize << " bytes");

          int actualBytes = socket->Send (packet);
          NS_LOG_DEBUG (this << " Send() packet " << packet
                             << " of " << packetSize << " bytes,"
                             << " return value= " << actualBytes);
          m_txTrace (packet);

          if ((unsigned) actualBytes == packetSize)
            {
              // the packet go through successfully
              m_txBuffer->DepleteBufferSize (socket, contentSize);
              NS_LOG_INFO (this << " remaining object to be sent "
                                << m_txBuffer->GetBufferSize (socket) << " bytes");
              return contentSize;
            }
          else
            {
              NS_LOG_INFO (this << " failed to send object,"
                                << " GetErrNo= " << socket->GetErrno () << ","
                                << " suspending transmission"
                                << " and waiting for another Tx opportunity");
              return 0;
            }

        } // end of `if (socketSize > headerSize)`
      else
        {
          NS_LOG_INFO (this << " not enough space for Tx in socket,"
                            << " suspending transmission"
                            << " and waiting for another Tx opportunity");
          return 0;
        }

    } // end of else of `if (!m_txBuffer->IsTxBufferEmpty (socket))`

} // end of `uint32_t ServeFromTxBuffer (Ptr<Socket> socket)`


void
HttpServer::SwitchToState (HttpServer::State_t state)
{
  const std::string oldState = GetStateString ();
  const std::string newState = GetStateString (state);
  NS_LOG_FUNCTION (this << oldState << newState);
  m_state = state;
  NS_LOG_INFO (this << " HttpServer " << oldState << " --> " << newState);
  m_stateTransitionTrace (oldState, newState);
}


// HTTP SERVER TX BUFFER //////////////////////////////////////////////////////


HttpServerTxBuffer::HttpServerTxBuffer ()
{
  NS_LOG_FUNCTION (this);
}


bool
HttpServerTxBuffer::IsSocketAvailable (Ptr<Socket> socket) const
{
  std::map<Ptr<Socket>, TxBuffer_t>::const_iterator it;
  it = m_txBuffer.find (socket);
  return (it != m_txBuffer.end ());
}


void
HttpServerTxBuffer::AddSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  NS_ASSERT_MSG (!IsSocketAvailable (socket),
                 this << " cannot add socket " << socket
                      << " because it has already added before");

  TxBuffer_t txBuffer;
  txBuffer.txBufferContentType = HttpEntityHeader::NOT_SET;
  txBuffer.txBufferSize = 0;
  txBuffer.hasTxedPartOfObject = false;
  m_txBuffer.insert (std::pair<Ptr<Socket>, TxBuffer_t> (socket, txBuffer));
}


void
HttpServerTxBuffer::RemoveSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  std::map<Ptr<Socket>, TxBuffer_t>::iterator it;
  it = m_txBuffer.find (socket);
  NS_ASSERT_MSG (it != m_txBuffer.end (),
                 "Socket " << socket << " cannot be found");

  if (!Simulator::IsExpired (it->second.nextServe))
    {
      NS_LOG_INFO (this << " canceling a serving event which is due in "
                        << Simulator::GetDelayLeft (it->second.nextServe).GetSeconds ()
                        << " seconds");
      Simulator::Cancel (it->second.nextServe);
    }

  it->first->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (),
                                MakeNullCallback<void, Ptr<Socket> > ());
  it->first->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  it->first->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());

  m_txBuffer.erase (it);
}


void
HttpServerTxBuffer::CloseSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  std::map<Ptr<Socket>, TxBuffer_t>::iterator it;
  it = m_txBuffer.find (socket);
  NS_ASSERT_MSG (it != m_txBuffer.end (),
                 "Socket " << socket << " cannot be found");

  if (!Simulator::IsExpired (it->second.nextServe))
    {
      NS_LOG_INFO (this << " canceling a serving event which is due in "
                        << Simulator::GetDelayLeft (it->second.nextServe).GetSeconds ()
                        << " seconds");
      Simulator::Cancel (it->second.nextServe);
    }

  if (it->second.txBufferSize > 0)
    {
      NS_LOG_WARN (this << " closing a socket where "
                        << it->second.txBufferSize << " bytes of transmission"
                        << " is still pending in the corresponding Tx buffer");
    }

  it->first->Close ();
  it->first->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (),
                                MakeNullCallback<void, Ptr<Socket> > ());
  it->first->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  it->first->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());

  m_txBuffer.erase (it);
}


void
HttpServerTxBuffer::CloseAllSockets ()
{
  NS_LOG_FUNCTION (this);

  std::map<Ptr<Socket>, TxBuffer_t>::iterator it;
  for (it = m_txBuffer.begin ();
       it != m_txBuffer.end (); it++)
    {
      if (!Simulator::IsExpired (it->second.nextServe))
        {
          NS_LOG_INFO (this << " canceling a serving event which is due in "
                            << Simulator::GetDelayLeft (it->second.nextServe).GetSeconds ()
                            << " seconds");
          Simulator::Cancel (it->second.nextServe);
        }

      it->first->Close ();
      it->first->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (),
                                    MakeNullCallback<void, Ptr<Socket> > ());
      it->first->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      it->first->SetSendCallback (MakeNullCallback<void, Ptr<Socket>, uint32_t > ());
    }

  m_txBuffer.clear ();
}


bool
HttpServerTxBuffer::IsBufferEmpty (Ptr<Socket> socket) const
{
  std::map<Ptr<Socket>, TxBuffer_t>::const_iterator it;
  it = m_txBuffer.find (socket);
  NS_ASSERT_MSG (it != m_txBuffer.end (),
                 "Socket " << socket << " cannot be found");
  return (it->second.txBufferSize == 0);
}


HttpEntityHeader::ContentType_t
HttpServerTxBuffer::GetBufferContentType (Ptr<Socket> socket) const
{
  std::map<Ptr<Socket>, TxBuffer_t>::const_iterator it;
  it = m_txBuffer.find (socket);
  NS_ASSERT_MSG (it != m_txBuffer.end (),
                 "Socket " << socket << " cannot be found");
  return it->second.txBufferContentType;
}


uint32_t
HttpServerTxBuffer::GetBufferSize (Ptr<Socket> socket) const
{
  std::map<Ptr<Socket>, TxBuffer_t>::const_iterator it;
  it = m_txBuffer.find (socket);
  NS_ASSERT_MSG (it != m_txBuffer.end (),
                 "Socket " << socket << " cannot be found");
  return it->second.txBufferSize;
}


bool
HttpServerTxBuffer::HasTxedPartOfObject (Ptr<Socket> socket) const
{
  std::map<Ptr<Socket>, TxBuffer_t>::const_iterator it;
  it = m_txBuffer.find (socket);
  NS_ASSERT_MSG (it != m_txBuffer.end (),
                 "Socket " << socket << " cannot be found");
  return it->second.hasTxedPartOfObject;
}


void
HttpServerTxBuffer::WriteNewObject (Ptr<Socket> socket,
                                    HttpEntityHeader::ContentType_t contentType,
                                    uint32_t objectSize)
{
  NS_LOG_FUNCTION (this << socket << contentType << objectSize);

  NS_ASSERT_MSG (contentType != HttpEntityHeader::NOT_SET,
                 "Unable to write an object without a proper Content-Type");
  NS_ASSERT_MSG (objectSize > 0,
                 "Unable to write a zero-sized object");

  std::map<Ptr<Socket>, TxBuffer_t>::iterator it;
  it = m_txBuffer.find (socket);
  NS_ASSERT_MSG (it != m_txBuffer.end (),
                 "Socket " << socket << " cannot be found");
  NS_ASSERT_MSG (it->second.txBufferSize == 0,
                 "Cannot write to Tx buffer of socket " << socket
                                                        << " until the previous content has been completely sent");
  it->second.txBufferContentType = contentType;
  it->second.txBufferSize = objectSize;
  it->second.hasTxedPartOfObject = false;
}


void
HttpServerTxBuffer::RecordNextServe (Ptr<Socket> socket, EventId eventId)
{
  NS_LOG_FUNCTION (this << socket);

  std::map<Ptr<Socket>, TxBuffer_t>::iterator it;
  it = m_txBuffer.find (socket);
  NS_ASSERT_MSG (it != m_txBuffer.end (),
                 "Socket " << socket << " cannot be found");
  it->second.nextServe = eventId;
}


void
HttpServerTxBuffer::DepleteBufferSize (Ptr<Socket> socket, uint32_t amount)
{
  NS_LOG_FUNCTION (this << socket << amount);

  NS_ASSERT_MSG (amount > 0, "Unable to consume zero bytes");

  std::map<Ptr<Socket>, TxBuffer_t>::iterator it;
  it = m_txBuffer.find (socket);
  NS_ASSERT_MSG (it != m_txBuffer.end (),
                 "Socket " << socket << " cannot be found");
  NS_ASSERT_MSG (it->second.txBufferSize >= amount,
                 "The requested amount is larger than the current buffer size");
  it->second.txBufferSize -= amount;
  it->second.hasTxedPartOfObject = true;
}


} // end of `namespace ns3`
