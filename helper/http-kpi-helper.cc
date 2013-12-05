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

#include "http-kpi-helper.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/http-client.h>
#include <ns3/http-server.h>
#include <ns3/http-helper.h>
#include <ns3/packet.h>
#include <ns3/inet-socket-address.h>
#include <ns3/ipv4.h>
#include <ns3/flow-monitor.h>
#include <ns3/ipv4-flow-classifier.h>
#include <ns3/ipv4-l3-protocol.h>
#include <iomanip>


NS_LOG_COMPONENT_DEFINE ("HttpKpiHelper");


namespace ns3 {


HttpKpiHelper::HttpKpiHelper ()
{
  NS_LOG_FUNCTION (this);
}


HttpKpiHelper::HttpKpiHelper (const HttpHelper * helper)
{
  NS_LOG_FUNCTION (this << helper);

  AddClient (helper->GetClients ());
  SetServer (helper->GetServer ());
}


void
HttpKpiHelper::AddClient (Ptr<HttpClient> client)
{
  NS_LOG_FUNCTION (this << client);

  const Ipv4Address address = GetAddress (client->GetNode ());
  const std::string context = AddressToString (address);
  client->TraceConnect ("RxMainObjectPacket", context,
                        MakeCallback (&HttpKpiHelper::RxCallback, this));
  client->TraceConnect ("RxEmbeddedObjectPacket", context,
                        MakeCallback (&HttpKpiHelper::RxCallback, this));
  client->TraceConnect ("RxMainObject", context,
                        MakeCallback (&HttpKpiHelper::RxMainObjectCallback,
                                      this));
  client->TraceConnect ("RxEmbeddedObject", context,
                        MakeCallback (&HttpKpiHelper::RxEmbeddedObjectCallback,
                                      this));

  ClientCounter_t counter;
  counter.rxBytes = 0;
  counter.rxMainObjects = 0;
  counter.rxEmbeddedObjects = 0;
  counter.rxIpLevelPackets = 0;
  counter.sumPacketDelay = MilliSeconds (0);
  counter.appStart = client->GetStartTime ();
  if (client->IsScheduledToStop ())
    {
      counter.appStop = client->GetStopTime ();
    }
  else
    {
      NS_ASSERT (Simulator::Now () <= client->GetStartTime ());
      counter.appStop = Simulator::Now ();
    }

  NS_ASSERT_MSG (m_clientCounters.find (address) == m_clientCounters.end (),
                 "Found a client with duplicate address " << address);
  m_clientCounters[address] = counter;

  Ptr<Node> node = client->GetNode ();
  NS_ASSERT (node->GetObject<Ipv4L3Protocol> () != 0);
  m_flowMonitorHelper.Install (node);
}


void
HttpKpiHelper::AddClient (ApplicationContainer apps)
{
  NS_LOG_FUNCTION (this << apps.GetN ());

  for (ApplicationContainer::Iterator it = apps.Begin ();
       it != apps.End (); it++)
    {
      Ptr<HttpClient> client = (*it)->GetObject<HttpClient> ();
      NS_ASSERT (client != 0);
      AddClient (client);
    }
}


void
HttpKpiHelper::SetServer (Ptr<HttpServer> server)
{
  NS_LOG_FUNCTION (this << server);

  Ptr<Node> node = server->GetNode ();
  NS_ASSERT (node->GetObject<Ipv4L3Protocol> () != 0);
  m_flowMonitorHelper.Install (node);
  m_serverAddress = GetAddress (node);
}


void
HttpKpiHelper::SetServer (ApplicationContainer apps)
{
  NS_LOG_FUNCTION (this << apps.GetN ());
  NS_ASSERT_MSG (apps.GetN () == 1,
                 "Unable to accept more than one server applications as input");
  Ptr<HttpServer> server = apps.Get (0)->GetObject<HttpServer> ();
  NS_ASSERT (server != 0);
  SetServer (server);
}


void
HttpKpiHelper::Print ()
{
  NS_LOG_FUNCTION (this);

  // GET PACKET DELAY INFORMATION FROM FLOW MONITOR

  // in order to make sure all possibly lost packets are accounted for
  const Ptr<FlowMonitor> flowMonitor = m_flowMonitorHelper.GetMonitor ();
  flowMonitor->CheckForLostPackets ();

  const Ptr<FlowClassifier> baseClassifier = m_flowMonitorHelper.GetClassifier ();
  const Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (baseClassifier);

  const std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats ();
  std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it1;
  for (it1 = stats.begin (); it1 != stats.end (); ++it1)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (it1->first);
      if (t.sourceAddress == m_serverAddress) // only DL direction is considered
        {
          NS_ASSERT (m_clientCounters.find (t.destinationAddress) != m_clientCounters.end ());
          m_clientCounters[t.destinationAddress].rxIpLevelPackets += it1->second.rxPackets;
          m_clientCounters[t.destinationAddress].sumPacketDelay += it1->second.delaySum;
        }
    }

