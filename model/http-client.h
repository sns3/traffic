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

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <ns3/application.h>
#include <ns3/address.h>
#include <ns3/traced-callback.h>
#include <ns3/http-entity-header.h>


namespace ns3 {


class Socket;
class Packet;
class HttpVariables;


/**
 * \ingroup traffic
 * \brief Model application which simulates the traffic of a client of a
 *        Hypertext Transfer Protocol (HTTP) service, i.e., a web browser.
 *
 * In summary, the application works as follows.
 * 1. Upon start, it opens a connection to the destination web server
 *    (HttpServer).
 * 2. After the connection is established, the application immediately requests
 *    a *main object* from the server by sending a request packet.
 * 3. After receiving a main object (which can take some time if it consists of
 *    several packets), the application "parses" the main object.
 * 4. The parsing takes a short time (randomly determined) to determine the
 *    number of *embedded objects* (also randomly determined) in the web page.
 *    - If at least one embedded object is determined, the application requests
 *      the first embedded object from the server. The request for the next
 *      embedded object follows after the previous embedded object has been
 *      completely received.
 *    - If there is no embedded object to request, the application enters the
 *      *reading time*.
 * 5. Reading time is a long delay (again, randomly determined) where the
 *    application does not induce any network traffic, thus simulating the user
 *    reading the downloaded web page.
 * 6. After the reading time is finished, the process repeats to step #2.
 *
 * Each request by default has a constant size of 350 bytes, plus the additional
 * 6 bytes of HttpEntityHeader in the beginning. The Content-Type field of the
 * header is set with the type of object requested, which is either main or
 * embedded object. The Content-Length field is set to zero. In addition to this
 * header, the packet also contains an HttpSeqTsTag byte tag, which will may be
 * used by the recipient to compute the delay of the packet.
 *
 * The application expects to receive packets in the following format. The first
 * packet of each object (either main or embedded) must contain the
 * HttpEntityHeader header and the HttpSeqTsTag byte tag. The value of the
 * Content-Type field of the header must match with the type of object that the
 * applications is expecting. Then the Content-Length field shall contain the
 * *object size* in bytes. If the received packet is smaller than the object
 * size, then the application will expect more packets to come. The combined
 * size of the packets (minus the header on the first packet) must be equal to
 * the object size. The information in the tag is used for computing the delay
 * of the object.
 */
class HttpClient : public Application
{
public:

  /**
   * \brief Creates a new instance of HTTP client application.
   *
   * After creation, the application must be further configured through
   * attributes. To avoid having to do this process manually, please use one of
   * the helper classes (either HttpHelper or HttpClientHelper).
   *
   * \warning At the moment, only TCP protocol and IPv4 is supported.
   *
   * Upon creation, the application randomly determines its working mode. The
   * persistent mode (HTTP 1.1) keeps the connection alive during the whole
   * application lifetime. While the burst mode (HTTP 1.0) closes the connection
   * upon receiving a whole object (main or embedded alike) and then opens
   * another connection when the application needs to request another object.
   */
  HttpClient ();

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
    /// Before StartApplication() is invoked.
    NOT_STARTED = 0,
    /// Sent the server a connection request and waiting for the server to be accept it.
    CONNECTING,
    /// Sent the server a request for a main object and waiting to receive the packets.
    EXPECTING_MAIN_OBJECT,
    /// Parsing a main object that has just been received.
    PARSING_MAIN_OBJECT,
    /// Sent the server a request for an embedded object and waiting to receive the packets.
    EXPECTING_EMBEDDED_OBJECT,
    /// User reading a web page that has just been received.
    READING,
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
  // SOCKET CALLBACK METHODS
  void ConnectionSucceededCallback (Ptr<Socket> socket);
  void ConnectionFailedCallback (Ptr<Socket> socket);
  void NormalCloseCallback (Ptr<Socket> socket);
  void ErrorCloseCallback (Ptr<Socket> socket);
  void ReceivedDataCallback (Ptr<Socket> socket);
  void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize);

  void OpenConnection ();
  void RetryConnection ();
  void CloseConnection ();

  void RequestMainObject ();
  void RequestEmbeddedObject ();
  void ReceiveMainObject (Ptr<Packet> packet, const Address &from);
  void ReceiveEmbeddedObject (Ptr<Packet> packet, const Address &from);
  uint32_t Receive (Ptr<Packet> packet,
                    HttpEntityHeader::ContentType_t expectedContentType);
  void EnterParsingTime ();
  void ParseMainObject ();
  void EnterReadingTime ();

  void CancelAllPendingEvents ();
  void SwitchToState (State_t state);

  State_t      m_state;
  bool         m_isBurstMode;
  Ptr<Socket>  m_socket;
  uint32_t     m_objectBytesToBeReceived;
  Time         m_objectArrivalTime;
  uint32_t     m_embeddedObjectsToBeRequested;

  // ATTRIBUTES

  Ptr<HttpVariables>  m_httpVariables;
  Address             m_remoteServerAddress;
  uint16_t            m_remoteServerPort;
  TypeId              m_protocol;

  // TRACE SOURCES

  TracedCallback<Ptr<const Packet> >        m_txMainObjectRequestTrace;
  TracedCallback<Ptr<const Packet> >        m_txEmbeddedObjectRequestTrace;
  TracedCallback<Ptr<const Packet> >        m_rxMainObjectPacketTrace;
  TracedCallback<>                          m_rxMainObjectTrace;
  TracedCallback<Ptr<const Packet> >        m_rxEmbeddedObjectPacketTrace;
  TracedCallback<>                          m_rxEmbeddedObjectTrace;
  TracedCallback<std::string, std::string>  m_stateTransitionTrace;

  /*
   * Example signature of callback function (with context):
   *
   *     void RxCallback (std::string context, Ptr<const Packet> packet,
   *                      const Address & from);
   */
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

  /*
   * Example signature of callback function (with context):
   *
   *     void RxDelayCallback (std::string context, Time delay,
   *                           const Address & from);
   */
  TracedCallback<Time, const Address &> m_rxDelayTrace;

  // EVENTS

  EventId m_eventRequestMainObject;
  EventId m_eventRequestEmbeddedObject;
  EventId m_eventRetryConnection;
  EventId m_eventParseMainObject;

}; // end of `class HttpClient`


}  // end of `namespace ns3`


#endif /* HTTP_CLIENT_H */
