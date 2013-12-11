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
  // LISTENER SOCKET CALLBACK METHODS

  bool ConnectionRequestCallback (Ptr<Socket> socket, const Address & address);
  void NewConnectionCreatedCallback (Ptr<Socket> socket,
                                     const Address & address);
  void NormalCloseCallback (Ptr<Socket> socket);
  void ErrorCloseCallback (Ptr<Socket> socket);

  /**
   * \brief Invoked by NrtvServerVideoWorker instance if it has completed the
   *        transmission of a video.
   */
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
 * \brief Represent a single video session and its transmission over the network
 *        to a client.
 */
class NrtvServerVideoWorker : public SimpleRefCount<NrtvServerVideoWorker>
{
public:

  /**
   * \brief Creates a new instance of worker.
   *
   * \param server pointer to the parent server instance, which is the source
   *               of random variables used by the worker, and will be notified
   *               on every packet transmitted by the worker and upon the
   *               completion of the video transmission
   * \param socket pointer to the socket (must be already connected to a
   *               destination client) that will be utilized by the worker to
   *               transmit video packets
   *
   * The worker will determine the length of video by calling the parent's
   * server random variable. Other variables are also retrieved from this
   * variable, such as number of frames per second (frame rate) and number of
   * slices per frame.
   *
   * The first video frame starts immediately. Each frame has a fixed number of
   * slices, and each slice is preceded by a random length of encoding delay.
   * The size of each slice is also determined randomly.
   *
   * Each frame always abides to the given frame rate, i.e., the start of each
   * frame is always punctual according to the frame rate. If the transmission
   * of the slices takes longer than the length of a single frame, then the
   * remaining untransmitted slices would be discarded, without* postponing the
   * start time of the next frame.
   *
   * Each slice transmitted will trigger the `Tx` trace source in the parent
   * server.
   *
   * Finally, when all the frames of the video have been transmitted, the worker
   * will call the parent's server NrtvServer::NotifyVideoCompleted() method.
   * The parent server is expected to destroy the worker to close the socket.
   */
  NrtvServerVideoWorker (NrtvServer* server, Ptr<Socket> socket);

  /// Object destructor, will close the socket.
  virtual ~NrtvServerVideoWorker ();

private:
  void ScheduleNewFrame ();
  void NewFrame ();
  void ScheduleNewSlice ();
  void NewSlice ();
  void CancelAllPendingEvents ();

  // SOCKET CALLBACK METHODS

  /// Invoked if the client disconnects.
  void NormalCloseCallback (Ptr<Socket> socket);
  /// Invoked if the client disconnects abruptly.
  void ErrorCloseCallback (Ptr<Socket> socket);
  /// Invoked if the socket has space for transmission.
  void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize);

  // EVENTS

  EventId m_eventNewFrame;
  EventId m_eventNewSlice;

  NrtvServer*         m_server;  ///< Pointer to the parent's server instance.
  Ptr<Socket>         m_socket;  ///< Pointer to the socket for transmission.
  Ptr<NrtvVariables>  m_nrtvVariables;  ///< Pointer to parent's server random variable.

  /// Packet index, starting from 1, to be written in the packet header.
  uint32_t m_packetSeq;
  /// Length of time between consecutive frames.
  Time m_frameInterval;
  /// Number of frames, i.e., indicating the length of the video.
  uint32_t m_numOfFrames;
  /// The number of frames that has been transmitted.
  uint32_t m_numOfFramesServed;
  /// Number of slices in one frame.
  uint16_t m_numOfSlices;
  /// The number of slices that has been transmitted, resets to 0 after completing a frame.
  uint16_t m_numOfSlicesServed;

}; // end of `class NrtvServerVideoWorker`


} // end of `namespace ns3`


#endif /* NRTV_SERVER_H */
