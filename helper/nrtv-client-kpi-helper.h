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

#ifndef NRTV_CLIENT_KPI_HELPER_H
#define NRTV_CLIENT_KPI_HELPER_H

#include <ns3/nstime.h>
#include <ns3/application-container.h>
#include <ns3/ptr.h>


namespace ns3 {


class NrtvClient;


class NrtvClientKpiHelper
{
public:
  NrtvClientKpiHelper (Time duration);
  NrtvClientKpiHelper (Time duration, ApplicationContainer apps);
  ~NrtvClientKpiHelper ();

  void AddApplication (Ptr<NrtvClient> nrtvClient);
  void AddApplication (ApplicationContainer apps);

private:
  void Print () const;

  void RxCallback (std::string context, Ptr<const Packet> packet);

  static std::string GetContext (uint32_t appId);
  static uint32_t GetAppId (std::string context);

  Time m_duration;
  ApplicationContainer m_apps;
  std::vector<uint64_t> m_rxBytes;

}; // end of `class NrtvClientKpiHelper`


} // end of `namespace ns3`


#endif /* NRTV_CLIENT_KPI_HELPER_H */
