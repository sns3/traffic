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

#ifndef NRTV_SERVER_H
#define NRTV_SERVER_H

#include <ns3/application.h>
#include <ns3/address.h>
#include <ns3/nstime.h>
#include <ns3/traced-callback.h>
#include <map>


namespace ns3 {


class Socket;
class NrtvVariables;
class NrtvServerVideoWorker;


/**
 * \ingroup traffic
 * \brief
 */
class NrtvServer : public Application
{
public:
  NrtvServer ();
  virtual ~NrtvServer ();

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();

  Address GetLocalAddress () const;
  uint16_t GetLocalPort () const;

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
  // SOCKET CALLBACKS

  bool ConnectionRequestCallback (Ptr<Socket> socket, const Address & address);
  void NewConnectionCreatedCallback (Ptr<Socket> socket,
                                     const Address & address);
  void NormalCloseCallback (Ptr<Socket> socket);
  void ErrorCloseCallback (Ptr<Socket> socket);

  // VIDEO WORKER CALLBACK

  void NotifyVideoCompleted (Ptr<Socket> socket);

  void SwitchToState (State_t state);

  State_t       m_state;
  Ptr<Socket>   m_initialSocket;

  friend class NrtvServerVideoWorker;
  std::map<Ptr<Socket>, Ptr<NrtvServerVideoWorker> > m_workers;

  // ATTRIBUTES

  Ptr<NrtvVariables>  m_nrtvVariables;
  Address             m_localAddress;
  uint16_t            m_localPort;
  TypeId              m_protocol;

  // TRACE SOURCES

  TracedCallback<Ptr<const Packet> >        m_txTrace;
  TracedCallback<std::string, std::string>  m_stateTransitionTrace;

}; // end of `class NrtvServer`


/**
 * \internal
 * \ingroup traffic
 * \brief
 */
class NrtvServerVideoWorker : public SimpleRefCount<NrtvServerVideoWorker>
{
public:
  NrtvServerVideoWorker (NrtvServer* server, Ptr<Socket> socket);
  virtual ~NrtvServerVideoWorker ();

  void NormalCloseCallback (Ptr<Socket> socket);
  void ErrorCloseCallback (Ptr<Socket> socket);
  void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize);

private:
  void ScheduleNewFrame ();
  void NewFrame ();
  void ScheduleNewSlice ();
  void NewSlice ();

  // EVENTS

  EventId m_eventNewFrame;
  EventId m_eventNewSlice;

  NrtvServer*         m_server;
  Ptr<Socket>         m_socket;
  uint32_t            m_packetSeq;
  Time                m_frameInterval;
  uint32_t            m_numOfFrames;
  uint32_t            m_numOfFramesServed;
  uint16_t            m_numOfSlices;
  uint16_t            m_numOfSlicesServed;
  Ptr<NrtvVariables>  m_nrtvVariables;

}; // end of `class NrtvServerVideoWorker`


} // end of `namespace ns3`


#endif /* NRTV_SERVER_H */
