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

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <ns3/simple-ref-count.h>
#include <ns3/nstime.h>
#include <ns3/http-entity-header.h>
#include <ns3/application.h>
#include <ns3/address.h>
#include <ns3/traced-callback.h>
#include <map>


namespace ns3 {


class Socket;
class Packet;
class HttpVariables;
class HttpServerTxBuffer;


/**
 * \ingroup traffic
 * \brief
 */
class HttpServer : public Application
{
public:
  HttpServer ();
  virtual ~HttpServer ();
  static TypeId GetTypeId ();

  uint32_t GetMtuSize () const;
  Address GetAddress () const;
  uint16_t GetPort () const;

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
  bool ConnectionRequestCallback (Ptr<Socket> socket, const Address & address);
  void NewConnectionCreatedCallback (Ptr<Socket> socket,
                                     const Address & address);
  void NormalCloseCallback (Ptr<Socket> socket);
  void ErrorCloseCallback (Ptr<Socket> socket);
  void ReceivedDataCallback (Ptr<Socket> socket);
  void SendCallback (Ptr<Socket> socket, uint32_t availableBufferSize);

  void ServeNewMainObject (Ptr<Socket> socket);
  void ServeNewEmbeddedObject (Ptr<Socket> socket);
  uint32_t ServeFromTxBuffer (Ptr<Socket> socket);
  void SwitchToState (State_t state);

  State_t                  m_state;
  uint32_t                 m_mtuSize;
  Ptr<Socket>              m_initialSocket;
  Ptr<HttpServerTxBuffer>  m_txBuffer;

  // ATTRIBUTES

  Ptr<HttpVariables>  m_httpVariables;
  Address             m_localAddress;
  uint16_t            m_localPort;
  TypeId              m_protocol;

  // TRACE SOURCES

  TracedCallback<Ptr<const Packet> >                   m_txTrace;
  TracedCallback<Ptr<const Packet>, const Address &>   m_rxTrace;
  TracedCallback<std::string, std::string>             m_stateTransitionTrace;

}; // end of `class HttpServer`


/**
 * \internal
 * \ingroup traffic
 * \brief Transmission buffer for use in an HTTP server, which also handles the
 *        sockets to the connected HTTP clients.
 *
 * Each socket is allocated its own separate transmission buffer. The buffer
 * indicates the length (in bytes) and the type of the data in the buffer.
 *
 * Types of data are expressed using the HttpEntityHeader::ContentType_t type.
 * Only one type of data can be active at a time, i.e., the buffer cannot store
 * mixed types of data.
 */
class HttpServerTxBuffer : public SimpleRefCount<HttpServerTxBuffer>
{
public:
  /// Create a new instance of transmission buffer.
  HttpServerTxBuffer ();

  // SOCKET MANAGEMENT

  /**
   * \param socket pointer to the socket to be found
   * \return true if the given socket is found within the buffer
   *
   * This method is typically used before calling other methods. For example,
   * AddSocket() requires that the given socket does not exist among the stored
   * buffers. On the other hand, all the other methods that accept a pointer to
   * a socket as an argument require the existence of a buffer allocated to the
   * given socket.
   */
  bool IsSocketAvailable (Ptr<Socket> socket) const;

  /**
   * \brief Add a new socket and create an empty transmission buffer for it.
   * \param socket pointer to the new socket to be added (must not exist in the
   *               buffer)
   *
   * \warning Must be called only when IsSocketAvailable() for the given socket
   *          is false.
   *
   * After the method is completed, IsSocketAvailable() for the same pointer of
   * socket shall return true.
   */
  void AddSocket (Ptr<Socket> socket);

  /**
   * \brief Remove a socket and its associated transmission buffer, and then
   *        unset the socket's callbacks to prevent further interaction with
   *        the socket.
   * \param socket pointer to the socket to be removed
   *
   * \warning Must be called only when IsSocketAvailable() for the given socket
   *          is true.
   *
   * If the socket has a pending transmission event, it will be canceled.
   *
   * This method is useful for discarding a socket which is already closed,
   * e.g., by the HTTP client. This is due to the fact that double closing of a
   * socket may introduce undefined behaviour.
   *
   * After the method is completed, IsSocketAvailable() for the same pointer of
   * socket shall return false.
   */
  void RemoveSocket (Ptr<Socket> socket);

  /**
   * \brief Close and remove a socket and its associated transmission buffer,
   *        and then unset the socket's callback to prevent further interaction
   *        with the socket.
   * \param socket pointer to the socket to be closed and removed
   *
   * \warning Must be called only when IsSocketAvailable() for the given socket
   *          is true.
   *
   * This method is similar with RemoveSocket(), except that the latter does not
   * close the socket.
   *
   * After the method is completed, IsSocketAvailable() for the same pointer of
   * socket shall return false.
   */
  void CloseSocket (Ptr<Socket> socket);

  /**
   * \brief Close and remove all stored sockets, hence clearing the buffer.
   */
  void CloseAllSockets ();

  // BUFFER MANAGEMENT

