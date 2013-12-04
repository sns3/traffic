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

#include "nrtv-kpi-helper.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/nrtv-client.h>
#include <ns3/nrtv-server.h>
#include <ns3/nrtv-helper.h>
#include <ns3/packet.h>
#include <ns3/inet-socket-address.h>
#include <ns3/ipv4.h>
#include <ns3/flow-monitor.h>
#include <ns3/ipv4-flow-classifier.h>
#include <ns3/ipv4-l3-protocol.h>
#include <iomanip>


NS_LOG_COMPONENT_DEFINE ("NrtvKpiHelper");


namespace ns3 {


NrtvKpiHelper::NrtvKpiHelper ()
{
  NS_LOG_FUNCTION (this);
}


NrtvKpiHelper::NrtvKpiHelper (const NrtvHelper * helper)
{
  NS_LOG_FUNCTION (this << helper);

  AddClient (helper->GetClients ());
  SetServer (helper->GetServer ());
}


void
NrtvKpiHelper::AddClient (Ptr<NrtvClient> client)
{
  NS_LOG_FUNCTION (this << client);

  Ipv4Address address = GetAddress (client->GetNode ());
  client->TraceConnect ("Rx", AddressToString (address),
                         MakeCallback (&NrtvKpiHelper::RxCallback, this));

  ClientCounter_t counter;
  counter.rxBytes = 0;
  counter.rxAppLevelPackets = 0;
  counter.rxIpLevelPackets = 0;
  counter.sumPacketDelay = MilliSeconds (0);
  NS_ASSERT_MSG (m_clientCounters.find (address) == m_clientCounters.end (),
                 "Found a client with duplicate address " << address);
  m_clientCounters[address] = counter;

  Ptr<Node> node = client->GetNode ();
  NS_ASSERT (node->GetObject<Ipv4L3Protocol> () != 0);
  m_flowMonitorHelper.Install (node);
}


void
NrtvKpiHelper::AddClient (ApplicationContainer apps)
{
  NS_LOG_FUNCTION (this << apps.GetN ());

  for (ApplicationContainer::Iterator it = apps.Begin ();
       it != apps.End (); it++)
    {
      Ptr<NrtvClient> client = (*it)->GetObject<NrtvClient> ();
      NS_ASSERT (client != 0);
      AddClient (client);
    }
}


void
NrtvKpiHelper::SetServer (Ptr<NrtvServer> server)
{
  NS_LOG_FUNCTION (this << server);

  Ptr<Node> node = server->GetNode ();
  NS_ASSERT (node->GetObject<Ipv4L3Protocol> () != 0);
  m_flowMonitorHelper.Install (node);

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ipv4 != 0,
                 "No IPv4 object is found within node " << node);
  /// \todo Still unclear if the hard-coded indices below will work in any possible cases.
  Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress (1, 0);
  m_serverAddress = interfaceAddress.GetLocal ();
}


void
NrtvKpiHelper::SetServer (ApplicationContainer apps)
{
  NS_LOG_FUNCTION (this << apps.GetN ());
  NS_ASSERT_MSG (apps.GetN () == 1,
                 "Unable to accept more than one server applications as input");
  Ptr<NrtvServer> server = apps.Get (0)->GetObject<NrtvServer> ();
  NS_ASSERT (server != 0);
  SetServer (server);
}


void
NrtvKpiHelper::Print ()
{
  NS_LOG_FUNCTION (this);

  // GET PACKET DELAY INFORMATION FROM FLOW MONITOR

  // in order to make sure all possibly lost packets are accounted for
  Ptr<FlowMonitor> flowMonitor = m_flowMonitorHelper.GetMonitor ();
  flowMonitor->CheckForLostPackets ();

  Ptr<FlowClassifier> baseClassifier = m_flowMonitorHelper.GetClassifier ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (baseClassifier);

  std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats ();
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

  std::cout << " NRTV clients round-up statistics:" << std::endl;
  std::cout << " ----------------------------------------------------------------" << std::endl;
  std::cout << std::setw (16) << "address"
            << std::setw (12) << "bytes"
            << std::setw (12) << "kbps"
            << std::setw (12) << "packets"
            << std::setw (12) << "avg. delay" << std::endl;
  std::cout << " ----------------------------------------------------------------" << std::endl;

  // PRINT ONE LINE FOR EACH CLIENT

  double duration = Simulator::Now ().GetSeconds ();
  double throughput = 0.0;
  double avgDelaySecond = 0.0;

  uint64_t sumRxBytes = 0;
  uint32_t sumRxAppLevelPackets = 0;
  uint32_t sumRxIpLevelPackets = 0;
  double sumPacketDelaySecond = 0.0;

  std::map<Ipv4Address, ClientCounter_t>::const_iterator it2;
  for (it2 = m_clientCounters.begin (); it2 != m_clientCounters.end (); ++it2)
    {
      throughput = static_cast<double> (it2->second.rxBytes * 8) / 1000.0 / duration;
      avgDelaySecond = it2->second.sumPacketDelay.GetSeconds () / it2->second.rxIpLevelPackets;
      std::cout << std::setw (16) << AddressToString (it2->first)
                << std::setw (12) << it2->second.rxBytes
                << std::setw (12) << throughput
                << std::setw (12) << it2->second.rxAppLevelPackets
                << std::setw (12) << avgDelaySecond << std::endl;
      sumRxBytes += it2->second.rxBytes;
      sumRxAppLevelPackets += it2->second.rxAppLevelPackets;
      sumRxIpLevelPackets += it2->second.rxIpLevelPackets;
      sumPacketDelaySecond += it2->second.sumPacketDelay.GetSeconds ();
    }

  // PRINT FOOTER

  throughput = static_cast<double> (sumRxBytes * 8) / 1000.0 / duration;
  avgDelaySecond = sumPacketDelaySecond / sumRxIpLevelPackets;
  std::cout << " ----------------------------------------------------------------" << std::endl;
  std::cout << std::setw (16) << "sum"
            << std::setw (12) << sumRxBytes
            << std::setw (12) << throughput
            << std::setw (12) << sumRxAppLevelPackets
            << std::setw (12) << avgDelaySecond << std::endl;
  std::cout << std::setw (16) << "avg"
            << std::setw (12) << static_cast<double> (sumRxBytes) / m_clientCounters.size ()
            << std::setw (12) << throughput / m_clientCounters.size ()
            << std::setw (12) << static_cast<double> (sumRxAppLevelPackets) / m_clientCounters.size ()
            << std::setw (12) << "n/a" << std::endl;
  std::cout << " ----------------------------------------------------------------" << std::endl;

} // end of `Print()`


void
NrtvKpiHelper::RxCallback (std::string context, Ptr<const Packet> packet)
{
  Ipv4Address address (context.c_str ());
  NS_ASSERT (m_clientCounters.find (address) != m_clientCounters.end ());
  m_clientCounters[address].rxBytes += packet->GetSize ();
  m_clientCounters[address].rxAppLevelPackets++;
}


Ipv4Address
NrtvKpiHelper::GetAddress (Ptr<Node> node)
{
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ipv4 != 0,
                 "No IPv4 object is found within node " << node);
  /// \todo Still unclear if the hard-coded indices below will work in any possible cases.
  Ipv4InterfaceAddress interfaceAddress = ipv4->GetAddress (1, 0);
  return interfaceAddress.GetLocal ();
}


std::string
NrtvKpiHelper::AddressToString (Ipv4Address address)
{
  std::ostringstream oss;
  address.Print (oss);
  return oss.str ();
}


} // end of `namespace ns3`
