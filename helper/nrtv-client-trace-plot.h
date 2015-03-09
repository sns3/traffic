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

#ifndef NRTV_CLIENT_TRACE_PLOT_H
#define NRTV_CLIENT_TRACE_PLOT_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/gnuplot.h>


namespace ns3 {


class NrtvClient;
class Packet;
class Address;


/**
 * \ingroup traffic
 * \brief Installed on an NrtvClient application, this helper class will
 *        generate a Gnuplot file out of the Rx traffic experienced by the
 *        application.
 *
 * One helper is intended only for one NrtvClient and will generate one Gnuplot
 * file at the end of the simulation. Usage example:
 *
 *     Ptr<NrtvClient> nrtvClient = apps.Get (0);
 *     Ptr<NrtvClientTracePlot> nrtvClientTracePlot =
 *         CreateObject<NrtvClientTracePlot> (nrtvClient);
 *
 * By default, the Gnuplot file name is "nrtv-client-trace.plt". This can be
 * modified by calling the SetOutputName() method, or by using the extended
 * constructor.
 *
 * The Gnuplot file can be converted to a PNG file, for example by using this
 * command:
 *
 *     $ gnuplot nrtv-client-trace.plt
 *
 * The above command generates a new file "nrtv-client-trace.png" in the same
 * directory.
 */
class NrtvClientTracePlot : public Object
{
public:
  /**
   * \brief Creates a new object instance which generates a plot file named
   *        "nrtv-client-trace.plt".
   *
   * \param nrtvClient the client application from which the traffic data is
   *                   taken and generated as a plot
   */
  NrtvClientTracePlot (Ptr<NrtvClient> nrtvClient);

  /**
   * \brief Creates a new object instance which generates a plot file with the
   *        specified name.
   *
   * \param nrtvClient the client application from which the traffic data is
   *                   taken and generated as a plot
   * \param outputName the name of the plot file, e.g., specifying the value
   *                   "output" will generate "output.plt" file, which then can
   *                   be converted to "output.png"
   */
  NrtvClientTracePlot (Ptr<NrtvClient> nrtvClient, std::string outputName);

  /// Object destructor, which will generate the output.
  ~NrtvClientTracePlot ();

  // Inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \return pointer to the currently active client application.
   */
  Ptr<NrtvClient> GetNrtvClient () const;

  /**
   * \param outputName the name of the plot file, e.g., specifying the value
   *                   "output" will generate "output.plt" file, which then can
   *                   be converted to "output.png"
   */
  void SetOutputName (std::string outputName);

  /**
   * \return the name of the plot file
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

  void RxCallback (Ptr<const Packet> packet, const Address & from);


  Ptr<NrtvClient> m_nrtvClient;  ///< The currently active client application.
  std::string m_outputName;      ///< The name of the plot file.
  Gnuplot2dDataset m_packet;     ///< Size of every packet received.

}; // end of `class NrtvClientTracePlot`


} // end of `namespace ns3`


#endif /* NRTV_CLIENT_TRACE_PLOT_H */
