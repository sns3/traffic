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

#ifndef NRTV_KPI_HELPER_H
#define NRTV_KPI_HELPER_H

#include <ns3/nstime.h>
#include <ns3/application-container.h>
#include <ns3/ptr.h>
#include <ns3/flow-monitor-helper.h>


namespace ns3 {


class NrtvHelper;
class NrtvClient;
class NrtvServer;


/**
 * \brief Prints a table of Key Performance Indicator (KPI) of each NRTV client.
 */
class NrtvKpiHelper
{
public:
  /**
   * \brief Creates an empty instance of helper.
   */
  NrtvKpiHelper ();

  /**
   * \brief Creates a new instance of helper which collect KPI from the latest
   *        server and client applications installed by the given NrtvHelper.
   * \param helper the NrtvHelper instance which the initial server and client
   *               applications are taken from
   */
  NrtvKpiHelper (const NrtvHelper * helper);

  /**
   * \brief Print the KPI to standard output.
   */
  void Print ();

private:
  void AddClient (Ptr<NrtvClient> client);
  void AddClient (ApplicationContainer apps);
  void SetServer (Ptr<NrtvServer> server);
  void SetServer (ApplicationContainer apps);

  FlowMonitorHelper m_flowMonitorHelper;
  Ipv4Address m_serverAddress;

}; // end of `class NrtvKpiHelper`


} // end of `namespace ns3`


#endif /* NRTV_KPI_HELPER_H */
