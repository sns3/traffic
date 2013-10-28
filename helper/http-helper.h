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
 * \brief Helper to make it easier to instantiate an ns3::HttpClient on a set
 *        of nodes.
 */
class HttpClientHelper
{
public:
  /**
   * \brief Create a HttpClientHelper to make it easier to work with
   * HttpClient applications.
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
   *        but _not_ the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Install an ns3::HttpClient on each node of the input container
   *        configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which an HttpClient will be
   *          installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * \brief Install an ns3::HttpClient on each node of the input container
   *        configured with all the attributes set with SetAttribute.
   *
   * \param node The node on which an HttpClient will be installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * \brief Install an ns3::HttpClient on each node of the input container
   *        configured with all the attributes set with SetAttribute.
   *
   * \param nodeName The name of the node on which an HttpClient will be
   *                 installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * \internal
   * \brief Install an ns3::HttpClient on the node configured with all the
   *        attributes set with SetAttribute.
   *
   * \param node The node on which an HttpClient will be installed
   * \return Ptr to the application installed
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;

}; // end of `class HttpClientHelper`



/**
 * \brief Helper to make it easier to instantiate an ns3::HttpServer on a set
 *        of nodes.
 */
class HttpServerHelper
{
public:
  /**
   * \brief Create a HttpServerHelper to make it easier to work with
   * HttpServer applications.
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
   *        but _not_ the socket attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Install an ns3::HttpServer on each node of the input container
   *        configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which an HttpServer will be
   *          installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * \brief Install an ns3::HttpServer on each node of the input container
   *        configured with all the attributes set with SetAttribute.
   *
   * \param node The node on which an HttpServer will be installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * \brief Install an ns3::HttpServer on each node of the input container
   *        configured with all the attributes set with SetAttribute.
   *
   * \param nodeName The name of the node on which an HttpServer will be
   *                 installed
   * \return Container of Ptr to the applications installed
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * \internal
   * \brief Install an ns3::HttpServer on the node configured with all the
   *        attributes set with SetAttribute.
   *
   * \param node The node on which an HttpClient will be installed
   * \return Ptr to the application installed
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;

}; // end of `class HttpServerHelper`


} // end of `namespace ns3`


#endif /* HTTP_HELPER_H */
