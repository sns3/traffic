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

  // PRINT HEADER

  std::cout << " NRTV clients round-up statistics:" << std::endl;
  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << std::setw (5)  << "#"
            << std::setw (12) << "bytes"
            << std::setw (12) << "kbps"
            << std::setw (6)  << "pkets"
            << std::setw (12) << "avg. delay" << std::endl;
  std::cout << " -----------------------------------------------" << std::endl;

  double duration = Simulator::Now ().GetSeconds ();
  uint64_t rxBytes = 0;
  double throughput = 0.0;
  double delay = 0.0;

  uint64_t sumRxBytes = 0;
  uint32_t sumRxPackets = 0;
  double sumDelay = 0.0;

  // in order to make sure all possibly lost packets are accounted for
  Ptr<FlowMonitor> flowMonitor = m_flowMonitorHelper.GetMonitor ();
  flowMonitor->CheckForLostPackets ();

  // PRINT ONE LINE FOR EACH CLIENT

  Ptr<FlowClassifier> baseClassifier = m_flowMonitorHelper.GetClassifier ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (baseClassifier);

  uint16_t numOfFlows = 0;
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats ();
  std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it;
  for (it = stats.begin (); it != stats.end (); ++it)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (it->first);
      if (t.sourceAddress == m_serverAddress) // only DL direction is considered
        {
          numOfFlows++;
          // uncount 40-byte TCP header
          NS_ASSERT (it->second.rxBytes > (40 * it->second.rxPackets));
          rxBytes = it->second.rxBytes - (40 * it->second.rxPackets);
          throughput = static_cast<double> (rxBytes * 8) / 1000.0 / duration;
          delay = it->second.delaySum.GetSeconds () / it->second.rxPackets;
          std::cout << std::setw (5)  << numOfFlows
                    << std::setw (12) << rxBytes
                    << std::setw (12) << throughput
                    << std::setw (6)  << it->second.rxPackets
                    << std::setw (12) << delay << std::endl;
          sumRxBytes += rxBytes;
          sumRxPackets += it->second.rxPackets;
          sumDelay += delay;
        }
    }

  // PRINT FOOTER

  throughput = static_cast<double> (sumRxBytes * 8) / 1000.0 / duration;
  delay = sumDelay / sumRxPackets;
  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << std::setw (5) << "sum"
            << std::setw (12) << sumRxBytes
            << std::setw (12) << throughput
            << std::setw (6)  << sumRxPackets
            << std::setw (12) << delay << std::endl;
  std::cout << std::setw (5) << "avg"
            << std::setw (12) << static_cast<double> (sumRxBytes) / numOfFlows
            << std::setw (12) << throughput / numOfFlows
            << std::setw (6)  << static_cast<double> (sumRxPackets) / numOfFlows
            << std::setw (12) << "n/a" << std::endl;
  std::cout << " -----------------------------------------------" << std::endl;

} // end of `Print()`


} // end of `namespace ns3`
