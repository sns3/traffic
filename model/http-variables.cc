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

#include "http-variables.h"
#include <ns3/log.h>
#include <ns3/integer.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <ns3/rng-stream.h>
#include <cmath>


NS_LOG_COMPONENT_DEFINE ("HttpVariables");

namespace ns3 {


// LOG NORMAL WRAPPER CLASS ///////////////////////////////////////////////////


NS_OBJECT_ENSURE_REGISTERED (HttpBoundedLogNormalVariable);


HttpBoundedLogNormalVariable::HttpBoundedLogNormalVariable ()
  : LogNormalRandomVariable ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
HttpBoundedLogNormalVariable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::HttpBoundedLogNormalVariable")
    .SetParent<LogNormalRandomVariable> ()
    .AddConstructor<HttpBoundedLogNormalVariable> ()
  ;
  return tid;
}


uint32_t
HttpBoundedLogNormalVariable::GetBoundedInteger ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_min <= m_max, "Lower bound is greater than upper bound");

  uint32_t ret;
  do
    {
      ret = GetInteger (); // invoking a function of parent class
    }
  while ((ret < m_min) || (ret > m_max));
  return ret;
}


void
HttpBoundedLogNormalVariable::SetMin (uint32_t min)
{
  NS_LOG_FUNCTION (this << min);
  m_min = min;
}


uint32_t
HttpBoundedLogNormalVariable::GetMin () const
{
  return m_min;
}


void
HttpBoundedLogNormalVariable::SetMax (uint32_t max)
{
  NS_LOG_FUNCTION (this << max);
  m_max = max;
}


uint32_t
HttpBoundedLogNormalVariable::GetMax () const
{
  return m_max;
}


void
HttpBoundedLogNormalVariable::SetMean (uint32_t mean)
{
  NS_LOG_FUNCTION (this << mean);
  NS_ASSERT_MSG (mean > 0, "Mean must be greater than zero");
  m_mean = mean;
  RefreshBaseParameters ();
}


uint32_t
HttpBoundedLogNormalVariable::GetMean () const
{
  return m_mean;
}


void
HttpBoundedLogNormalVariable::SetStdDev (uint32_t stdDev)
{
  NS_LOG_FUNCTION (this << stdDev);
  m_stdDev = stdDev;
  RefreshBaseParameters ();
}


uint32_t
HttpBoundedLogNormalVariable::GetStdDev () const
{
  return m_stdDev;
}


void
HttpBoundedLogNormalVariable::RefreshBaseParameters ()
{
  NS_LOG_FUNCTION (this);

  double a1 = pow (m_stdDev, 2);
  double a2 = pow (m_mean, 2);
  double a = log (1 + (a1 / a2));

  double mu = log (m_mean) - (0.5 * a);
  double sigma = sqrt (a);
  NS_LOG_INFO (this << " Mu= " << mu << " Sigma= " << sigma);

  // updating attributes of parent class
  SetAttribute ("Mu", DoubleValue (mu));
  SetAttribute ("Sigma", DoubleValue (sigma));
}


// PARETO WRAPPER CLASS ///////////////////////////////////////////////////////


NS_OBJECT_ENSURE_REGISTERED (HttpBoundedParetoVariable);


HttpBoundedParetoVariable::HttpBoundedParetoVariable ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
HttpBoundedParetoVariable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::HttpBoundedParetoVariable")
    .SetParent<ParetoRandomVariable> ()
    .AddConstructor<HttpBoundedParetoVariable> ()
  ;
  return tid;
}


uint32_t
HttpBoundedParetoVariable::GetBoundedInteger ()
{
  NS_LOG_FUNCTION (this);

  double upperBound = GetBound (); // extracting parameter value from parent class
  NS_ASSERT_MSG (m_scale <= upperBound,
                 "Bound attribute in a bounded Pareto distribution"
                 << " must not be less than the scale parameter");

  uint32_t ret;
  do
    {
      ret = GetInteger (); // invoking a function of parent class
    }
  while ((ret < m_scale) || (ret > upperBound));

  return ret - m_scale;
}


