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

#include "http-client-trace-plot.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/http-client.h>
#include <ns3/packet.h>
#include <fstream>


NS_LOG_COMPONENT_DEFINE ("HttpClientTracePlot");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HttpClientTracePlot);


HttpClientTracePlot::HttpClientTracePlot (Ptr<HttpClient> httpClient)
  : m_httpClient (httpClient),
    m_outputName ("http-client-trace")
{
  NS_LOG_FUNCTION (this << httpClient << m_outputName);

  if (httpClient == 0)
    {
      NS_FATAL_ERROR ("Invalid HTTP Client object is given");
    }

  Initialize ();
}


HttpClientTracePlot::HttpClientTracePlot (Ptr<HttpClient> httpClient,
                                          std::string outputName)
  : m_httpClient (httpClient),
    m_outputName (outputName)
{
  NS_LOG_FUNCTION (this << httpClient << m_outputName);

  if (httpClient == 0)
    {
      NS_FATAL_ERROR ("Invalid HTTP Client object is given");
    }

  Initialize ();
}


HttpClientTracePlot::~HttpClientTracePlot ()
{
  NS_LOG_FUNCTION (this);

  Plot ();
}


TypeId
HttpClientTracePlot::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::HttpClientTracePlot")
    .SetParent<Object> ()
  ;
  return tid;
}


Ptr<HttpClient>
HttpClientTracePlot::GetHttpClient () const
{
  return m_httpClient;
}


void
HttpClientTracePlot::SetOutputName (std::string outputName)
{
  NS_LOG_FUNCTION (this << outputName);
  m_outputName = outputName;
}


std::string
HttpClientTracePlot::GetOutputName () const
{
  return m_outputName;
}


void
HttpClientTracePlot::Initialize ()
{
  NS_LOG_FUNCTION (this);

  // CONNECT TO TRACE SOURCES

  m_httpClient->TraceConnectWithoutContext ("TxMainObjectRequest",
                                            MakeCallback (&HttpClientTracePlot::TxMainObjectRequestCallback,
                                                          this));
  m_httpClient->TraceConnectWithoutContext ("TxEmbeddedObjectRequest",
                                            MakeCallback (&HttpClientTracePlot::TxEmbeddedObjectRequestCallback,
                                                          this));
  m_httpClient->TraceConnectWithoutContext ("RxMainObjectPacket",
                                            MakeCallback (&HttpClientTracePlot::RxMainObjectPacketCallback,
                                                          this));
  m_httpClient->TraceConnectWithoutContext ("RxMainObject",
                                            MakeCallback (&HttpClientTracePlot::RxMainObjectCallback,
                                                          this));
  m_httpClient->TraceConnectWithoutContext ("RxEmbeddedObjectPacket",
                                            MakeCallback (&HttpClientTracePlot::RxEmbeddedObjectPacketCallback,
                                                          this));
  m_httpClient->TraceConnectWithoutContext ("RxEmbeddedObject",
                                            MakeCallback (&HttpClientTracePlot::RxEmbeddedObjectCallback,
                                                          this));

  // CREATE NEW DATA SETS

  m_request.SetTitle ("Request");
  m_request.SetStyle (Gnuplot2dDataset::IMPULSES);

  m_responseMainObject.SetTitle ("Response of main object");
  m_responseMainObject.SetStyle (Gnuplot2dDataset::IMPULSES);

  m_responseEmbeddedObject.SetTitle ("Response of embedded object");
  m_responseEmbeddedObject.SetStyle (Gnuplot2dDataset::IMPULSES);

  m_mainObjectAck.SetTitle ("Done receiving a main object");
  m_mainObjectAck.SetStyle (Gnuplot2dDataset::POINTS);

  m_embeddedObjectAck.SetTitle ("Done receiving an embedded object");
  m_embeddedObjectAck.SetStyle (Gnuplot2dDataset::POINTS);

  m_isRequestExist = false;
  m_isResponseMainObjectExist = false;
  m_isResponseEmbeddedObjectExist = false;
  m_isMainObjectAckExist = false;
  m_isEmbeddedObjectAckExist = false;

} // end of `void Initialize ()`


void
HttpClientTracePlot::Plot ()
{
  NS_LOG_FUNCTION (this << m_outputName);

  Gnuplot plot (m_outputName + ".png");
  plot.SetTitle ("HTTP Client Traffic Trace");
  plot.SetTerminal ("png");
  plot.SetLegend ("Time (in seconds)", "Bytes transmitted");

  if (m_isRequestExist)
    {
      plot.AddDataset (m_request);
    }

  if (m_isResponseMainObjectExist)
    {
      plot.AddDataset (m_responseMainObject);
    }

  if (m_isResponseEmbeddedObjectExist)
    {
      plot.AddDataset (m_responseEmbeddedObject);
    }

  if (m_isMainObjectAckExist)
    {
      plot.AddDataset (m_mainObjectAck);
    }

  if (m_isEmbeddedObjectAckExist)
    {
      plot.AddDataset (m_embeddedObjectAck);
    }

  std::string plotFileName = m_outputName + ".plt";
  std::ofstream plotFile (plotFileName.c_str ());
  plot.GenerateOutput (plotFile);
  plotFile.close ();

} // end of `void Plot ()`


void
HttpClientTracePlot::TxMainObjectRequestCallback (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  m_request.Add (Simulator::Now ().GetSeconds (),
                 static_cast<double> (packet->GetSize ()));
  m_isRequestExist = true;
}


void
HttpClientTracePlot::TxEmbeddedObjectRequestCallback (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  m_request.Add (Simulator::Now ().GetSeconds (),
                 static_cast<double> (packet->GetSize ()));
  m_isRequestExist = true;
}


void
HttpClientTracePlot::RxMainObjectPacketCallback (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  m_responseMainObject.Add (Simulator::Now ().GetSeconds (),
                            static_cast<double> (packet->GetSize ()));
  m_isResponseMainObjectExist = true;
}


void
HttpClientTracePlot::RxMainObjectCallback ()
{
  NS_LOG_FUNCTION (this);
  m_mainObjectAck.Add (Simulator::Now ().GetSeconds (), 0.0);
  m_isMainObjectAckExist = true;
}


void
HttpClientTracePlot::RxEmbeddedObjectPacketCallback (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  m_responseEmbeddedObject.Add (Simulator::Now ().GetSeconds (),
                                static_cast<double> (packet->GetSize ()));
  m_isResponseEmbeddedObjectExist = true;
}


void
HttpClientTracePlot::RxEmbeddedObjectCallback ()
{
  NS_LOG_FUNCTION (this);
  m_embeddedObjectAck.Add (Simulator::Now ().GetSeconds (), 0.0);
  m_isEmbeddedObjectAckExist = true;
}


} // end of `namespace ns3`
