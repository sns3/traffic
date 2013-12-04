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
#include <ns3/flow-monitor-module.h>
#include <ns3/traffic-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("HttpP2pExample");


/*
double
GetAvgHop (uint32_t timesForwarded, uint32_t rxPackets)
{
  if (rxPackets > 0)
    {
      return 1 + ((double) timesForwarded / (double) rxPackets);
    }
  else
    {
      return 0;
    }
}


double
GetAvgHop (FlowMonitor::FlowStats stats)
{
  return GetAvgHop (stats.timesForwarded, stats.rxPackets);
}


double
GetPacketLossRatio (uint32_t lostPackets, uint32_t rxPackets)
{
  uint32_t totalPackets = lostPackets + rxPackets;
  if (totalPackets > 0)
    {
      return (double) lostPackets / (double) totalPackets * 100;
    }
  else
    {
      return 0;
    }
}


double
GetPacketLossRatio (FlowMonitor::FlowStats stats)
{
  return GetPacketLossRatio (stats.lostPackets, stats.rxPackets);
}


double
GetPdf (uint32_t rxPackets, uint32_t txPackets)
{
  if (txPackets > 0)
    {
      return (double) rxPackets / (double) txPackets * 100;
    }
  else
    {
      return 0;
    }
}


double
GetPdf (FlowMonitor::FlowStats stats)
{
  return GetPdf (stats.rxPackets, stats.txPackets);
}


double
GetAvgDelay (double delaySum, uint32_t rxPackets)
{
  if (rxPackets > 0)
    {
      return (double) delaySum / (double) rxPackets;
    }
  else
    {
      return 0;
    }
}


double
GetAvgDelay (FlowMonitor::FlowStats stats)
{
  return GetAvgDelay (stats.delaySum.GetSeconds (), stats.rxPackets);
}


std::string
PrintFlowMonitorFullStats (Ptr<FlowMonitor> flowMon,
                           Ptr<Ipv4FlowClassifier> classifier,
                           std::string fs = "\t")
{
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowMon->GetFlowStats ();
  std::ostringstream headOss, sumOss, bodyOss;

  // WRITE HEADER OF THE TABLE
  headOss
    << "id"           << fs           // running number
    << "src"          << fs           // source address
    << "dest"         << fs           // destination address
    << "protocol"     << fs           // protocol ID

    << "txStart"      << fs           // time (in milliseconds) when the first packet is transmitted
    << "txEnd"        << fs           // time (in milliseconds) when the last packet is transmitted
    << "txPackets"    << fs           // number of Tx packets
    << "txKbytes"     << fs           // Tx kilobytes
    << "txSec"        << fs           // Tx elapsed time in seconds
    << "txKbps"       << fs           // Tx throughput in kbit/s

    << "rxStart"      << fs           // time (in milliseconds) when the first packet is received
    << "rxEnd"        << fs           // time (in milliseconds) when the last packet is received
    << "rxPackets"    << fs           // number of Rx packets
    << "rxKbytes"     << fs           // Rx kilobytes
    << "rxSec"        << fs           // Rx time in seconds
    << "rxKbps"       << fs           // Rx throughput in kbit/s

    << "lostPackets"  << fs           // number of packets assumed to be lost (missing over 10 seconds)
    << "dropPackets"  << fs           // dropped packets (including lost packets)
    << "plr"          << fs           // packet loss ratio (in %)
    << "pdf"          << fs           // Rx/Tx ratio (in %)
    << "avgHop"       << fs           // average hop count
    << "avgDelay"     << std::endl;   // average delay per Rx packets

  // INITIALIZE ACCUMULATORS
  uint32_t    sumTxPackets = 0,       sumRxPackets = 0;
  uint64_t    sumTxKbytes = 0,        sumRxKbytes = 0;
  double      sumTxThroughput = 0,    sumRxThroughput = 0;
  uint32_t    sumLostPackets = 0,     sumDropPackets = 0;
  uint32_t sumTimesForwarded = 0;
  double sumDelay = 0;
  double sumPacketLossRatio, sumPdf, sumAvgHop, sumAvgDelay;

  // INITIALIZE CALCULATION VARIABLES
  double      txKbytes = 0,           rxKbytes = 0;
  double      txDuration = 0,         rxDuration = 0;
  double      txThroughput = 0,       rxThroughput = 0;

  // ITERATE EACH FLOW STATS
  bool outAvail = false;
  std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it;
  for (it = stats.begin (); it != stats.end (); ++it)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (it->first);

      // CALCULATE THROUGHPUT
      txKbytes = (double) it->second.txBytes / 1000;
      rxKbytes = (double) it->second.rxBytes / 1000;
      txDuration = it->second.timeLastTxPacket.GetSeconds () - it->second.timeFirstTxPacket.GetSeconds ();
      rxDuration = it->second.timeLastRxPacket.GetSeconds () - it->second.timeFirstRxPacket.GetSeconds ();
      txThroughput = (txDuration > 0) ? txKbytes * 8 / txDuration : 0;
      rxThroughput = (rxDuration > 0) ? rxKbytes * 8 / rxDuration : 0;

      // WRITE A SINGLE ROW IN THE BODY OF THE TABLE
      bodyOss
        << it->first                                          << fs           // id
        << t.sourceAddress << ":" << t.sourcePort             << fs           // src
        << t.destinationAddress << ":" << t.destinationPort   << fs           // dest
        << (uint32_t) t.protocol                              << fs           // protocol

        << it->second.timeFirstTxPacket.GetMilliSeconds ()    << fs           // txStart
        << it->second.timeLastTxPacket.GetMilliSeconds ()     << fs           // txEnd
        << it->second.txPackets                               << fs           // txPackets
        << txKbytes                                           << fs           // txKbytes
        << txDuration                                         << fs           // txSec
        << txThroughput                                       << fs           // txKbps

        << it->second.timeFirstRxPacket.GetMilliSeconds ()    << fs           // rxStart
        << it->second.timeLastRxPacket.GetMilliSeconds ()     << fs           // rxEnd
        << it->second.rxPackets                               << fs           // rxPackets
        << rxKbytes                                           << fs           // rxKbytes
        << rxDuration                                         << fs           // rxSec
        << rxThroughput                                       << fs           // rxKbps

        << it->second.lostPackets                             << fs           // lostPackets
        << it->second.packetsDropped.size ()                  << fs           // dropPackets
        << GetPacketLossRatio (it->second)                    << fs           // plr
        << GetPdf (it->second)                                << fs           // pdf
        << GetAvgHop (it->second)                             << fs           // avgHop
        << GetAvgDelay (it->second)                           << std::endl;   // avgDelay

      // INCREMENT ACCUMULATORS
      sumTxPackets += it->second.txPackets;
      sumTxKbytes += txKbytes;
      sumTxThroughput += txThroughput;

      sumRxPackets += it->second.txPackets;
      sumRxKbytes += txKbytes;
      sumRxThroughput += txThroughput;

      sumLostPackets += it->second.lostPackets;
      sumDropPackets += it->second.packetsDropped.size ();

      sumTimesForwarded += it->second.timesForwarded;
      sumDelay += it->second.delaySum.GetSeconds ();

      outAvail = true;
    } // end of for (it = stats.begin (); it != stats.end (); ++it)

  // CONSOLIDATE OVERVIEW STATISTICS
  sumPacketLossRatio = GetPacketLossRatio(sumLostPackets, sumRxPackets);
  sumPdf = GetPdf(sumRxPackets, sumTxPackets);
  sumAvgHop = GetAvgHop (sumTimesForwarded, sumRxPackets);
  sumAvgDelay = GetAvgDelay(sumDelay, sumRxPackets);
  std::string blank = ""; // used to indicate empty value

  // WRITE SUMMARY
  sumOss
    << "sum"                  << fs           // id
    << blank                  << fs           // src
    << blank                  << fs           // dest
    << blank                  << fs           // protocol

    << blank                  << fs           // txStart
    << blank                  << fs           // txEnd
    << sumTxPackets           << fs           // txPackets
    << sumTxKbytes            << fs           // txKbytes
    << blank                  << fs           // txSec
    << sumTxThroughput        << fs           // txKbps

    << blank                  << fs           // rxStart
    << blank                  << fs           // rxEnd
    << sumRxPackets           << fs           // rxPackets
    << sumRxKbytes            << fs           // rxKbytes
    << blank                  << fs           // rxSec
    << sumRxThroughput        << fs           // rxKbps

    << sumLostPackets         << fs           // lostPackets
    << sumDropPackets         << fs           // dropPackets
    << sumPacketLossRatio     << fs           // plr
    << sumPdf                 << fs           // pdf
    << sumAvgHop              << fs           // avgHop
    << sumAvgDelay            << std::endl;   // avgDelay

  // RETURN EVERYTHING
  if (outAvail)
    {
      std::ostringstream oss;
      oss << headOss.str () << sumOss.str () << bodyOss.str ();
      return oss.str ();
    }
  else
    {
      return "";
    }

} // end of std::string PrintFlowMonitorFullStats


void
PrintToFile (std::string filePath, std::string content)
{
  std::ofstream outFile;
  outFile.open (filePath.c_str ());
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Cannot open file " << filePath);
    }
  outFile << content;
  outFile.close ();
}
*/


