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

#include <ns3/http-variables.h>
#include <ns3/command-line.h>
#include <ns3/log.h>
#include <ns3/callback.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <fstream>
#include <cmath>


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
 * - `main-object-size.plt`
 * - `embedded-object-size.plt`
 * - `num-of-embedded-objects.plt`
 * - `reading-time.plt`
 * - `parsing-time.plt`
 *
 * These files are Gnuplot files. Each of these files can be converted to a PNG
 * file, for example by this command:
 *
 *     $ gnuplot main-object-size.plt
 *
 * which will produce `main-object-size.png` file in the same directory. To
 * convert all the Gnuplot files in the directory, the command below can be
 * used:
 *
 *     $ gnuplot *.plt
 *
 */
int main (int argc, char *argv[]); // only forwarding


template<typename T> void
PlotHistogram (Callback<T> valueStream, std::string name,
               std::string plotTitle, std::string axisLabel,
               uint32_t nSamples, T binWidth, double theoreticalMean,
               T max = 0)
{
  std::string plotFileName = name + ".plt";
  std::ofstream ofs (plotFileName.c_str ());

  if (!ofs.is_open ())
    {
      NS_FATAL_ERROR ("Unable to write to " << plotFileName);
    }

  ofs << "set terminal png" << std::endl;
  ofs << "set output '" << name << ".png'" << std::endl;

  ofs << "set title '" << plotTitle << "'" << std::endl;
  ofs << "set xlabel '" << axisLabel << "'" << std::endl;
  ofs << "set ylabel 'Frequency (out of " << nSamples << " samples)'" << std::endl;

  if (static_cast<uint32_t> (max) == 0)
    {
      ofs << "set xrange [0:" << 2 * exp (1) * theoreticalMean << "]" << std::endl;
    }
  else
    {
      ofs << "set xrange [0:" << 1.1 * max << "]" << std::endl;
    }

  ofs << "set yrange [0:]" << std::endl;

  ofs << "set tics out nomirror" << std::endl;
  ofs << "set boxwidth " << binWidth << std::endl;

  ofs << "bin(x)=" << binWidth << "*floor(x/" << binWidth << ")"
      << "+" << (0.5 * binWidth) << std::endl;

  ofs << "plot '-' using (bin($1)):(1.0/" << nSamples << ") smooth freq with boxes notitle, "
      << "'-' title 'Theoretical mean' with points, "
      << "'-' title 'Actual mean' with points" << std::endl;

  // start writing the histogram data points
  T value;
  T sum = 0;
  for (uint32_t i = 0; i < nSamples; i++)
    {
      value = valueStream ();
      sum += value;
      ofs << valueStream () << std::endl;
    }

  ofs << "e" << std::endl;

  // write the theoretical mean data point
  ofs << theoreticalMean << " " << "0" << std::endl;
  ofs << "e" << std::endl;

  // write the actual mean data point
  ofs << (sum / static_cast<double> (nSamples)) << " " << "0" << std::endl;
  ofs << "e" << std::endl;

  ofs.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;
}



// MAIN FUNCTION //////////////////////////////////////////////////////////////


int
main (int argc, char *argv[])
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

  PlotHistogram<uint32_t> (MakeCallback (&HttpVariables::GetMainObjectSize,
                                         httpVariables),
                           "main-object-size",
                           "Histogram of main object size in HTTP traffic model",
                           "Main object size (in bytes)",
                           numOfSamples, 1000, // bar width = 1000 objects
                           static_cast<double> (httpVariables->GetMainObjectSizeMean ()));

  PlotHistogram<uint32_t> (MakeCallback (&HttpVariables::GetEmbeddedObjectSize,
                                         httpVariables),
                           "embedded-object-size",
                           "Histogram of embedded object size in HTTP traffic model",
                           "Embedded object size (in bytes)",
                           numOfSamples, 1000, // bin width = 1000 objects
                           static_cast<double> (httpVariables->GetEmbeddedObjectSizeMean ()));

  PlotHistogram<uint32_t> (MakeCallback (&HttpVariables::GetNumOfEmbeddedObjects,
                                         httpVariables),
                           "num-of-embedded-objects",
                           "Histogram of number of embedded objects in HTTP traffic model",
                           "Number of embedded objects per web page",
                           numOfSamples, 1, // bin width = 1 object
                           httpVariables->GetNumOfEmbeddedObjectsMean (),
                           httpVariables->GetNumOfEmbeddedObjectsMax ());

  PlotHistogram<double> (MakeCallback (&HttpVariables::GetReadingTimeSeconds,
                                       httpVariables),
                         "reading-time",
                         "Histogram of reading time in HTTP traffic model",
                         "Reading time (in seconds)",
                         numOfSamples, 1.0, // bar width = 1 s
                         httpVariables->GetReadingTimeMean ().GetSeconds ());

  PlotHistogram<double> (MakeCallback (&HttpVariables::GetParsingTimeSeconds,
                                       httpVariables),
                         "parsing-time",
                         "Histogram of parsing time in HTTP traffic model",
                         "Parsing time (in seconds)",
                         numOfSamples, 0.01, // bar width = 10 ms
                         httpVariables->GetParsingTimeMean ().GetSeconds ());

  return 0;
}
