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

#ifndef HTTP_CLIENT_TRACE_PLOT_H
#define HTTP_CLIENT_TRACE_PLOT_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/gnuplot.h>


namespace ns3 {


class HttpClient;
class Packet;


/**
 * \brief Installed on an HttpClient application, this helper class will
 *        generate a Gnuplot file out of the Tx and Rx traffic experienced by
 *        the application.
 *
 * One helper is intended only for one HttpClient and will generate one Gnuplot
 * file. Usage example:
 *
 *     Ptr<HttpClient> httpClient = apps.Get (0);
 *     Ptr<HttpClientTracePlot> httpClientTracePlot =
 *         CreateObject<HttpClientTracePlot> (httpClient);
 *
 * By default, the Gnuplot file name is "http-client-trace.plt". This can be
 * modified by calling the SetOutputName() method, or by using the extended
 * constructor.
 *
 * The Gnuplot file can be converted to a PNG file, for example by using this
 * command:
 *
 *     $ gnuplot http-client-trace.plt
 *
 */
class HttpClientTracePlot : public Object
{
public:
  HttpClientTracePlot (Ptr<HttpClient> httpClient);
  HttpClientTracePlot (Ptr<HttpClient> httpClient, std::string outputName);
  ~HttpClientTracePlot ();
  static TypeId GetTypeId ();

  Ptr<HttpClient> GetHttpClient () const;
  void SetOutputName (std::string outputName);
  std::string GetOutputName () const;

private:
  void Initialize ();
  void Plot ();

  void TxMainObjectRequestCallback (Ptr<const Packet> packet);
  void TxEmbeddedObjectRequestCallback (Ptr<const Packet> packet);
  void RxMainObjectPacketCallback (Ptr<const Packet> packet);
  void RxMainObjectCallback ();
  void RxEmbeddedObjectPacketCallback (Ptr<const Packet> packet);
  void RxEmbeddedObjectCallback ();

  Ptr<HttpClient> m_httpClient;
  std::string m_outputName;

  Gnuplot2dDataset m_request;
  Gnuplot2dDataset m_responseMainObject;
  Gnuplot2dDataset m_responseEmbeddedObject;
  Gnuplot2dDataset m_mainObjectAck;
  Gnuplot2dDataset m_embeddedObjectAck;

  bool m_isRequestExist;
  bool m_isResponseMainObjectExist;
  bool m_isResponseEmbeddedObjectExist;
  bool m_isMainObjectAckExist;
  bool m_isEmbeddedObjectAckExist;

}; // end of `class HttpClientTracePlot`


} // end of `namespace ns3`


#endif /* HTTP_CLIENT_TRACE_PLOT_H */
