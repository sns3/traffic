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

#ifndef HISTOGRAM_PLOT_HELPER_H
#define HISTOGRAM_PLOT_HELPER_H

#include <ns3/callback.h>
#include <fstream>
#include <cmath>


namespace ns3 {


class HistogramPlotHelper
{
public:
  template<typename T> static void
  Plot (Callback<T> valueStream, std::string name,
        std::string plotTitle, std::string axisLabel,
        uint32_t numOfSamples, T binWidth,
        double theoreticalMean, T max = 0);
};


/*
 * Defined here in .h file, because static templated function like this is not
 * visible to linker if put in .cc file.
 */

template<typename T> void
HistogramPlotHelper::Plot (Callback<T> valueStream, std::string name,
                           std::string plotTitle, std::string axisLabel,
                           uint32_t numOfSamples, T binWidth,
                           double theoreticalMean, T max)
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
  ofs << "set ylabel 'Frequency (out of " << numOfSamples << " samples)'"
      << std::endl;

  if (static_cast<uint32_t> (max) == 0)
    {
      ofs << "set xrange [0:" << 2 * exp (1) * theoreticalMean << "]"
          << std::endl;
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

  ofs << "plot '-' using (bin($1)):(1.0/" << numOfSamples << ") "
      << "smooth freq with boxes notitle, "
      << "'-' title 'Theoretical mean' with points, "
      << "'-' title 'Actual mean' with points" << std::endl;

  // start writing the histogram data points
  T value;
  T sum = 0;
  for (uint32_t i = 0; i < numOfSamples; i++)
    {
      value = valueStream ();
      sum += value;
      ofs << valueStream () << std::endl;
    }

  ofs << "e" << std::endl;

  // write the theoretical mean data point
  ofs << theoreticalMean << " 0" << std::endl;
  ofs << "e" << std::endl;

  // write the actual mean data point
  ofs << (sum / static_cast<double> (numOfSamples)) << " 0" << std::endl;
  ofs << "e" << std::endl;

  ofs.close ();

  std::cout << "Output file written: " << plotFileName << std::endl;
}


} // end of `namespace ns3`


#endif /* HISTOGRAM_PLOT_HELPER_H */
