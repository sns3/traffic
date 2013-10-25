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

#ifndef HTTP_VARIABLES_H
#define HTTP_VARIABLES_H

#include <ns3/object.h>
#include <ns3/random-variable-stream.h>


namespace ns3 {


/**
 * \brief Container of various random variables for HTTP traffic model.
 */
class HttpVariables : public Object
{
public:
  HttpVariables ();
  static TypeId GetTypeId ();

  bool IsPersistentMode ();
  uint32_t GetMtuSize ();
  uint32_t GetMainObjectSize ();
  uint32_t GetEmbeddedObjectSize ();

  // ATTRIBUTES SETTER AND GETTER METHODS

  void SetStream (int64_t stream);
  int64_t GetStream () const;

  // MAIN OBJECT SIZE ATTRIBUTES SETTER AND GETTER METHODS

  void SetMainObjectSizeMean (uint32_t mean);
  uint32_t GetMainObjectSizeMean () const;

  void SetMainObjectSizeStdDev (uint32_t stdDev);
  uint32_t GetMainObjectSizeStdDev () const;

  void SetMainObjectSizeMin (uint32_t min);
  uint32_t GetMainObjectSizeMin () const;

  void SetMainObjectSizeMax (uint32_t max);
  uint32_t GetMainObjectSizeMax () const;

  /**
   * \f$ \mu = ln(mean) - \frac{1}{2}ln\left(1+\frac{stddev^2}{mean^2}\right) \f$
   */
  double GetMainObjectSizeMu () const;

  /**
   * \f$ \sigma = \sqrt{ln\left(1+\frac{stddev^2}{mean^2}\right)} \f$
   */
  double GetMainObjectSizeSigma () const;

  // EMBEDDED OBJECT SIZE ATTRIBUTES SETTER AND GETTER METHODS

  void SetEmbeddedObjectSizeMean (uint32_t mean);
  uint32_t GetEmbeddedObjectSizeMean () const;

  void SetEmbeddedObjectSizeStdDev (uint32_t stdDev);
  uint32_t GetEmbeddedObjectSizeStdDev () const;

  void SetEmbeddedObjectSizeMin (uint32_t min);
  uint32_t GetEmbeddedObjectSizeMin () const;

  void SetEmbeddedObjectSizeMax (uint32_t max);
  uint32_t GetEmbeddedObjectSizeMax () const;

  /**
   * \f$ \mu = ln(mean) - \frac{1}{2}ln\left(1+\frac{stddev^2}{mean^2}\right) \f$
   */
  double GetEmbeddedObjectSizeMu () const;

  /**
   * \f$ \sigma = \sqrt{ln\left(1+\frac{stddev^2}{mean^2}\right)} \f$
   */
  double GetEmbeddedObjectSizeSigma () const;

  // NUMBER OF EMBEDDED OBJECTS ATTRIBUTES SETTER AND GETTER METHODS

  void SetNumEmbeddedObjectsMean (uint32_t mean);
  uint32_t GetNumEmbeddedObjectsMean () const;

private:

  // INTERNAL METHODS

  void RefreshMainObjectSizeDistribution ();
  void RefreshEmbeddedObjectSizeDistribution ();
  //void RefreshNumEmbeddedObjectsDistribution ();

  // ATTRIBUTES

  int64_t m_stream;
  uint32_t m_mainObjectSizeMean;
  uint32_t m_mainObjectSizeStdDev;
  uint32_t m_mainObjectSizeMin;
  uint32_t m_mainObjectSizeMax;
  uint32_t m_embeddedObjectSizeMean;
  uint32_t m_embeddedObjectSizeStdDev;
  uint32_t m_embeddedObjectSizeMin;
  uint32_t m_embeddedObjectSizeMax;
  uint32_t m_numEmbeddedObjectsMean;

  // RANDOM NUMBER VARIABLES

  Ptr<UniformRandomVariable> m_httpVersionRng;
  Ptr<UniformRandomVariable> m_mtuSizeRng;
  Ptr<LogNormalRandomVariable> m_mainObjectSizeRng;
  Ptr<LogNormalRandomVariable> m_embeddedObjectSizeRng;
  //Ptr<ParetoRandomVariable> m_numEmbeddedObjectsRng;

}; // end of `class HttpVariables`


} // end of `namespace ns3`

#endif /* HTTP_VARIABLES_H */

