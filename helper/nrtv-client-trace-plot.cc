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

#include "nrtv-client-trace-plot.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/nrtv-client.h>
#include <ns3/packet.h>
#include <fstream>


NS_LOG_COMPONENT_DEFINE ("NrtvClientTracePlot");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrtvClientTracePlot);


NrtvClientTracePlot::NrtvClientTracePlot (Ptr<NrtvClient> nrtvClient)
  : m_nrtvClient (nrtvClient),
    m_outputName ("nrtv-client-trace")
{
  NS_LOG_FUNCTION (this << nrtvClient << m_outputName);

  if (nrtvClient == 0)
    {
      NS_FATAL_ERROR ("Invalid NRTV Client object is given");
    }

  Initialize ();
}


NrtvClientTracePlot::NrtvClientTracePlot (Ptr<NrtvClient> nrtvClient,
                                          std::string outputName)
  : m_nrtvClient (nrtvClient),
    m_outputName (outputName)
{
  NS_LOG_FUNCTION (this << nrtvClient << m_outputName);

  if (nrtvClient == 0)
    {
      NS_FATAL_ERROR ("Invalid NRTV Client object is given");
    }

  Initialize ();
}


NrtvClientTracePlot::~NrtvClientTracePlot ()
{
  NS_LOG_FUNCTION (this);

  Plot ();
}


TypeId
NrtvClientTracePlot::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrtvClientTracePlot")
    .SetParent<Object> ()
    // for future attributes to configure how the plot is drawn?
  ;
  return tid;
}


Ptr<NrtvClient>
NrtvClientTracePlot::GetNrtvClient () const
{
  return m_nrtvClient;
}


void
NrtvClientTracePlot::SetOutputName (std::string outputName)
{
  NS_LOG_FUNCTION (this << outputName);
  m_outputName = outputName;
}


std::string
NrtvClientTracePlot::GetOutputName () const
{
  return m_outputName;
}


void
NrtvClientTracePlot::Initialize ()
{
  NS_LOG_FUNCTION (this);

  m_nrtvClient->TraceConnectWithoutContext ("Rx",
                                            MakeCallback (&NrtvClientTracePlot::RxCallback,
                                                          this));
  m_packet.SetTitle ("Packet");
  m_packet.SetStyle (Gnuplot2dDataset::IMPULSES);
}


void
NrtvClientTracePlot::Plot ()
{
  NS_LOG_FUNCTION (this << m_outputName);

  Gnuplot plot (m_outputName + ".png");
  plot.SetTitle ("NRTV Client Traffic Trace");
  plot.SetTerminal ("png");
  plot.SetLegend ("Time (in seconds)", "Bytes received");
  plot.AddDataset (m_packet);
  std::string plotFileName = m_outputName + ".plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();
}


void
NrtvClientTracePlot::RxCallback (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  m_packet.Add (Simulator::Now ().GetSeconds (),
                static_cast<double> (packet->GetSize ()));
}


} // end of `namespace ns3`
