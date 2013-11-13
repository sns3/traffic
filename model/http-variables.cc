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
#include <ns3/traffic-bounded-log-normal-variable.h>
#include <ns3/traffic-bounded-pareto-variable.h>


NS_LOG_COMPONENT_DEFINE ("HttpVariables");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (HttpVariables);


HttpVariables::HttpVariables ()
  : m_httpVersionRng                   (CreateObject<UniformRandomVariable> ()),
    m_mtuSizeRng                       (CreateObject<UniformRandomVariable> ()),
    m_requestSizeRng                   (CreateObject<ConstantRandomVariable> ()),
    m_mainObjectGenerationDelayRng     (CreateObject<ConstantRandomVariable> ()),
    m_mainObjectSizeRng                (CreateObject<TrafficBoundedLogNormalVariable> ()),
    m_embeddedObjectGenerationDelayRng (CreateObject<ConstantRandomVariable> ()),
    m_embeddedObjectSizeRng            (CreateObject<TrafficBoundedLogNormalVariable> ()),
    m_numOfEmbeddedObjectsRng          (CreateObject<TrafficBoundedParetoVariable> ()),
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
                   "The constant time needed by HTTP server "
                   "to generate a main object as a response.",
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
                   "The constant time needed by HTTP server "
                   "to generate an embedded object as a response.",
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
                   "The upper bound parameter of Pareto distribution for "
                   "the number of embedded objects per web page. The actual "
                   "maximum value is this value subtracted by the scale parameter",
                   UintegerValue (55),
                   MakeUintegerAccessor (&HttpVariables::SetNumOfEmbeddedObjectsMax,
                                         &HttpVariables::GetNumOfEmbeddedObjectsMax),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("NumOfEmbeddedObjectsShape",
                   "The shape parameter of Pareto distribution for "
                   "the number of embedded objects per web page.",
                   DoubleValue (1.1),
                   MakeDoubleAccessor (&HttpVariables::SetNumOfEmbeddedObjectsShape),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NumOfEmbeddedObjectsScale",
                   "The scale parameter of Pareto distribution for "
                   "the number of embedded objects per web page.",
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

} // end of `TypeId HttpVariables::GetTypeId ()`


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


Time
HttpVariables::GetMainObjectGenerationDelay ()
{
  return Seconds (m_mainObjectGenerationDelayRng->GetValue ());
}


uint32_t
HttpVariables::GetMainObjectSize ()
{
  return m_mainObjectSizeRng->GetBoundedInteger ();
}


Time
HttpVariables::GetEmbeddedObjectGenerationDelay ()
{
  return Seconds (m_embeddedObjectGenerationDelayRng->GetValue ());
}


uint32_t
HttpVariables::GetEmbeddedObjectSize ()
{
  return m_embeddedObjectSizeRng->GetBoundedInteger ();
}


uint32_t
HttpVariables::GetNumOfEmbeddedObjects ()
{
  return m_numOfEmbeddedObjectsRng->GetBoundedNormalizedInteger ();
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