void
HttpBoundedParetoVariable::SetScale (double scale)
{
  NS_LOG_FUNCTION (this << scale);

  NS_ASSERT_MSG (scale > 0.0, "Scale parameter must be greater than zero");
  m_scale = scale;
  RefreshBaseParameters ();
}


double
HttpBoundedParetoVariable::GetScale () const
{
  return m_scale;
}


void
HttpBoundedParetoVariable::RefreshBaseParameters ()
{
  NS_LOG_FUNCTION (this);

  double shape = GetShape (); // extracting parameter value from parent class
  if (fabs (shape - 1.0) < 0.000001)
    {
      NS_FATAL_ERROR ("Shape parameter of a Pareto distribution must not equal to 1.0"
        << " (the current value is " << shape << ")");
    }

  double mean = (shape * m_scale) / (shape - 1.0);
  NS_LOG_INFO (this << " mean= " << mean);

  // updating attribute of parent class
  SetAttribute ("Mean", DoubleValue (mean));
}


// PRIMARY CLASS //////////////////////////////////////////////////////////////


NS_OBJECT_ENSURE_REGISTERED (HttpVariables);


HttpVariables::HttpVariables ()
  : m_httpVersionRng                   (CreateObject<UniformRandomVariable> ()),
    m_mtuSizeRng                       (CreateObject<UniformRandomVariable> ()),
    m_requestSizeRng                   (CreateObject<ConstantRandomVariable> ()),
    m_mainObjectGenerationDelayRng     (CreateObject<ConstantRandomVariable> ()),
    m_mainObjectSizeRng                (CreateObject<HttpBoundedLogNormalVariable> ()),
    m_embeddedObjectGenerationDelayRng (CreateObject<ConstantRandomVariable> ()),
    m_embeddedObjectSizeRng            (CreateObject<HttpBoundedLogNormalVariable> ()),
    m_numOfEmbeddedObjectsRng          (CreateObject<HttpBoundedParetoVariable> ()),
    m_readingTimeRng                   (CreateObject<ExponentialRandomVariable> ()),
    m_parsingTimeRng                   (CreateObject<ExponentialRandomVariable> ())
{
  NS_LOG_FUNCTION (this);
}


