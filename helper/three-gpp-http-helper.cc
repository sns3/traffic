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
 * Converted to 3GPP HTTP web browsing traffic models by:
 * - Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include <ns3/names.h>
#include <ns3/ipv4.h>
#include "three-gpp-http-helper.h"

namespace ns3 {


// 3GPP HTTP CLIENT HELPER /////////////////////////////////////////////////////////

ThreeGppHttpClientHelper::ThreeGppHttpClientHelper (const Address &address)
{
  m_factory.SetTypeId ("ns3::ThreeGppHttpClient");
  m_factory.Set ("RemoteServerAddress", AddressValue (address));
}

void
ThreeGppHttpClientHelper::SetAttribute (const std::string &name,
                                		const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
ThreeGppHttpClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
ThreeGppHttpClientHelper::Install (const std::string &nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
ThreeGppHttpClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
ThreeGppHttpClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}


// HTTP SERVER HELPER /////////////////////////////////////////////////////////

ThreeGppHttpServerHelper::ThreeGppHttpServerHelper (const Address &address)
{
  m_factory.SetTypeId ("ns3::ThreeGppHttpServer");
  m_factory.Set ("LocalAddress", AddressValue (address));
}

void
ThreeGppHttpServerHelper::SetAttribute (const std::string &name,
										const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
ThreeGppHttpServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
ThreeGppHttpServerHelper::Install (const std::string &nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
ThreeGppHttpServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
ThreeGppHttpServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}

// NRTV HELPER ////////////////////////////////////////////////////////////////

ThreeGppHttpHelper::ThreeGppHttpHelper ()
{
  Address invalidAddr;
  m_clientHelper = new ThreeGppHttpClientHelper (invalidAddr);
  m_serverHelper = new ThreeGppHttpServerHelper (invalidAddr);
  m_httpVariables = CreateObject<ThreeGppHttpVariables> ();
}

ThreeGppHttpHelper::~ThreeGppHttpHelper ()
{
  delete m_clientHelper;
  delete m_serverHelper;
}

void
ThreeGppHttpHelper::SetClientAttribute (std::string name, const AttributeValue &value)
{
  m_clientHelper->SetAttribute (name, value);
}

void
ThreeGppHttpHelper::SetServerAttribute (std::string name, const AttributeValue &value)
{
  m_serverHelper->SetAttribute (name, value);
}

void
ThreeGppHttpHelper::SetVariablesAttribute (std::string name, const AttributeValue &value)
{
	m_httpVariables->SetAttribute (name, value);
}

ApplicationContainer
ThreeGppHttpHelper::InstallUsingIpv4 (Ptr<Node> serverNode, NodeContainer clientNodes)
{
  ApplicationContainer ret; // the return value of the function

  Ptr<Ipv4> ipv4 = serverNode->GetObject<Ipv4> ();
  if (ipv4 == 0)
    {
      NS_FATAL_ERROR ("No IPv4 object is found within the server node " << serverNode);
    }
  else
    {
      /// Still unclear if the hard-coded indices below will work in any possible cases.
      const Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress (1, 0);
      const Ipv4Address serverAddress = interfaceAddress.GetLocal ();

      m_serverHelper->SetAttribute ("LocalAddress", AddressValue (serverAddress));

      m_lastInstalledServer = m_serverHelper->Install (serverNode);
      ret.Add (m_lastInstalledServer);

			m_clientHelper->SetAttribute ("RemoteServerAddress",
																		AddressValue (serverAddress));
			m_lastInstalledClients = m_clientHelper->Install (clientNodes);
			ret.Add (m_lastInstalledClients);
    }

  return ret;
}

ApplicationContainer
ThreeGppHttpHelper::InstallUsingIpv4 (Ptr<Node> serverNode, Ptr<Node> clientNode)
{
  return InstallUsingIpv4 (serverNode, NodeContainer (clientNode));
}

ApplicationContainer
ThreeGppHttpHelper::GetClients () const
{
  return m_lastInstalledClients;
}

ApplicationContainer
ThreeGppHttpHelper::GetServer () const
{
  return m_lastInstalledServer;
}


} // end of `namespace ns3`
