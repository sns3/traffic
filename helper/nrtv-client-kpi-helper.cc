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

#include "nrtv-client-kpi-helper.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/nrtv-client.h>
#include <ns3/packet.h>
#include <iomanip>


NS_LOG_COMPONENT_DEFINE ("NrtvClientKpiHelper");


namespace ns3 {


NrtvClientKpiHelper::NrtvClientKpiHelper (Time duration)
  : m_duration (duration)
{
  NS_LOG_FUNCTION (this);
}


NrtvClientKpiHelper::NrtvClientKpiHelper (Time duration,
                                          ApplicationContainer apps)
  : m_duration (duration)
{
  NS_LOG_FUNCTION (this);
  AddApplication (apps);
}


NrtvClientKpiHelper::~NrtvClientKpiHelper ()
{
  NS_LOG_FUNCTION (this);
  Print ();
}


void
NrtvClientKpiHelper::AddApplication (Ptr<NrtvClient> nrtvClient)
{
  NS_LOG_FUNCTION (this << nrtvClient);
  std::string context = GetContext (m_apps.GetN ());
  nrtvClient->TraceConnect ("Rx", context,
                            MakeCallback (&NrtvClientKpiHelper::RxCallback,
                                          this));
  m_rxBytes.push_back (0);
  m_apps.Add (nrtvClient);
  NS_ASSERT (m_rxBytes.size () == m_apps.GetN ());
}


void
NrtvClientKpiHelper::AddApplication (ApplicationContainer apps)
{
  NS_LOG_FUNCTION (this << apps.GetN ());

  for (ApplicationContainer::Iterator it = apps.Begin ();
       it != apps.End (); it++)
    {
      Ptr<NrtvClient> nrtvClient = (*it)->GetObject<NrtvClient> ();
      NS_ASSERT (nrtvClient != 0);
      AddApplication (nrtvClient);
    }
}


void
NrtvClientKpiHelper::Print () const
{
  NS_LOG_FUNCTION (this);

  std::cout << " NRTV clients round-up statistics:" << std::endl;
  std::cout << " -----------------------------------------" << std::endl;
  std::cout << std::setw (5) << "#"
            << std::setw (12) << "bytes"
            << std::setw (12) << "kbps"
            << std::setw (12) << "delay" << std::endl;
  std::cout << " -----------------------------------------" << std::endl;

  uint64_t sumRxBytes = 0;
  double throughput = 0.0;
  //uint32_t sumRxPackets = 0;
  //Time sumRxDelay = MilliSeconds (0);
  Time delay = MilliSeconds (0);
  double duration = m_duration.GetSeconds ();
  uint32_t n = m_rxBytes.size ();

  for (uint32_t i = 0; i < n; i++)
    {
      throughput = static_cast<double> (m_rxBytes[i] * 8) / 1000.0 / duration;
      //Ptr<NrtvClient> app = m_apps.Get (i)->GetObject<NrtvClient> ();
      //delay = app->GetDelayAverage ();
      delay = Seconds (0);
      std::cout << std::setw (5) << i
                << std::setw (12) << m_rxBytes[i]
                << std::setw (12) << throughput
                << std::setw (12) << delay.GetSeconds () << std::endl;
      sumRxBytes += m_rxBytes[i];
      //sumRxDelay += app->GetDelaySum ();
    }

  throughput = static_cast<double> (sumRxBytes * 8) / 1000.0 / duration;
  //delay = Seconds (sumRxDelay.GetSeconds () / static_cast<double> (sumRxPackets));
  delay = Seconds (0);
  std::cout << " -----------------------------------------" << std::endl;
  std::cout << std::setw (5) << "sum"
            << std::setw (12) << sumRxBytes
            << std::setw (12) << throughput << std::endl;
  std::cout << std::setw (5) << "avg"
            << std::setw (12) << static_cast<double> (sumRxBytes) / n
            << std::setw (12) << static_cast<double> (throughput) / n
            << std::setw (12) << delay.GetSeconds () / n << std::endl;
  std::cout << " -----------------------------------------" << std::endl;

} // end of `Print()`


void
NrtvClientKpiHelper::RxCallback (std::string context, Ptr<const Packet> packet)
{
  uint32_t appId = GetAppId (context);
  NS_ASSERT (appId < m_rxBytes.size ());
  m_rxBytes[appId] += packet->GetSize ();
}


std::string
NrtvClientKpiHelper::GetContext (uint32_t appId)
{
  std::ostringstream context;
  context << appId;
  return context.str ();
}

uint32_t
NrtvClientKpiHelper::GetAppId (std::string context)
{
  std::stringstream ss (context);
  uint32_t appId;
  if (!(ss >> appId))
    {
      NS_FATAL_ERROR ("Cannot convert context '" << context << "' to number");
    }
  return appId;
}


} // end of `namespace ns3`
