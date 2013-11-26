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

#include "nrtv-helper.h"
#include <ns3/string.h>
#include <ns3/inet-socket-address.h>
#include <ns3/ipv4.h>
#include <ns3/names.h>

namespace ns3 {


// NRTV CLIENT HELPER /////////////////////////////////////////////////////////

NrtvClientHelper::NrtvClientHelper (std::string protocol, Address address)
{
  m_factory.SetTypeId ("ns3::NrtvClient");
  m_factory.Set ("Protocol", StringValue (protocol));
  m_factory.Set ("RemoteServerAddress", AddressValue (address));
}

void
NrtvClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
NrtvClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
NrtvClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
NrtvClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
NrtvClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}


// NRTV SERVER HELPER /////////////////////////////////////////////////////////

NrtvServerHelper::NrtvServerHelper (std::string protocol, Address address)
{
  m_factory.SetTypeId ("ns3::NrtvServer");
  m_factory.Set ("Protocol", StringValue (protocol));
  m_factory.Set ("LocalAddress", AddressValue (address));
}

void
NrtvServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
NrtvServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
NrtvServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
NrtvServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
NrtvServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}


// NRTV HELPER ////////////////////////////////////////////////////////////////

NrtvHelper::NrtvHelper (std::string protocol)
{
  Address invalidAddr;
  m_clientHelper = new NrtvClientHelper (protocol, invalidAddr);
  m_serverHelper = new NrtvServerHelper (protocol, invalidAddr);
}

NrtvHelper::~NrtvHelper ()
{
  delete m_clientHelper;
  delete m_serverHelper;
}

void
NrtvHelper::SetClientAttribute (std::string name, const AttributeValue &value)
{
  m_clientHelper->SetAttribute (name, value);
}

void
NrtvHelper::SetServerAttribute (std::string name, const AttributeValue &value)
{
  m_serverHelper->SetAttribute (name, value);
}

ApplicationContainer
NrtvHelper::InstallUsingIpv4 (Ptr<Node> serverNode, NodeContainer clientNodes)
{
  ApplicationContainer ret;

  Ptr<Ipv4> ipv4 = serverNode->GetObject<Ipv4> ();
  if (ipv4 == 0)
    {
      NS_FATAL_ERROR ("No IPv4 object is found within the server node " << serverNode);
    }
  else
    {
      /// \todo Still unclear if the hard-coded indices below will work in any possible cases.
      Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress (1, 0);
      Ipv4Address serverAddress = interfaceAddress.GetLocal ();

      m_serverHelper->SetAttribute ("LocalAddress",
                                    AddressValue (serverAddress));
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
NrtvHelper::InstallUsingIpv4 (Ptr<Node> serverNode, Ptr<Node> clientNode)
{
  ApplicationContainer ret;

  Ptr<Ipv4> ipv4 = serverNode->GetObject<Ipv4> ();
  if (ipv4 == 0)
    {
      NS_FATAL_ERROR ("No IPv4 object is found within the server node " << serverNode);
    }
  else
    {
      /// \todo Still unclear if the hard-coded indices below will work in any possible cases.
      Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress (1, 0);
      Ipv4Address serverAddress = interfaceAddress.GetLocal ();

      m_serverHelper->SetAttribute ("LocalAddress",
                                    AddressValue (serverAddress));
      m_lastInstalledServer = m_serverHelper->Install (serverNode);
      ret.Add (m_lastInstalledServer);

      m_clientHelper->SetAttribute ("RemoteServerAddress",
                                    AddressValue (serverAddress));
      m_lastInstalledClients = m_clientHelper->Install (clientNode);
      ret.Add (m_lastInstalledClients);
    }

  return ret;
}

ApplicationContainer
NrtvHelper::GetClients () const
{
  return m_lastInstalledClients;
}

ApplicationContainer
NrtvHelper::GetServer () const
{
  return m_lastInstalledServer;
}


} // end of `namespace ns3`