/**
 * \ingroup traffic
 * \brief Simple example of two nodes connected by a point-to-point link. One
 *        acts as a web server, while the other node acts as the web browsing
 *        client.
 */
int main (int argc, char *argv[])
{
  LogComponentEnable ("HttpClient", LOG_PREFIX_ALL);
  LogComponentEnable ("HttpServer", LOG_PREFIX_ALL);
  LogComponentEnable ("HttpClient", LOG_WARN);
  LogComponentEnable ("HttpServer", LOG_WARN);
  LogComponentEnable ("HttpClient", LOG_ERROR);
  LogComponentEnable ("HttpServer", LOG_ERROR);

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

  HttpHelper httpHelper ("ns3::TcpSocketFactory");
  httpHelper.InstallUsingIpv4 (nodes.Get (1), nodes.Get (0));
  httpHelper.GetServer ().Start (Seconds (0.0));
  httpHelper.GetClients ().Start (Seconds (1.0));

  Ptr<HttpClientTracePlot> plot = CreateObject<HttpClientTracePlot> (
    httpHelper.GetClients ().Get (0)->GetObject<HttpClient> ());

//  FlowMonitorHelper flowMonHelper;
//  Ptr<FlowMonitor> flowMon = flowMonHelper.InstallAll ();

  Simulator::Stop (Seconds (1000.0));
  Simulator::Run ();

//  Ptr<Ipv4FlowClassifier> classifier;
//  flowMon->CheckForLostPackets (); // in order to make sure all possibly lost packets are accounted for
//  Ptr<FlowClassifier> baseClassifier = flowMonHelper.GetClassifier ();
//  classifier = DynamicCast<Ipv4FlowClassifier> (baseClassifier);
//
//  PrintToFile ("http-p2p-example-flow.txt",
//               PrintFlowMonitorFullStats (flowMon, classifier));

  Simulator::Destroy ();
  return 0;

} // end of `int main (int argc, char *argv[])`
