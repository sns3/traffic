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
 * Converted to NRTV traffic models by:
 * - Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#ifndef NRTV_HELPER_H
#define NRTV_HELPER_H

#include <ns3/object-factory.h>
#include <ns3/ipv4-address.h>
#include <ns3/node-container.h>
#include <ns3/application-container.h>


namespace ns3 {


/**
 * \ingroup traffic
 * \brief Helper to make it easier to instantiate an NrtvClient on a set of
 *        nodes.
 */
class NrtvClientHelper
{
public:
  /**
   * \brief Create a NrtvClientHelper to make it easier to work with
   *        NrtvClient applications.
   *
   * \param protocol the name of the protocol to be used to send and receive
   *                 traffic
   * \param address the address of the remote server node to send traffic to
   *
   * The protocol argument is a string identifying the socket factory type used
   * to create sockets for the applications. A typical value would be
   * "ns3::TcpSocketFactory" or "ns3::UdpSocketFactory".
   */
  NrtvClientHelper (std::string protocol, Address address);

  /**
   * \brief Helper function used to set the underlying application attributes,
   *        but _not_ the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Install an NrtvClient on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param c NodeContainer of the set of nodes on which an NrtvClient will be
   *          installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * \brief Install an NrtvClient on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param node The node on which an NrtvClient will be installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * \brief Install an NrtvClient on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param nodeName The name of the node on which an NrtvClient will be
   *                 installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * \internal
   * \brief Install an NrtvClient on the node configured with all the
   *        attributes set with SetAttribute().
   *
   * \param node The node on which an NrtvClient will be installed
   * \return Ptr to the application installed
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;

}; // end of `class NrtvClientHelper`



/**
 * \ingroup traffic
 * \brief Helper to make it easier to instantiate an NrtvServer on a set of
 *        nodes.
 */
class NrtvServerHelper
{
public:
  /**
   * \brief Create a NrtvServerHelper to make it easier to work with
   *        NrtvServer applications.
   *
   * \param protocol the name of the protocol to be used to send and receive
   *                 traffic
   * \param address the address of the server
   *
   * The protocol argument is a string identifying the socket factory type used
   * to create sockets for the applications. A typical value would be
   * "ns3::TcpSocketFactory" or "ns3::UdpSocketFactory".
   */
  NrtvServerHelper (std::string protocol, Address address);

  /**
   * \brief Helper function used to set the underlying application attributes,
   *        but _not_ the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Install an NrtvServer on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param c NodeContainer of the set of nodes on which an NrtvServer will be
   *          installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * \brief Install an NrtvServer on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param node The node on which an NrtvServer will be installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * \brief Install an NrtvServer on each node of the input container
   *        configured with all the attributes set with SetAttribute().
   *
   * \param nodeName The name of the node on which an NrtvServer will be
   *                 installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * \internal
   * \brief Install an NrtvServer on the node configured with all the
   *        attributes set with SetAttribute().
   *
   * \param node The node on which an NrtvServer will be installed
   * \return Ptr to the application installed
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;

}; // end of `class NrtvServerHelper`


/**
 * \ingroup traffic
 * \brief Helper to make it easier to instantiate an NRTV server and a group of
 *        connected NRTV clients.
 */
class NrtvHelper
{
public:
  /**
   * \brief Create a NrtvServerHelper to make it easier to work with
   *        NrtvClient and NrtvServer applications.
   *
   * \param protocol the name of the protocol to be used in the simulated
   *                 traffic
   *
   * The protocol argument is a string identifying the socket factory type used
   * to create sockets for the applications. A typical value would be
   *  or "ns3::UdpSocketFactory".
   */
  NrtvHelper (std::string protocol);

  /**
   * \internal
   * \brief Class destructor.
   */
  virtual ~NrtvHelper ();

  /**
   * \brief Helper function used to set the underlying NrtvClient application
   *        attributes, but _not_ the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   *
   * \warning This method does not modify the attribute RemoteServerAddress.
   */
  void SetClientAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Helper function used to set the underlying NrtvClient application
   *        attributes, but _not_ the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   *
   * \warning This method does not modify the attribute LocalAddress.
   */
  void SetServerAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Install an NrtvServer application and several NrtvClient
   *        applications, in which each client is connected using IPv4 to the
   *        server.
   *
   * \param serverNode the node on which an NrtvServer will be installed
   * \param clientNodes the set of nodes on which NrtvClient applications will
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
   * \brief Install a pair of interconnected NrtvServer application and
   *        NrtvClient application using IPv4.
   *
   * \param serverNode the node on which an NrtvServer will be installed
   * \param clientNode the node on which an NrtvClient will be installed
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
   * \brief Retrieve pointers to the NRTV clients which were installed by the
   *        previous call of Install().
   *
   * \return an application container containing NRTV clients, or an empty
   *         container if Install() has never been called before
   */
  ApplicationContainer GetClients () const;

  /**
   * \brief Retrieve a pointer to the NRTV server which was installed by the
   *        previous call of Install().
   *
   * \return an application container containing a single NRTV server, or an
   *         empty container if Install() has never been called before
   */
  ApplicationContainer GetServer () const;

private:
  NrtvServerHelper* m_serverHelper;
  NrtvClientHelper* m_clientHelper;
  ApplicationContainer m_lastInstalledClients;
  ApplicationContainer m_lastInstalledServer;

}; // end of `class NrtvHelper`


} // end of `namespace ns3`


#endif /* NRTV_HELPER_H */
