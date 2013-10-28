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
  LogComponentEnable ("HttpEntityHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("HttpClient", LOG_LEVEL_ALL);
  LogComponentEnable ("HttpServer", LOG_LEVEL_ALL);
  LogComponentEnable ("HttpClientTracePlot", LOG_LEVEL_ALL);
  //LogComponentEnableAll (LOG_LEVEL_ALL);
  LogComponentDisable ("ObjectBase", LOG_LEVEL_ALL);
  LogComponentDisable ("Object", LOG_LEVEL_ALL);
  LogComponentDisable ("TypeID", LOG_LEVEL_ALL);
//  LogComponentEnable ("TcpNewReno", LOG_LEVEL_ALL);
//  LogComponentEnable ("TcpSocketBase", LOG_LEVEL_ALL);
//  LogComponentEnable ("Socket", LOG_LEVEL_ALL);
//  LogComponentEnable ("PacketSocket", LOG_LEVEL_ALL);
//  LogComponentEnable ("Node", LOG_LEVEL_ALL);
//  LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);
//  LogComponentEnable ("BulkSendApplication", LOG_LEVEL_ALL);
//  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
//  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);

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

  Ptr<HttpClient> httpClient = CreateObject<HttpClient> ();
  httpClient->SetAttribute ("RemoteServerAddress",
                            AddressValue (serverAddress));
  httpClient->SetStartTime (Seconds (5.0));
  nodes.Get (0)->AddApplication (httpClient);

  Ptr<HttpServer> httpServer = CreateObject<HttpServer> ();
  httpServer->SetAttribute ("LocalAddress",
                            AddressValue (serverAddress));
  httpServer->SetStartTime (Seconds (1.0));
  nodes.Get (1)->AddApplication (httpServer);

  Ptr<HttpClientTracePlot> plot = CreateObject<HttpClientTracePlot> (httpClient);

  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;

} // end of `int main (int argc, char *argv[])`
