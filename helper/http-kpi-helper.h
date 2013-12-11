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

#ifndef HTTP_KPI_HELPER_H
#define HTTP_KPI_HELPER_H

#include <ns3/ptr.h>
#include <ns3/nstime.h>
#include <ns3/ipv4-address.h>
#include <ns3/application-container.h>
#include <ns3/flow-monitor-helper.h>


namespace ns3 {


class HttpHelper;
class HttpClient;
class HttpServer;


/**
 * \ingroup traffic
 * \brief Prints a table of Key Performance Indicator (KPI) of each HTTP client.
 *
 * Each row in the table represent a single HTTP client, indicated by its IP
 * address. The available KPIs on each row are:
 * - number of received bytes;
 * - throughput (in kbps), i.e., the number of received bytes divided by the
 *   duration between client's start time and stop time;
 * - number of received main objects;
 * - number of received embedded objects; and
 * - average packet delays (in seconds), i.e., the sum of all packet delays
 *   divided by the number of packets.
 *
 * Most of these KPIs are taken from the application layer, except for packet
 * delays, which are taken from the IP layer (with the help of FlowMonitor).
 * Note that the number of packets in IP layer may differ with the number of
 * packets in the application layer due to fragmentation.
 *
 * At the end of the table, two additional rows are printed to present the
 * sum and the average of all rows. Most of the computation are straightforward,
 * except the following:
 * - average throughput is computed by dividing total bytes received by the
 *   duration of the whole simulation;
 * - sum of average packet delays is computed by dividing the total of all
 *   packet delays from all clients by the total number of IP-level packets
 *   received; and
 * - average of average packet delays (duh) is not computed.
 *
 * See HttpKpiHelper() for usage example.
 */
class HttpKpiHelper
{
public:

  /**
   * \brief Creates an empty instance of helper.
   *
   * After this call, add the clients and server to be included in the KPI table
   * by calling AddClient() and SetServer(). Each of these are overloaded to
   * accept either a single application or a container of applications.
   *
   * Alternatively, the clients and server can also be taken from an HttpHelper
   * object. The other constructor, HttpKpiHelper(const HttpHelper*) is intended
   * for this purpose.
   */
  HttpKpiHelper ();

  /**
   * \brief Creates a new instance of helper which collect KPI from the latest
   *        client and server applications installed by the given HttpHelper.
   *
   * \param helper the HttpHelper instance which the initial client and server
   *               applications are taken from
   *
   * The clients and server for this helper instance are automatically taken
   * from HttpHelper::GetClients() and HttpHelper::GetServer() of the given
   * HttpHelper object instance.
   */
  HttpKpiHelper (const HttpHelper * helper);

  /**
   * \brief Print the KPI table to the standard output.
   *
   * To be called between Simulator::Run() and Simulator::Destroy().
   */
  void Print (); // can't make this const because of non-const methods in FlowMonitor

  /**
   * \brief Add a new client to be included in the KPI table.
   *
   * \param client pointer to the client instance, which must have unique IP
   *               address among the added clients
   *
   *  This method connects this helper instance to trace sources of the client.
   */
  void AddClient (Ptr<HttpClient> client);

  /**
   * \brief Add new clients to be included in the KPI table.
   *
   * \param apps an ApplicationContainer of HttpClient instances
   */
  void AddClient (ApplicationContainer apps);

  /**
   * \param server pointer to the server instance
   *
   * Adding clients to this helper instance is sufficient to produce most of the
   * KPIs, except for packet delay KPI, which requires the server to be
   * specified.
   */
  void SetServer (Ptr<HttpServer> server);

  /**
   * \param apps an ApplicationContainer containing exactly one HttpServer
   *             instance
   *
   * Adding clients to this helper instance is sufficient to produce most of the
   * KPIs, except for packet delay KPI, which requires the server to be
   * specified.
   */
  void SetServer (ApplicationContainer apps);

private:

  // TRACE CALLBACK FUNCTIONS

  void RxCallback (std::string context, Ptr<const Packet> packet);
  void RxMainObjectCallback (std::string context);
  void RxEmbeddedObjectCallback (std::string context);

  // STATIC UTILITY FUNCTIONS

  /**
   * \param node pointer to the node
   * \return the IP address of the given node
   */
  static Ipv4Address GetAddress (Ptr<Node> node);

  /**
   * \param address an IP address
   * \return the given IP address in the form of printable string
   */
  static std::string AddressToString (Ipv4Address address);

  /**
   * \param bytes number of bytes received
   * \param duration the length of time spent receiving
   * \return the equivalent throughput in kilobits per second
   */
  static double GetKbps (uint64_t bytes, Time duration);

  FlowMonitorHelper m_flowMonitorHelper;
  Ipv4Address m_serverAddress;

  struct ClientCounter_t
  {
    uint64_t rxBytes;
    uint32_t rxMainObjects;
    uint32_t rxEmbeddedObjects;
    uint32_t rxIpLevelPackets;
    Time sumPacketDelay;
    Time appStart;
    Time appStop;
  };
  std::map<Ipv4Address, ClientCounter_t> m_clientCounters;

}; // end of `class HttpKpiHelper`


} // end of `namespace ns3`


#endif /* HTTP_KPI_HELPER_H */
