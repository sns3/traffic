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
 * \brief Model application which simulates the traffic of a client of a Near
 *        Real Time Video (NRTV) service, i.e., a client accessing a video
 *        streaming service.
 *
 * Upon start, the application sends a connection request to the destination
 * server. Once connected, the application waits for incoming video packets.
 *
 * When the server terminates the connection, the application regards it as the
 * end of a video session. At this point, the application enters the IDLE state,
 * which is a randomly determined delay that simulates the user "resting"
 * between videos (e.g., commenting or picking the next video). After the IDLE
 * timer expires, the application restarts again by sending another connection
 * request.
 */
class NrtvClient : public Application
{
public:

  /**
   * \brief Creates a new instance of NRTV client application.
   *
   * After creation, the application must be further configured through
   * attributes. To avoid having to do this process manually, please use one of
   * the helper classes (either NrtvHelper or NrtvClientHelper).
   *
   * \warning At the moment, only TCP protocol and IPv4 is supported.
   */
  NrtvClient ();

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \return the time the application is scheduled to start
   */
  Time GetStartTime () const;

  /**
   * \return the time the application is scheduled to stop, or 0 if the stop has
   *         never been scheduled
   */
  Time GetStopTime () const;

  /**
   * \return true if the application has been scheduled to stop during the
   *         simulation
   */
  bool IsScheduledToStop () const;

  /**
   * \return the address of the destination server
   */
  Address GetRemoteServerAddress () const;

  /**
   * \return the destination port
   */
  uint16_t GetRemoteServerPort () const;

  /// The possible states of the application.
  enum State_t
  {
    /// Before the StartApplication() is executed.
    NOT_STARTED = 0,
    /// Sent the server a connection request and waiting for the server to be accept it.
    CONNECTING,
    /// Receiving incoming video packets.
    RECEIVING,
    /// Finished received a video and transitioning to the next video.
    IDLE,
    /// After StopApplication() is invoked.
    STOPPED
  };

  /**
   * \return the current state of the application
   */
  State_t GetState () const;

  /**
   * \return the current state of the application in string format
   */
  std::string GetStateString () const;

  /**
   * \param an arbitrary state of an application
   * \return the state equivalently expressed in string format
   */
  static std::string GetStateString (State_t state);

protected:
  // Inherited from Object base class
  virtual void DoDispose ();

  // Inherited from Application base class
  virtual void StartApplication ();
  virtual void StopApplication ();

private:
  // SOCKET CALLBACK METHODS
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
