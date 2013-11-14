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


/**
 * \brief Installed on an NrtvClient application, this helper class will
 *        generate a Gnuplot file out of the Rx traffic experienced by the
 *        application.
 *
 * One helper is intended only for one NrtvClient and will generate one Gnuplot
 * file. Usage example:
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
 */
class NrtvClientTracePlot : public Object
{
public:
  NrtvClientTracePlot (Ptr<NrtvClient> nrtvClient);
  NrtvClientTracePlot (Ptr<NrtvClient> nrtvClient, std::string outputName);
  ~NrtvClientTracePlot ();
  static TypeId GetTypeId ();

  Ptr<NrtvClient> GetNrtvClient () const;
  void SetOutputName (std::string outputName);
  std::string GetOutputName () const;

private:
  void Initialize ();
  void Plot ();

  void RxCallback (Ptr<const Packet> packet);

  Ptr<NrtvClient> m_nrtvClient;
  std::string m_outputName;

  Gnuplot2dDataset m_packet;

  bool m_isPacketExist;

}; // end of `class NrtvClientTracePlot`


} // end of `namespace ns3`


#endif /* NRTV_CLIENT_TRACE_PLOT_H */
