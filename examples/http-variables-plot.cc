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

#include <ns3/core-module.h>
#include <ns3/traffic-module.h>


using namespace ns3;


/**
 * \ingroup traffic
 * \brief Example script for plotting histograms from some of the random
 *        variable distributions used in HTTP traffic model.
 *
 * The script repeatedly draws random samples from the distributions and then
 * plot a histogram for each distribution. By default, 100 000 samples are
 * taken, which can be modified through a command line argument, for example:
 *
 *     $ ./waf --run="http-variables-plot --numOfSamples=1000000"
 *
 * The script generates the following files in the ns-3 project root directory:
 * - `http-main-object-size.plt`
 * - `http-embedded-object-size.plt`
 * - `http-num-of-embedded-objects.plt`
 * - `http-reading-time.plt`
 * - `http-parsing-time.plt`
 *
 * These files are Gnuplot files. Each of these files can be converted to a PNG
 * file, for example by this command:
 *
 *     $ gnuplot http-main-object-size.plt
 *
 * which will produce `main-object-size.png` file in the same directory. To
 * convert all the Gnuplot files in the directory, the command below can be
 * used:
 *
 *     $ gnuplot *.plt
 *
 */
int main (int argc, char *argv[])
{
  uint32_t numOfSamples = 100000;

  // read command line arguments given by the user
  CommandLine cmd;
  cmd.AddValue ("numOfSamples",
                "Number of samples taken from each random number distribution",
                numOfSamples);
  cmd.Parse (argc, argv);

  Ptr<HttpVariables> httpVariables = CreateObject<HttpVariables> ();
  //httpVariables->SetStream (99);

  HistogramPlotHelper::Plot<uint32_t> (MakeCallback (&HttpVariables::GetMainObjectSize,
                                                     httpVariables),
                                       "http-main-object-size",
                                       "Histogram of main object size in HTTP traffic model",
                                       "Main object size (in bytes)",
                                       numOfSamples, 1000, // bar width = 1000 bytes
                                       static_cast<double> (httpVariables->GetMainObjectSizeMean ()));

  HistogramPlotHelper::Plot<uint32_t> (MakeCallback (&HttpVariables::GetEmbeddedObjectSize,
                                                     httpVariables),
                                       "http-embedded-object-size",
                                       "Histogram of embedded object size in HTTP traffic model",
                                       "Embedded object size (in bytes)",
                                       numOfSamples, 1000, // bin width = 1000 bytes
                                       static_cast<double> (httpVariables->GetEmbeddedObjectSizeMean ()));

  HistogramPlotHelper::Plot<uint32_t> (MakeCallback (&HttpVariables::GetNumOfEmbeddedObjects,
                                                     httpVariables),
                                       "http-num-of-embedded-objects",
                                       "Histogram of number of embedded objects in HTTP traffic model",
                                       "Number of embedded objects per web page",
                                       numOfSamples, 1, // bin width = 1 object
                                       httpVariables->GetNumOfEmbeddedObjectsMean (),
                                       httpVariables->GetNumOfEmbeddedObjectsMax ());

  HistogramPlotHelper::Plot<double> (MakeCallback (&HttpVariables::GetReadingTimeSeconds,
                                                   httpVariables),
                                     "http-reading-time",
                                     "Histogram of reading time in HTTP traffic model",
                                     "Reading time (in seconds)",
                                     numOfSamples, 1.0, // bar width = 1 s
                                     httpVariables->GetReadingTimeMean ().GetSeconds ());

  HistogramPlotHelper::Plot<double> (MakeCallback (&HttpVariables::GetParsingTimeSeconds,
                                                   httpVariables),
                                     "http-parsing-time",
                                     "Histogram of parsing time in HTTP traffic model",
                                     "Parsing time (in seconds)",
                                     numOfSamples, 0.01, // bar width = 10 ms
                                     httpVariables->GetParsingTimeMean ().GetSeconds ());

  return 0;

} // end of `int main (int argc, char *argv[])`
