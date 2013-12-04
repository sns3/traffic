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

#ifndef NRTV_CLIENT_H
#define NRTV_CLIENT_H

#include <ns3/application.h>
#include <ns3/address.h>
#include <ns3/nstime.h>
#include <ns3/traced-callback.h>


namespace ns3 {


class Socket;
class Packet;
class NrtvVariables;


/**
 * \ingroup traffic
 * \brief
 */
class NrtvClient : public Application
{
public:
  NrtvClient ();
  virtual ~NrtvClient ();
  static TypeId GetTypeId ();

  Address GetRemoteServerAddress () const;
  uint16_t GetRemoteServerPort () const;

  uint32_t GetNumOfRxSlices () const;
  Time GetDelaySum () const;
  /**
   * \brief Retrieve the average packet delay experienced by this client
   *        instance (downlink).
   * \return the average delay of all received packets from the start of the
   *         application until now
   *
   * Equivalent as manually computing the average by dividing GetDelaySum()
   * with GetNumOfRxSlices().
   */
  Time GetDelayAverage () const;

  enum State_t
  {
    NOT_STARTED = 0,
    CONNECTING,
    RECEIVING,
    IDLE,
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
  void ConnectionSucceededCallback (Ptr<Socket> socket);
  void ConnectionFailedCallback (Ptr<Socket> socket);
  void NormalCloseCallback (Ptr<Socket> socket);
  void ErrorCloseCallback (Ptr<Socket> socket);
  void ReceivedDataCallback (Ptr<Socket> socket);

  void OpenConnection ();
  void RetryConnection ();
  void CloseConnection ();

  uint32_t Receive (Ptr<Packet> packet);

  void CancelAllPendingEvents ();
  void SwitchToState (State_t state);

  State_t      m_state;
  uint32_t     m_numOfRxSlices;
  Time         m_delaySum;
  Time         m_dejitterBufferWindowSize;
  Ptr<Socket>  m_socket;

  // ATTRIBUTES

  Ptr<NrtvVariables>  m_nrtvVariables;
  Address             m_remoteServerAddress;
  uint16_t            m_remoteServerPort;
  TypeId              m_protocol;

  // TRACE SOURCES

  TracedCallback<Ptr<const Packet> >        m_rxTrace;
  TracedCallback<uint16_t, uint16_t>        m_rxSliceTrace;
  TracedCallback<uint32_t, uint32_t>        m_rxFrameTrace;
  TracedCallback<std::string, std::string>  m_stateTransitionTrace;

  // EVENTS

  EventId m_eventRetryConnection;

}; // end of `class NrtvClient`


}  // end of `namespace ns3`


#endif /* NRTV_CLIENT_H */