  // PRINT HEADER

  std::cout << "HTTP clients round-up statistics:" << std::endl;
  std::cout << "-------------------------------------------------------------------------" << std::endl;
  std::cout << std::setw (12) << "address"
            << std::setw (12) << "bytes"
            << std::setw (12) << "kbps"
            << std::setw (12) << "main obj."
            << std::setw (12) << "emb. obj."
            << std::setw (12) << "avg. delay" << std::endl;
  std::cout << "-------------------------------------------------------------------------" << std::endl;

  // PRINT ONE LINE FOR EACH CLIENT

  uint64_t sumRxBytes = 0;
  uint32_t sumRxMainObjects = 0;
  uint32_t sumRxEmbeddedObjects = 0;
  uint32_t sumRxIpLevelPackets = 0;
  double sumPacketDelaySecond = 0.0;

  std::map<Ipv4Address, ClientCounter_t>::const_iterator it2;
  for (it2 = m_clientCounters.begin (); it2 != m_clientCounters.end (); ++it2)
    {
      const Time userDuration =
        (it2->second.appStop <= it2->second.appStart) ?
        (Simulator::Now () - it2->second.appStart) : // app stops as simulation stops
        (it2->second.appStop - it2->second.appStart); // app stops before simulation stops
      const double userThroughput = GetKbps (it2->second.rxBytes, userDuration);
      const double userAvgDelaySecond = it2->second.sumPacketDelay.GetSeconds () / it2->second.rxIpLevelPackets;
      std::cout << std::setw (12) << AddressToString (it2->first)
                << std::setw (12) << it2->second.rxBytes
                << std::setw (12) << userThroughput
                << std::setw (12) << it2->second.rxMainObjects
                << std::setw (12) << it2->second.rxEmbeddedObjects
                << std::setw (12) << userAvgDelaySecond << std::endl;
      sumRxBytes += it2->second.rxBytes;
      sumRxMainObjects += it2->second.rxMainObjects;
      sumRxEmbeddedObjects += it2->second.rxEmbeddedObjects;
      sumRxIpLevelPackets += it2->second.rxIpLevelPackets;
      sumPacketDelaySecond += it2->second.sumPacketDelay.GetSeconds ();
    }

  // PRINT FOOTER

  const double sumThroughput = GetKbps (sumRxBytes, Simulator::Now ());
  const double avgDelaySecond = sumPacketDelaySecond / sumRxIpLevelPackets;
  std::cout << "-------------------------------------------------------------------------" << std::endl;
  std::cout << std::setw (12) << "sum"
            << std::setw (12) << sumRxBytes
            << std::setw (12) << sumThroughput
            << std::setw (12) << sumRxMainObjects
            << std::setw (12) << sumRxEmbeddedObjects
            << std::setw (12) << avgDelaySecond << std::endl;
  std::cout << std::setw (12) << "avg"
            << std::setw (12) << static_cast<double> (sumRxBytes) / m_clientCounters.size ()
            << std::setw (12) << sumThroughput / m_clientCounters.size ()
            << std::setw (12) << static_cast<double> (sumRxMainObjects) / m_clientCounters.size ()
            << std::setw (12) << static_cast<double> (sumRxEmbeddedObjects) / m_clientCounters.size ()
            << std::setw (12) << "n/a" << std::endl;
  std::cout << "-------------------------------------------------------------------------" << std::endl;

} // end of `Print()`


void
HttpKpiHelper::RxCallback (std::string context, Ptr<const Packet> packet)
{
  Ipv4Address address (context.c_str ());
  NS_ASSERT (m_clientCounters.find (address) != m_clientCounters.end ());
  m_clientCounters[address].rxBytes += packet->GetSize ();
}


void
HttpKpiHelper::RxMainObjectCallback (std::string context)
{
  Ipv4Address address (context.c_str ());
  NS_ASSERT (m_clientCounters.find (address) != m_clientCounters.end ());
  m_clientCounters[address].rxMainObjects++;
}

void
HttpKpiHelper::RxEmbeddedObjectCallback (std::string context)
{
  Ipv4Address address (context.c_str ());
  NS_ASSERT (m_clientCounters.find (address) != m_clientCounters.end ());
  m_clientCounters[address].rxEmbeddedObjects++;
}


Ipv4Address
HttpKpiHelper::GetAddress (Ptr<Node> node)
{
  const Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ipv4 != 0,
                 "No IPv4 object is found within node " << node);
  /// \todo Still unclear if the hard-coded indices below will work in any possible cases.
  const Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress (1, 0);
  return interfaceAddress.GetLocal ();
}


std::string
HttpKpiHelper::AddressToString (Ipv4Address address)
{
  std::ostringstream oss;
  address.Print (oss);
  return oss.str ();
}


double
HttpKpiHelper::GetKbps (uint64_t bytes, Time duration)
{
  return static_cast<double> (bytes * 8) / 1000.0 / duration.GetSeconds ();
}


} // end of `namespace ns3`
