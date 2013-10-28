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

NS_LOG_COMPONENT_DEFINE ("HttpP2pExample");

int
main (int argc, char *argv[])
{
  LogComponentEnableAll (LOG_PREFIX_ALL);
  LogComponentEnable ("HttpClient", LOG_LEVEL_ALL);
  LogComponentEnable ("HttpServer", LOG_LEVEL_ALL);

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
  Ipv4Address serverAddress = interfaces.GetAddress (1);

  HttpClientHelper httpClientHelper ("ns3::TcpSocketFactory", serverAddress);
  ApplicationContainer clientApps = httpClientHelper.Install (nodes.Get (0));
  clientApps.Start (Seconds (1.0));

  HttpServerHelper httpServerHelper ("ns3::TcpSocketFactory", serverAddress);
  ApplicationContainer serverApps = httpServerHelper.Install (nodes.Get (1));
  serverApps.Start (Seconds (0.0));

  Ptr<HttpClientTracePlot> plot = CreateObject<HttpClientTracePlot> (
    clientApps.Get (0)->GetObject<HttpClient> ());

  Simulator::Stop (Seconds (1000.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;

} // end of `int main (int argc, char *argv[])`
