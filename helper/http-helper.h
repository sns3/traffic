/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Original work author (from packet-sink-helper.cc):
 * - Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *
 * Converted to HTTP traffic models by:
 * - Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#ifndef HTTP_HELPER_H
#define HTTP_HELPER_H

#include <ns3/object-factory.h>
#include <ns3/ipv4-address.h>
#include <ns3/node-container.h>
#include <ns3/application-container.h>


namespace ns3 {


/**
 * \ingroup traffic
 * \brief Helper to make it easier to instantiate an HttpClient on a set of
 *        nodes.
 */
class HttpClientHelper
{
public:
  /**
   * \brief Create a HttpClientHelper to make it easier to work with
   *        HttpClient applications.
   *
   * \param protocol the name of the protocol to be used to send and receive
   *                 traffic
   * \param address the address of the remote server node to send traffic to
   *
   * The protocol argument is a string identifying the socket factory type used
   * to create sockets for the applications. A typical value would be
   * "ns3::TcpSocketFactory".
   */
  HttpClientHelper (std::string protocol, Address address);

  /**
   * \brief Helper function used to set the underlying application attributes,
   *        but *not* the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Install an HttpClient on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param c NodeContainer of the set of nodes on which an HttpClient will be
   *          installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * \brief Install an HttpClient on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param node The node on which an HttpClient will be installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * \brief Install an HttpClient on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param nodeName The name of the node on which an HttpClient will be
   *                 installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * \internal
   * \brief Install an HttpClient on the node configured with all the
   *        attributes set with SetAttribute().
   *
   * \param node The node on which an HttpClient will be installed
   * \return Ptr to the application installed
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;

}; // end of `class HttpClientHelper`



/**
 * \ingroup traffic
 * \brief Helper to make it easier to instantiate an HttpServer on a set of
 *        nodes.
 */
class HttpServerHelper
{
public:
  /**
   * \brief Create a HttpServerHelper to make it easier to work with
   *        HttpServer applications.
   *
   * \param protocol the name of the protocol to be used to send and receive
   *                 traffic
   * \param address the address of the server
   *
   * The protocol argument is a string identifying the socket factory type used
   * to create sockets for the applications. A typical value would be
   * "ns3::TcpSocketFactory".
   */
  HttpServerHelper (std::string protocol, Address address);

  /**
   * \brief Helper function used to set the underlying application attributes,
   *        but *not* the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Install an HttpServer on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param c NodeContainer of the set of nodes on which an HttpServer will be
   *          installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * \brief Install an HttpServer on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param node The node on which an HttpServer will be installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * \brief Install an HttpServer on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param nodeName The name of the node on which an HttpServer will be
   *                 installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * \internal
   * \brief Install an HttpServer on the node configured with all the
   *        attributes set with SetAttribute().
   *
   * \param node The node on which an HttpServer will be installed
   * \return Ptr to the application installed
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;

}; // end of `class HttpServerHelper`


/**
 * \ingroup traffic
 * \brief Helper to make it easier to instantiate an HTTP server and a group of
 *        connected HTTP clients.
 */
class HttpHelper
{
public:
  /**
   * \brief Create a HttpServerHelper to make it easier to work with
   *        HttpClient and HttpServer applications.
   *
   * \param protocol the name of the protocol to be used in the simulated
   *                 traffic
   *
   * The protocol argument is a string identifying the socket factory type used
   * to create sockets for the applications. A typical value would be
   * "ns3::TcpSocketFactory".
   */
  HttpHelper (std::string protocol);

  /// Instance destructor.
  virtual ~HttpHelper ();

  /**
   * \brief Helper function used to set the underlying HttpClient application
   *        attributes, but *not* the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   *
   * \warning This method does not modify the attribute RemoteServerAddress.
   */
  void SetClientAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Helper function used to set the underlying HttpClient application
   *        attributes, but *not* the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   *
   * \warning This method does not modify the attribute LocalAddress.
   */
  void SetServerAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Install an HttpServer application and several HttpClient
   *        applications, in which each client is connected using IPv4 to the
   *        server.
   *
   * \param serverNode the node on which an HttpServer will be installed
   * \param clientNodes the set of nodes on which HttpClient applications will
   *                    be installed
   * \return container of Ptr to the server and client applications installed
   *
   * \warning The given nodes must have Internet stack installed properly
   *          before this method can be called.
   *
   * The installed applications will be configured with all the attributes set
   * with SetClientAttribute() and SetServerAttribute(). Pointers to these
   * applications can be retrieved afterwards by calling GetClients() and
   * GetServer() methods separately.
   */
  ApplicationContainer InstallUsingIpv4 (Ptr<Node> serverNode,
                                         NodeContainer clientNodes);

  /**
   * \brief Install a pair of interconnected HttpServer application and
   *        HttpClient application using IPv4.
   *
   * \param serverNode the node on which an HttpServer will be installed
   * \param clientNode the node on which an HttpClient will be installed
   * \return container of Ptr to the server and client applications installed
   *
   * \warning The given nodes must have Internet stack installed properly
   *          before this method can be called.
   *
   * The installed applications will be configured with all the attributes set
   * with SetClientAttribute() and SetServerAttribute(). Pointers to these
   * applications can be retrieved afterwards by calling GetClients() and
   * GetServer() methods separately.
   */
  ApplicationContainer InstallUsingIpv4 (Ptr<Node> serverNode,
                                         Ptr<Node> clientNode);

  /**
   * \brief Retrieve pointers to the HTTP clients which were installed by the
   *        previous call of InstallUsingIpv4().
   *
   * \return an application container containing HTTP clients, or an empty
   *         container if InstallUsingIpv4() has never been called before
   */
  ApplicationContainer GetClients () const;

  /**
   * \brief Retrieve a pointer to the HTTP server which was installed by the
   *        previous call of InstallUsingIpv4().
   *
   * \return an application container containing a single HTTP server, or an
   *         empty container if InstallUsingIpv4() has never been called before
   */
  ApplicationContainer GetServer () const;

private:
  HttpServerHelper* m_serverHelper;
  HttpClientHelper* m_clientHelper;
  ApplicationContainer m_lastInstalledClients;
  ApplicationContainer m_lastInstalledServer;

}; // end of `class HttpHelper`


} // end of `namespace ns3`


#endif /* HTTP_HELPER_H */
