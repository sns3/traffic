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
 * \ingroup traffic
 * \brief Installed on an HttpClient application, this helper class will
 *        generate a Gnuplot file out of the Tx and Rx traffic experienced by
 *        the application.
 *
 * One helper is intended only for one HttpClient and will generate one Gnuplot
 * file at the end of the simulation. Usage example:
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
 * The above command generates a new file "http-client-trace.png" in the same
 * directory.
 */
class HttpClientTracePlot : public Object
{
public:

  /**
   * \brief Creates a new object instance which generates a plot file named
   *        "http-client-trace.plt".
   *
   * \param httpClient the client application from which the traffic data is
   *                   taken and generated as a plot
   */
  HttpClientTracePlot (Ptr<HttpClient> httpClient);

  /**
   * \brief Creates a new object instance which generates a plot file with the
   *        specified name.
   *
   * \param httpClient the client application from which the traffic data is
   *                   taken and generated as a plot
   * \param outputName the name of the plot file, e.g., specifying the value
   *                   "output" will generate "output.plt" file, which then can
   *                   be converted to "output.png"
   */
  HttpClientTracePlot (Ptr<HttpClient> httpClient, std::string outputName);

  /// Object destructor, which will generate the output.
  ~HttpClientTracePlot ();

  // Inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \return pointer to the currently active client application.
   */
  Ptr<HttpClient> GetHttpClient () const;

  /**
   * \param outputName the name of the plot file, e.g., specifying the value
   *                   "output" will generate "output.plt" file, which then can
   *                   be converted to "output.png"
   */
  void SetOutputName (std::string outputName);

  /**
   * return the name of the plot file
   */
  std::string GetOutputName () const;

private:

  /**
   * \internal
   * \brief Connecting the object to trace sources in the client application and
   *        creating the Gnuplot datasets for storing the gathered data.
   */
  void Initialize ();

  /// Generating the plot.
  void Plot ();

  // TRACE CALLBACK FUNCTIONS

  void TxMainObjectRequestCallback (Ptr<const Packet> packet);
  void TxEmbeddedObjectRequestCallback (Ptr<const Packet> packet);
  void RxMainObjectPacketCallback (Ptr<const Packet> packet);
  void RxMainObjectCallback ();
  void RxEmbeddedObjectPacketCallback (Ptr<const Packet> packet);
  void RxEmbeddedObjectCallback ();

  Ptr<HttpClient> m_httpClient;  ///< The currently active client application.
  std::string m_outputName;      ///< The name of the plot file.

  /// Size of every client's request for either main or embedded objects.
  Gnuplot2dDataset m_request;
  /// Size of every packet of main object received.
  Gnuplot2dDataset m_responseMainObject;
  /// Size of every packet of embedded object received.
  Gnuplot2dDataset m_responseEmbeddedObject;
  /// Indications of every main object completely received.
  Gnuplot2dDataset m_mainObjectAck;
  /// Indications of every embedded object completely received.
  Gnuplot2dDataset m_embeddedObjectAck;

  /// True if there has been at least one client's request sent.
  bool m_isRequestExist;
  /// True if there has been at least one packet of main object received.
  bool m_isResponseMainObjectExist;
  /// True if there has been at least one packet of embedded object received.
  bool m_isResponseEmbeddedObjectExist;
  /// True if there has been at least one whole main object received.
  bool m_isMainObjectAckExist;
  /// True if there has been at least one whole embedded object received.
  bool m_isEmbeddedObjectAckExist;

}; // end of `class HttpClientTracePlot`


} // end of `namespace ns3`


#endif /* HTTP_CLIENT_TRACE_PLOT_H */
