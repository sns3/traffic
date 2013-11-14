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

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/applications-module.h>
#include <ns3/traffic-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NrtvP2pExample");

/**
 * \ingroup traffic
 * \brief
 */
int main (int argc, char *argv[])
{
  LogComponentEnableAll (LOG_PREFIX_ALL);
  //LogComponentEnable ("NrtvApplication", LOG_LEVEL_ALL);
  //LogComponentEnable ("NrtvClient", LOG_LEVEL_ALL);
  //LogComponentEnable ("NrtvServer", LOG_LEVEL_ALL);
  //LogComponentEnable ("HttpServer", LOG_LEVEL_ALL);

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);
//  Ipv4Address clientAddress = interfaces.GetAddress (0);
  Ipv4Address serverAddress = interfaces.GetAddress (1);

  std::string protocol = "ns3::TcpSocketFactory";

//  HttpServerHelper httpServerHelper (protocol, clientAddress);
//  ApplicationContainer clientApps = httpServerHelper.Install (nodes.Get (0));
//  clientApps.Start (Seconds (1.0));
//  //clientApps.Stop (Seconds (30.0));

//  Ptr<NrtvApplication> nrtvApp = CreateObject<NrtvApplication> ();
//  nrtvApp->SetAttribute ("Protocol", StringValue (protocol));
//  nrtvApp->SetAttribute ("RemoteAddress", AddressValue (clientAddress));
////  nrtvApp->SetAttribute ("RemotePort", UintegerValue (1935));
//  nrtvApp->SetStartTime (Seconds (2.0));
//  nodes.Get (1)->AddApplication (nrtvApp);

  Ptr<NrtvClient> nrtvClient = CreateObject<NrtvClient> ();
  nrtvClient->SetAttribute ("Protocol", StringValue (protocol));
  nrtvClient->SetAttribute ("RemoteServerAddress", AddressValue (serverAddress));
  nrtvClient->SetStartTime (Seconds (2.0));
  nodes.Get (0)->AddApplication (nrtvClient);

  Ptr<NrtvServer> nrtvServer = CreateObject<NrtvServer> ();
  nrtvServer->SetAttribute ("Protocol", StringValue (protocol));
  nrtvServer->SetAttribute ("LocalAddress", AddressValue (serverAddress));
  nrtvServer->SetStartTime (Seconds (1.0));
  nodes.Get (1)->AddApplication (nrtvServer);

//  Ptr<NrtvApplicationTracePlot> plot = CreateObject<NrtvApplicationTracePlot> (nrtvApp);
//  Ptr<NrtvServerTracePlot> plot = CreateObject<NrtvServerTracePlot> (nrtvServer);

  Simulator::Stop (Seconds (1000.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;

} // end of `int main (int argc, char *argv[])`