TypeId
HttpVariables::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::HttpVariables")
    .SetParent<Object> ()
    .AddConstructor<HttpVariables> ()
    .AddAttribute ("Stream",
                   "The stream number for the underlying random number generators stream. "
                   "-1 means \"allocate a stream automatically\".",
                   IntegerValue (-1),
                   MakeIntegerAccessor (&HttpVariables::SetStream),
                   MakeIntegerChecker<int64_t> ())

    // REQUEST SIZE
    .AddAttribute ("RequestSize",
                   "The constant size of HTTP request packet (in bytes).",
                   UintegerValue (350),
                   MakeUintegerAccessor (&HttpVariables::SetRequestSize),
                   MakeUintegerChecker<uint32_t> ())

    // MAIN OBJECT GENERATION DELAY
    .AddAttribute ("MainObjectGenerationDelay",
                   "The constant time needed by HTTP server to generate a main object as a response.",
                   TimeValue (MilliSeconds (0)),
                   MakeTimeAccessor (&HttpVariables::SetMainObjectGenerationDelay),
                   MakeTimeChecker ())

    // MAIN OBJECT SIZE
    .AddAttribute ("MainObjectSizeMean",
                   "The mean of main object sizes (in bytes).",
                   UintegerValue (10710),
                   MakeUintegerAccessor (&HttpVariables::SetMainObjectSizeMean,
                                         &HttpVariables::GetMainObjectSizeMean),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MainObjectSizeStdDev",
                   "The standard deviation of main object sizes (in bytes).",
                   UintegerValue (25032),
                   MakeUintegerAccessor (&HttpVariables::SetMainObjectSizeStdDev),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MainObjectSizeMin",
                   "The minimum value of main object sizes (in bytes).",
                   UintegerValue (100),
                   MakeUintegerAccessor (&HttpVariables::SetMainObjectSizeMin),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MainObjectSizeMax",
                   "The maximum value of main object sizes (in bytes).",
                   UintegerValue (2000000), // 2 MB
                   MakeUintegerAccessor (&HttpVariables::SetMainObjectSizeMax),
                   MakeUintegerChecker<uint32_t> ())

    // EMBEDDED OBJECT GENERATION DELAY
    .AddAttribute ("EmbeddedObjectGenerationDelay",
                   "The constant time needed by HTTP server to generate an embedded object as a response.",
                   TimeValue (MilliSeconds (0)),
                   MakeTimeAccessor (&HttpVariables::SetEmbeddedObjectGenerationDelay),
                   MakeTimeChecker ())

    // EMBEDDED OBJECT SIZE
    .AddAttribute ("EmbeddedObjectSizeMean",
                   "The mean of embedded object sizes (in bytes).",
                   UintegerValue (7758),
                   MakeUintegerAccessor (&HttpVariables::SetEmbeddedObjectSizeMean,
                                         &HttpVariables::GetEmbeddedObjectSizeMean),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("EmbeddedObjectSizeStdDev",
                   "The standard deviation of embedded object sizes (in bytes).",
                   UintegerValue (126168),
                   MakeUintegerAccessor (&HttpVariables::SetEmbeddedObjectSizeStdDev),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("EmbeddedObjectSizeMin",
                   "The minimum value of embedded object sizes (in bytes).",
                   UintegerValue (50),
                   MakeUintegerAccessor (&HttpVariables::SetEmbeddedObjectSizeMin),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("EmbeddedObjectSizeMax",
                   "The maximum value of embedded object sizes (in bytes).",
                   UintegerValue (2000000), // 2 MB
                   MakeUintegerAccessor (&HttpVariables::SetEmbeddedObjectSizeMax),
                   MakeUintegerChecker<uint32_t> ())

    // NUMBER OF EMBEDDED OBJECTS PER PAGE
    .AddAttribute ("NumOfEmbeddedObjectsMax",
                   "The maximum value of number of embedded objects per web page.",
                   UintegerValue (55),
                   MakeUintegerAccessor (&HttpVariables::SetNumOfEmbeddedObjectsMax,
                                         &HttpVariables::GetNumOfEmbeddedObjectsMax),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("NumOfEmbeddedObjectsShape",
                   "The shape parameter of Pareto distribution for number of embedded objects per web page.",
                   DoubleValue (1.1),
                   MakeDoubleAccessor (&HttpVariables::SetNumOfEmbeddedObjectsShape),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NumOfEmbeddedObjectsScale",
                   "The scale parameter of Pareto distribution for number of embedded objects per web page.",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&HttpVariables::SetNumOfEmbeddedObjectsScale),
                   MakeDoubleChecker<double> ())

    // READING TIME
    .AddAttribute ("ReadingTimeMean",
                   "The mean of reading time.",
                   TimeValue (Seconds (30)),
                   MakeTimeAccessor (&HttpVariables::SetReadingTimeMean,
                                     &HttpVariables::GetReadingTimeMean),
                   MakeTimeChecker ())

    // PARSING TIME
    .AddAttribute ("ParsingTimeMean",
                   "The mean of parsing time.",
                   TimeValue (MilliSeconds (130)),
                   MakeTimeAccessor (&HttpVariables::SetParsingTimeMean,
                                     &HttpVariables::GetParsingTimeMean),
                   MakeTimeChecker ())
  ;
  return tid;
}


bool
HttpVariables::IsBurstMode ()
{
  double r = m_httpVersionRng->GetValue ();
  NS_ASSERT (r >= 0.0);
  NS_ASSERT (r < 1.0);
  return (r < 0.5);
}


uint32_t
HttpVariables::GetMtuSize ()
{
  double r = m_mtuSizeRng->GetValue ();
  NS_ASSERT (r >= 0.0);
  NS_ASSERT (r < 1.0);
  if (r < 0.76)
    {
      return 1460; // 1500 bytes if including TCP header
    }
  else
    {
      return 536; // 576 bytes if including TCP header
    }
}


uint32_t
HttpVariables::GetRequestSize ()
{
  return m_requestSizeRng->GetInteger ();
}


uint32_t
HttpVariables::GetRequestSizeKbytes ()
{
  return m_requestSizeRng->GetInteger () / 1000;
}


Time
HttpVariables::GetMainObjectGenerationDelay ()
{
  return Seconds (m_mainObjectGenerationDelayRng->GetValue ());
}


double
HttpVariables::GetMainObjectGenerationDelaySeconds ()
{
  return m_mainObjectGenerationDelayRng->GetValue ();
}


uint32_t
HttpVariables::GetMainObjectSize ()
{
  return m_mainObjectSizeRng->GetBoundedInteger ();
}


uint32_t
HttpVariables::GetMainObjectSizeKbytes ()
{
  return m_mainObjectSizeRng->GetBoundedInteger () / 1000;
}


Time
HttpVariables::GetEmbeddedObjectGenerationDelay ()
{
  return Seconds (m_embeddedObjectGenerationDelayRng->GetValue ());
}


double
HttpVariables::GetEmbeddedObjectGenerationDelaySeconds ()
{
  return m_embeddedObjectGenerationDelayRng->GetValue ();
}


uint32_t
HttpVariables::GetEmbeddedObjectSize ()
{
  return m_embeddedObjectSizeRng->GetBoundedInteger ();
}


uint32_t
HttpVariables::GetEmbeddedObjectSizeKbytes ()
{
  return m_embeddedObjectSizeRng->GetBoundedInteger () / 1000;
}


uint32_t
HttpVariables::GetNumOfEmbeddedObjects ()
{
  return m_numOfEmbeddedObjectsRng->GetBoundedInteger ();
}


Time
HttpVariables::GetReadingTime ()
{
  return Seconds (m_readingTimeRng->GetValue ());
}


double
HttpVariables::GetReadingTimeSeconds ()
{
  return m_readingTimeRng->GetValue ();
}


Time
HttpVariables::GetParsingTime ()
{
  return Seconds (m_parsingTimeRng->GetValue ());
}


double
HttpVariables::GetParsingTimeSeconds ()
{
  return m_parsingTimeRng->GetValue ();
}


void
HttpVariables::SetStream (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_httpVersionRng->SetStream (stream);
  m_mtuSizeRng->SetStream (stream);
  m_requestSizeRng->SetStream (stream);
  m_mainObjectGenerationDelayRng->SetStream (stream);
  m_mainObjectSizeRng->SetStream (stream);
  m_embeddedObjectGenerationDelayRng->SetStream (stream);
  m_embeddedObjectSizeRng->SetStream (stream);
  m_numOfEmbeddedObjectsRng->SetStream (stream);
  m_readingTimeRng->SetStream (stream);
  m_parsingTimeRng->SetStream (stream);
}


// REQUEST SIZE SETTER METHODS ////////////////////////////////////////////////


void
HttpVariables::SetRequestSize (uint32_t constant)
{
  NS_LOG_FUNCTION (this << constant);
  m_requestSizeRng->SetAttribute ("Constant",
                                  DoubleValue (static_cast<double> (constant)));
}


// MAIN OBJECT GENERATION DELAY SETTER METHODS ////////////////////////////////


void
HttpVariables::SetMainObjectGenerationDelay (Time constant)
{
  NS_LOG_FUNCTION (this << constant.GetSeconds ());
  m_mainObjectGenerationDelayRng->SetAttribute ("Constant",
                                                DoubleValue (constant.GetSeconds ()));
}


// MAIN OBJECT SIZE ATTRIBUTES SETTER METHODS /////////////////////////////////


void
HttpVariables::SetMainObjectSizeMean (uint32_t mean)
{
  NS_LOG_FUNCTION (this << mean);
  m_mainObjectSizeRng->SetMean (mean);
}


void
HttpVariables::SetMainObjectSizeStdDev (uint32_t stdDev)
{
  NS_LOG_FUNCTION (this << stdDev);
  m_mainObjectSizeRng->SetStdDev (stdDev);
}


void
HttpVariables::SetMainObjectSizeMin (uint32_t min)
{
  NS_LOG_FUNCTION (this << min);
  m_mainObjectSizeRng->SetMin (min);
}


void
HttpVariables::SetMainObjectSizeMax (uint32_t max)
{
  NS_LOG_FUNCTION (this << max);
  m_mainObjectSizeRng->SetMax (max);
}


uint32_t
HttpVariables::GetMainObjectSizeMean () const
{
  return m_mainObjectSizeRng->GetMean ();
}


// EMBEDDED OBJECT GENERATION DELAY SETTER METHODS ////////////////////////////


void
HttpVariables::SetEmbeddedObjectGenerationDelay (Time constant)
{
  NS_LOG_FUNCTION (this << constant.GetSeconds ());
  m_embeddedObjectGenerationDelayRng->SetAttribute ("Constant",
                                                    DoubleValue (constant.GetSeconds ()));
}


// EMBEDDED OBJECT SIZE ATTRIBUTES SETTER AND GETTER METHODS //////////////////


void
HttpVariables::SetEmbeddedObjectSizeMean (uint32_t mean)
{
  NS_LOG_FUNCTION (this << mean);
  m_embeddedObjectSizeRng->SetMean (mean);
}


void
HttpVariables::SetEmbeddedObjectSizeStdDev (uint32_t stdDev)
{
  NS_LOG_FUNCTION (this << stdDev);
  m_embeddedObjectSizeRng->SetStdDev (stdDev);
}


void
HttpVariables::SetEmbeddedObjectSizeMin (uint32_t min)
{
  NS_LOG_FUNCTION (this << min);
  m_embeddedObjectSizeRng->SetMin (min);
}


void
HttpVariables::SetEmbeddedObjectSizeMax (uint32_t max)
{
  NS_LOG_FUNCTION (this << max);
  m_embeddedObjectSizeRng->SetMax (max);
}


uint32_t
HttpVariables::GetEmbeddedObjectSizeMean () const
{
  return m_embeddedObjectSizeRng->GetMean ();
}


// NUMBER OF EMBEDDED OBJECTS PER PAGE ATTRIBUTES SETTER AND GETTER METHODS ///


void
HttpVariables::SetNumOfEmbeddedObjectsMax (uint32_t max)
{
  NS_LOG_FUNCTION (this << max);
  m_numOfEmbeddedObjectsRng->SetAttribute ("Bound",
                                           DoubleValue (static_cast<double> (max)));
}


void
HttpVariables::SetNumOfEmbeddedObjectsShape (double shape)
{
  NS_LOG_FUNCTION (this << shape);
  m_numOfEmbeddedObjectsRng->SetAttribute ("Shape", DoubleValue (shape));
}


void
HttpVariables::SetNumOfEmbeddedObjectsScale (double scale)
{
  NS_LOG_FUNCTION (this << scale);
  m_numOfEmbeddedObjectsRng->SetScale (scale);
}


double
HttpVariables::GetNumOfEmbeddedObjectsMean () const
{
  // extract value from parent class
  return m_numOfEmbeddedObjectsRng->GetMean ();
}


uint32_t
HttpVariables::GetNumOfEmbeddedObjectsMax () const
{
  // extract value from parent class
  return static_cast<uint32_t> (m_numOfEmbeddedObjectsRng->GetBound ());
}



// READING TIME ATTRIBUTES SETTER AND GETTER METHODS //////////////////////////


void
HttpVariables::SetReadingTimeMean (Time mean)
{
  NS_LOG_FUNCTION (this << mean.GetSeconds ());
  m_readingTimeRng->SetAttribute ("Mean", DoubleValue (mean.GetSeconds ()));
}


Time
HttpVariables::GetReadingTimeMean () const
{
  return Seconds (m_readingTimeRng->GetMean ());
}


// PARSING TIME ATTRIBUTES SETTER AND GETTER METHODS //////////////////////////


void
HttpVariables::SetParsingTimeMean (Time mean)
{
  NS_LOG_FUNCTION (this << mean.GetSeconds ());
  m_parsingTimeRng->SetAttribute ("Mean", DoubleValue (mean.GetSeconds ()));
}


Time
HttpVariables::GetParsingTimeMean () const
{
  return Seconds (m_parsingTimeRng->GetMean ());
}


} // end of `namespace ns3`