  /**
   * \param socket pointer to the socket which is associated with the
   *               transmission buffer of interest
   * \return true if the current length of the transmission buffer is zero,
   *         i.e., no pending packet
   *
   * \warning Must be called only when IsSocketAvailable() for the given socket
   *          is true.
   */
  bool IsBufferEmpty (Ptr<Socket> socket) const;

  /**
   * \param socket pointer to the socket which is associated with the
   *               transmission buffer of interest
   * \return the Content-Type of the current data inside the transmission
   *         buffer
   *
   * \warning Must be called only when IsSocketAvailable() for the given socket
   *          is true.
   *
   * Returns HttpEntityHeader::NOT_SET when the buffer is new and never been
   * filled with any data before. Otherwise, returns either
   * HttpEntityHeader::MAIN_OBJECT or HttpEntityHeader::EMBEDDED_OBJECT.
   */
  HttpEntityHeader::ContentType_t GetBufferContentType (Ptr<Socket> socket) const;

  /**
   * \param socket pointer to the socket which is associated with the
   *               transmission buffer of interest
   * \return the length (in bytes) of the current data inside the transmission
   *         buffer
   *
   * \warning Must be called only when IsSocketAvailable() for the given socket
   *          is true.
   */
  uint32_t GetBufferSize (Ptr<Socket> socket) const;

  /**
   * \param socket pointer to the socket which is associated with the
   *               transmission buffer of interest
   * \return true if the buffer content has been read since it is written
   *
   * \warning Must be called only when IsSocketAvailable() for the given socket
   *          is true.
   *
   * This method returns true after WriteNewObject() method is called. It
   * becomes false after DepleteBufferSize() method is called.
   */
  bool HasTxedPartOfObject (Ptr<Socket> socket) const;

  /**
   * \brief Write a data representing a new main object or embedded object to
   *        the transmission buffer.
   * \param socket pointer to the socket which is associated with the
   *               transmission buffer of interest
   * \param contentType the Content-Type of the data to be written (must not
   *                    equal to HttpEntityHeader::NOT_SET)
   * \param objectSize the length (in bytes) of the new object to be created
   *                   (must be greater than zero)
   *
   * \warning Must be called only when both IsSocketAvailable() and
   *          IsBufferEmpty() for the given socket are true.
   *
   * Since this method writes a fresh new object, this method makes the
   * HasTxedPartOfObject() method returns false. The stored data can be later
   * consumed wholly of partially by DepleteBufferSize() method.
   */
  void WriteNewObject (Ptr<Socket> socket,
                       HttpEntityHeader::ContentType_t contentType,
                       uint32_t objectSize);

  /**
   * \brief Inform about a pending transmission event associated with the
   *        socket, so that it would be automatically canceled in case the
   *        socket is closed.
   * \param socket pointer to the socket which is associated with the
   *               transmission buffer of interest
   * \param eventId the event to be recorded, e.g., the return value of
   *                Simulator::Schedule function
   *
   * \warning Must be called only when IsSocketAvailable() for the given socket
   *          is true.
   */
  void RecordNextServe (Ptr<Socket> socket, EventId eventId);

  /**
   * \brief Simulate a consumption of an amount of data from the transmission
   *        buffer, e.g., for the purpose of actual transmission by the caller.
   * \param socket pointer to the socket which is associated with the
   *               transmission buffer of interest
   * \param amount the length (in bytes) to be consumed (must be greater than
   *               zero)
   *
   * \warning Must be called only when IsSocketAvailable() for the given socket
   *          is true. In addition, the requested amount must be larger than the
   *          current buffer size, which can be checked by calling the
   *          GetBufferSize() method.
   *
   * The Content-Type of the object to be consumed can be inquired beforehand
   * by the GetBufferContentType() method.
   *
   * After the method is completed, HasTxedPartOfObject() for the same
   * transmission buffer shall return true. If the method has consumed all the
   * remaining bytes within the buffer, IsBufferEmpty() for the buffer shall
   * return true.
   */
  void DepleteBufferSize (Ptr<Socket> socket, uint32_t amount);

private:
  /**
   * \brief Set of fields representing a single transmission buffer, which will
   *        be associated with a socket.
   */
  struct TxBuffer_t
  {
    /**
     * \brief Pending transmission event which will be automatically canceled
     *        when the associated socket is closed.
     */
    EventId nextServe;
    /**
     * \brief The Content-Type of the current data inside the transmission
     *        buffer. Accessible using the GetBufferContentType() method.
     */
    HttpEntityHeader::ContentType_t txBufferContentType;
    /**
     * \brief The length (in bytes) of the current data inside the transmission
     *        buffer. Accessible using the GetBufferSize() method.
     */
    uint32_t txBufferSize;
    /**
     * \brief True if the buffer content has been read since it is written.
     *        Accessible using the HasTxedPartOfObject() method.
     */
    bool hasTxedPartOfObject;
  };

  /// Collection of accepted sockets and its individual transmission buffer.
  std::map<Ptr<Socket>, TxBuffer_t> m_txBuffer;

}; // end of `class HttpServerTxBuffer`


}  // end of `namespace ns3`


#endif /* HTTP_SERVER_H */
