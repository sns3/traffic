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
#include <ns3/nstime.h>
#include <ns3/random-variable-stream.h>


namespace ns3 {


/**
 * \brief Wrapper of LogNormalRandomVariable for use in HTTP traffic model.
 *
 * Provides configurability using mean and standard deviation. In addition,
 * returned values are truncated using a given range.
 */
class HttpLogNormalVariable : public LogNormalRandomVariable
{
public:
  HttpLogNormalVariable ();
  static TypeId GetTypeId ();

  uint32_t GetTruncatedInteger ();

  void SetMin (uint32_t min);
  uint32_t GetMin () const;

  void SetMax (uint32_t max);
  uint32_t GetMax () const;

  void SetMean (uint32_t mean);
  uint32_t GetMean () const;

  void SetStdDev (uint32_t stdDev);
  uint32_t GetStdDev () const;

private:
  void RefreshBaseParameters ();

  uint32_t m_min;
  uint32_t m_max;
  uint32_t m_mean;
  uint32_t m_stdDev;

}; // end of `class HttpLogNormalVariable`


/**
 * \brief Container of various random variables for HTTP traffic model.
 */
class HttpVariables : public Object
{
public:
  HttpVariables ();
  static TypeId GetTypeId ();

  // THE MORE USEFUL METHODS

  bool IsPersistentMode ();
  uint32_t GetMtuSize ();
  uint32_t GetMainObjectSize ();
  uint32_t GetMainObjectSizeKbytes ();
  uint32_t GetEmbeddedObjectSize ();
  uint32_t GetEmbeddedObjectSizeKbytes ();
  uint32_t GetNumOfEmbeddedObjects ();
  Time GetReadingTime ();
  double GetReadingTimeSeconds ();
  Time GetParsingTime ();
  double GetParsingTimeSeconds ();

  void SetStream (int64_t stream);

  // MAIN OBJECT SIZE ATTRIBUTES SETTER AND GETTER METHODS

  void SetMainObjectSizeMean (uint32_t mean);
  void SetMainObjectSizeStdDev (uint32_t stdDev);
  void SetMainObjectSizeMin (uint32_t min);
  void SetMainObjectSizeMax (uint32_t max);
  uint32_t GetMainObjectSizeMean () const;

  // EMBEDDED OBJECT SIZE ATTRIBUTES SETTER AND GETTER METHODS

  void SetEmbeddedObjectSizeMean (uint32_t mean);
  void SetEmbeddedObjectSizeStdDev (uint32_t stdDev);
  void SetEmbeddedObjectSizeMin (uint32_t min);
  void SetEmbeddedObjectSizeMax (uint32_t max);
  uint32_t GetEmbeddedObjectSizeMean () const;

  // NUMBER OF EMBEDDED OBJECTS ATTRIBUTES SETTER AND GETTER METHODS

  void SetNumOfEmbeddedObjectsMean (double mean);
  void SetNumOfEmbeddedObjectsMax (uint32_t max);
  void SetNumOfEmbeddedObjectsParetoIndex (double paretoIndex);
  double GetNumOfEmbeddedObjectsMean () const;

  // READING TIME SETTER AND GETTER METHODS

  void SetReadingTimeMean (Time mean);
  Time GetReadingTimeMean () const;

  // PARSING TIME SETTER AND GETTER METHODS

  void SetParsingTimeMean (Time mean);
  Time GetParsingTimeMean () const;

private:

  // RANDOM NUMBER VARIABLES

  Ptr<UniformRandomVariable> m_httpVersionRng;
  Ptr<UniformRandomVariable> m_mtuSizeRng;
  Ptr<HttpLogNormalVariable> m_mainObjectSizeRng;
  Ptr<HttpLogNormalVariable> m_embeddedObjectSizeRng;
  Ptr<ParetoRandomVariable> m_numOfEmbeddedObjectsRng;
  Ptr<ExponentialRandomVariable> m_readingTimeRng;
  Ptr<ExponentialRandomVariable> m_parsingTimeRng;

}; // end of `class HttpVariables`


} // end of `namespace ns3`

#endif /* HTTP_VARIABLES_H */

