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
 * \brief Model application which simulates the traffic of a Near Real-Time
 *        Video (NRTV) service, i.e., a video streaming service.
 *
 * In summary, the application works as follows. Upon start, the application
 * opens a socket and listens to connection requests from clients (NrtvClient).
 * Once the request is accepted (always) and a connection is established, the
 * application begins to send a video (as a stream of packets) to the client.
 * When the transmission of the whole video is completed, the application
 * disconnects the client.
 *
 * The application maintains several workers (NrtvServerVideoWorker). Each
 * worker is responsible for sending a single video for a single client.
 *
 * The packets served by the worker share a common format. Each packet begins
 * with a 24-byte NrtvHeader. After that, the actual video content begins until
 * the end of the packet, which size is randomly determined by NrtvVariables.
 */
class NrtvServer : public Application
{
public:
  /**
   * \brief Creates a new instance of NRTV server application.
   *
   * After creation, the application must be further configured through
   * attributes. To avoid having to do this process manually, please use one of
   * the helper classes (either NrtvHelper or NrtvServerHelper).
   *
   * \warning At the moment, only TCP protocol and IPv4 is supported.
   */
  NrtvServer ();

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \return the address bound to the server
   */
  Address GetLocalAddress () const;

  /**
   * \return the port the server listens to
   */
  uint16_t GetLocalPort () const;

  /// The possible states of the application.
  enum State_t
  {
    NOT_STARTED = 0,  ///< Before StartApplication() is invoked.
    STARTED,          ///< Passively waiting for connections and/or actively sending videos.
    STOPPED           ///< After StopApplication() is invoked.
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
   * \param state an arbitrary state of an application
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
  /// Keeping all the active workers.
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
   * \brief Creates a new instance of worker and starts the transmission.
   *
   * \param server pointer to the parent server instance, which is the source
   *               of random variables used by the worker, and will be notified
   *               on every packet sent by the worker and upon the completion of
   *               the video transmission
   * \param socket pointer to the socket (must be already connected to a
   *               destination client) that will be utilized by the worker to
   *               send video packets
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
   * remaining unsent slices would be discarded, without postponing the start
   * time of the next frame.
   *
   * Each slice sent will trigger the `Tx` trace source in the parent server.
   *
   * Finally, when all the frames of the video have been sent, the worker will
   * call the parent's server NrtvServer::NotifyVideoCompleted() method. The
   * parent server is expected to destroy the worker to close the socket.
   */
  NrtvServerVideoWorker (NrtvServer* server, Ptr<Socket> socket);

  /// Instance destructor, will close the socket.
  virtual ~NrtvServerVideoWorker ();

private:
  // SOCKET CALLBACK METHODS

  /// Invoked if the client disconnects.
  void NormalCloseCallback (Ptr<Socket> socket);
  /// Invoked if the client disconnects abruptly.
  void ErrorCloseCallback (Ptr<Socket> socket);
  /// Invoked if the socket has space for transmission.
  void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize);

  void ScheduleNewFrame ();
  void NewFrame ();
  void ScheduleNewSlice ();
  void NewSlice ();
  void CancelAllPendingEvents ();

  // EVENTS

  EventId m_eventNewFrame;
  EventId m_eventNewSlice;

  NrtvServer*         m_server;  ///< Pointer to the parent's server instance.
  Ptr<Socket>         m_socket;  ///< Pointer to the socket for transmission.
  Ptr<NrtvVariables>  m_nrtvVariables;  ///< Pointer to parent's server random variable.

  /// Length of time between consecutive frames.
  Time m_frameInterval;
  /// Number of frames, i.e., indicating the length of the video.
  uint32_t m_numOfFrames;
  /// The number of frames that has been sent.
  uint32_t m_numOfFramesServed;
  /// Number of slices in one frame.
  uint16_t m_numOfSlices;
  /// The number of slices that has been sent, resets to 0 after completing a frame.
  uint16_t m_numOfSlicesServed;

}; // end of `class NrtvServerVideoWorker`


} // end of `namespace ns3`


#endif /* NRTV_SERVER_H */
