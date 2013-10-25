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
#include <ns3/object.h>
#include <ns3/log.h>
#include <ns3/callback.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <fstream>
#include <cmath>


using namespace ns3;


template<typename T> void
PlotHistogram (Callback<T> valueStream, std::string name,
               std::string plotTitle, std::string axisLabel,
               uint32_t nSamples, uint32_t resolution,
               T min, T max, double mean)
{
  T binWidth = (max - min) / static_cast<T> (resolution);
  PlotHistogram<T> (valueStream, name, plotTitle, axisLabel, nSamples, binWidth,
                    mean);
}


template<typename T> void
PlotHistogram (Callback<T> valueStream, std::string name,
               std::string plotTitle, std::string axisLabel,
               uint32_t nSamples, T binWidth, double mean)
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
  ofs << "set xrange [0:" << 2 * exp (1) * mean << "]" << std::endl;
  ofs << "set yrange [0:]" << std::endl;

  ofs << "set tics out nomirror" << std::endl;
  ofs << "set boxwidth " << binWidth << std::endl;

  ofs << "bin(x)=" << binWidth << "*floor(x/" << binWidth << ")"
      << "+" << (0.5 * binWidth) << std::endl;

  ofs << "plot '-' using (bin($1)):(1.0/" << nSamples << ") smooth freq with boxes notitle, "
      << "'-' title 'Mean' with points" << std::endl;

  for (uint32_t i = 0; i < nSamples; i++)
    {
      ofs << valueStream () << std::endl;
    }

  ofs << "e" << std::endl;

  ofs << mean << " " << "0" << std::endl;
  ofs << "e" << std::endl;

  ofs.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;
}



// MAIN FUNCTION //////////////////////////////////////////////////////////////


int
main (int argc, char *argv[])
{
  LogComponentEnable ("HttpVariables", LOG_INFO);

  Ptr<HttpVariables> httpVariables = CreateObject<HttpVariables> ();
  //httpVariables->SetStream (99);

  UintegerValue uintMean;
  httpVariables->GetAttribute ("MainObjectSizeMean", uintMean);
  PlotHistogram<uint32_t> (MakeCallback (&HttpVariables::GetMainObjectSize,
                                         httpVariables),
                           "main-object-size",
                           "Histogram of main object size in HTTP traffic model",
                           "Main object size (in bytes)",
                           1000, 1000, static_cast<double> (uintMean.Get ()));

  httpVariables->GetAttribute ("EmbeddedObjectSizeMean", uintMean);
  PlotHistogram<uint32_t> (MakeCallback (&HttpVariables::GetEmbeddedObjectSize,
                                         httpVariables),
                           "embedded-object-size",
                           "Histogram of embedded object size in HTTP traffic model",
                           "Embedded object size (in bytes)",
                           1000, 1000, static_cast<double> (uintMean.Get ()));

  DoubleValue doubleMean;
  httpVariables->GetAttribute ("NumOfEmbeddedObjectsMean", doubleMean);
  PlotHistogram<uint32_t> (MakeCallback (&HttpVariables::GetNumOfEmbeddedObjects,
                                         httpVariables),
                          "num-of-embedded-objects",
                          "Histogram of number of embedded objects in HTTP traffic model",
                          "Number of embedded objects per web page",
                          1000, 1, doubleMean.Get ());

  TimeValue timeMean;
  httpVariables->GetAttribute ("ReadingTimeMean", timeMean);
  PlotHistogram<double> (MakeCallback (&HttpVariables::GetReadingTimeSeconds,
                                       httpVariables),
                         "reading-time",
                         "Histogram of reading time in HTTP traffic model",
                         "Reading time (in seconds)",
                         1000, 1.0, timeMean.Get ().GetSeconds ());

  httpVariables->GetAttribute ("ParsingTimeMean", timeMean);
  PlotHistogram<double> (MakeCallback (&HttpVariables::GetParsingTimeSeconds,
                                       httpVariables),
                         "parsing-time",
                         "Histogram of parsing time in HTTP traffic model",
                         "Parsing time (in seconds)",
                         1000, 0.01, timeMean.Get ().GetSeconds ());

  return 0;
}
