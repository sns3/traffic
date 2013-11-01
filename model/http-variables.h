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
class HttpBoundedLogNormalVariable : public LogNormalRandomVariable
{
public:
  HttpBoundedLogNormalVariable ();
  static TypeId GetTypeId ();

  uint32_t GetBoundedInteger ();

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

}; // end of `class HttpBoundedLogNormalVariable`


/**
 * \brief Wrapper of ParetoRandomVariable for use in HTTP traffic model.
 *
 * Provides configurability using the scale parameter. In addition, returned
 * values are truncated within the range [scale, bound], and then normalized
 * (i.e., substracted by the scale parameter).
 */
class HttpBoundedParetoVariable : public ParetoRandomVariable
{
public:
  HttpBoundedParetoVariable ();
  static TypeId GetTypeId ();

  virtual uint32_t GetBoundedInteger ();

  void SetScale (double scale);
  double GetScale () const;

private:
  void RefreshBaseParameters ();

  double m_scale;

}; // end of `class HttpBoundedParetoVariable`


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
  uint32_t GetRequestSize ();
  uint32_t GetRequestSizeKbytes ();
  Time GetMainObjectGenerationDelay ();
  double GetMainObjectGenerationDelaySeconds ();
  uint32_t GetMainObjectSize ();
  uint32_t GetMainObjectSizeKbytes ();
  Time GetEmbeddedObjectGenerationDelay ();
  double GetEmbeddedObjectGenerationDelaySeconds ();
  uint32_t GetEmbeddedObjectSize ();
  uint32_t GetEmbeddedObjectSizeKbytes ();
  uint32_t GetNumOfEmbeddedObjects ();
  Time GetReadingTime ();
  double GetReadingTimeSeconds ();
  Time GetParsingTime ();
  double GetParsingTimeSeconds ();

  void SetStream (int64_t stream);

  // REQUEST SIZE SETTER METHODS

  void SetRequestSize (uint32_t constant);

  // MAIN OBJECT GENERATION DELAY SETTER METHODS

  void SetMainObjectGenerationDelay (Time constant);

  // MAIN OBJECT SIZE ATTRIBUTES SETTER METHODS

  void SetMainObjectSizeMean (uint32_t mean);
  void SetMainObjectSizeStdDev (uint32_t stdDev);
  void SetMainObjectSizeMin (uint32_t min);
  void SetMainObjectSizeMax (uint32_t max);
  uint32_t GetMainObjectSizeMean () const;

  // EMBEDDED OBJECT GENERATION DELAY SETTER METHODS

  void SetEmbeddedObjectGenerationDelay (Time constant);

  // EMBEDDED OBJECT SIZE ATTRIBUTES SETTER METHODS

  void SetEmbeddedObjectSizeMean (uint32_t mean);
  void SetEmbeddedObjectSizeStdDev (uint32_t stdDev);
  void SetEmbeddedObjectSizeMin (uint32_t min);
  void SetEmbeddedObjectSizeMax (uint32_t max);
  uint32_t GetEmbeddedObjectSizeMean () const;

  // NUMBER OF EMBEDDED OBJECTS ATTRIBUTES SETTER METHODS

  void SetNumOfEmbeddedObjectsMax (uint32_t max);
  void SetNumOfEmbeddedObjectsShape (double shape);
  void SetNumOfEmbeddedObjectsScale (double scale);
  double GetNumOfEmbeddedObjectsMean () const;
  uint32_t GetNumOfEmbeddedObjectsMax () const;

  // READING TIME SETTER METHODS

  void SetReadingTimeMean (Time mean);
  Time GetReadingTimeMean () const;

  // PARSING TIME SETTER METHODS

  void SetParsingTimeMean (Time mean);
  Time GetParsingTimeMean () const;

private:

  // RANDOM NUMBER VARIABLES

  Ptr<UniformRandomVariable>         m_httpVersionRng;
  Ptr<UniformRandomVariable>         m_mtuSizeRng;
  Ptr<ConstantRandomVariable>        m_requestSizeRng;
  Ptr<ConstantRandomVariable>        m_mainObjectGenerationDelayRng;
  Ptr<HttpBoundedLogNormalVariable>  m_mainObjectSizeRng;
  Ptr<ConstantRandomVariable>        m_embeddedObjectGenerationDelayRng;
  Ptr<HttpBoundedLogNormalVariable>  m_embeddedObjectSizeRng;
  Ptr<HttpBoundedParetoVariable>     m_numOfEmbeddedObjectsRng;
  Ptr<ExponentialRandomVariable>     m_readingTimeRng;
  Ptr<ExponentialRandomVariable>     m_parsingTimeRng;

}; // end of `class HttpVariables`


} // end of `namespace ns3`

#endif /* HTTP_VARIABLES_H */

