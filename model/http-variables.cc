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
#include <cmath>


NS_LOG_COMPONENT_DEFINE ("HttpVariables");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (HttpVariables);


HttpVariables::HttpVariables ()
  : m_httpVersionRng (CreateObject<UniformRandomVariable> ()),
    m_mtuSizeRng (CreateObject<UniformRandomVariable> ()),
    m_mainObjectSizeRng (CreateObject<LogNormalRandomVariable> ()),
    m_embeddedObjectSizeRng (CreateObject<LogNormalRandomVariable> ())
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
                   "-1 means \"allocate a stream automatically\". "
                   "Note that if -1 is set, Get will return -1, "
                   "so that it is not possible to know which value was was automatically allocated.",
                   IntegerValue (-1),
                   MakeIntegerAccessor (&HttpVariables::SetStream,
                                        &HttpVariables::GetStream),
                   MakeIntegerChecker<int64_t> ())

    // MAIN OBJECT SIZE
    .AddAttribute ("MainObjectSizeMean",
                   "The mean of main object sizes (in bytes).",
                   UintegerValue (10710),
                   MakeUintegerAccessor (&HttpVariables::SetMainObjectSizeMean,
                                         &HttpVariables::GetMainObjectSizeMean),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("MainObjectSizeSd",
                   "The standard deviation of main object sizes (in bytes).",
                   UintegerValue (25032),
                   MakeUintegerAccessor (&HttpVariables::SetMainObjectSizeStdDev,
                                         &HttpVariables::GetMainObjectSizeStdDev),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MainObjectSizeMin",
                   "The minimum value of main object sizes (in bytes).",
                   UintegerValue (100),
                   MakeUintegerAccessor (&HttpVariables::SetMainObjectSizeMin,
                                         &HttpVariables::GetMainObjectSizeMin),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MainObjectSizeMax",
                   "The maximum value of main object sizes (in bytes).",
                   UintegerValue (2000000), // 2 MB
                   MakeUintegerAccessor (&HttpVariables::SetMainObjectSizeMax,
                                         &HttpVariables::GetMainObjectSizeMax),
                   MakeUintegerChecker<uint32_t> ())

    // EMBEDDED OBJECT SIZE
    .AddAttribute ("EmbeddedObjectSizeMean",
                   "The mean of embedded object sizes (in bytes).",
                   UintegerValue (7758),
                   MakeUintegerAccessor (&HttpVariables::SetEmbeddedObjectSizeMean,
                                         &HttpVariables::GetEmbeddedObjectSizeMean),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("EmbeddedObjectSizeSd",
                   "The standard deviation of embedded object sizes (in bytes).",
                   UintegerValue (126168),
                   MakeUintegerAccessor (&HttpVariables::SetEmbeddedObjectSizeStdDev,
                                         &HttpVariables::GetEmbeddedObjectSizeStdDev),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("EmbeddedObjectSizeMin",
                   "The minimum value of embedded object sizes (in bytes).",
                   UintegerValue (50),
                   MakeUintegerAccessor (&HttpVariables::SetEmbeddedObjectSizeMin,
                                         &HttpVariables::GetEmbeddedObjectSizeMin),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("EmbeddedObjectSizeMax",
                   "The maximum value of embedded object sizes (in bytes).",
                   UintegerValue (2000000), // 2 MB
                   MakeUintegerAccessor (&HttpVariables::SetEmbeddedObjectSizeMax,
                                         &HttpVariables::GetEmbeddedObjectSizeMax),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}


bool
HttpVariables::IsPersistentMode ()
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
HttpVariables::GetMainObjectSize ()
{
  NS_LOG_FUNCTION (this);
  uint32_t ret;
  do
    {
      ret = m_mainObjectSizeRng->GetInteger ();
    }
  while ((ret < m_mainObjectSizeMin) || (ret > m_mainObjectSizeMax));
  return ret;
}


uint32_t
HttpVariables::GetEmbeddedObjectSize ()
{
  NS_LOG_FUNCTION (this);
  uint32_t ret;
  do
    {
      ret = m_embeddedObjectSizeRng->GetInteger ();
    }
  while ((ret < m_embeddedObjectSizeMin) || (ret > m_embeddedObjectSizeMax));
  return ret;
}


// ATTRIBUTES SETTER AND GETTER METHODS ///////////////////////////////////////


void
HttpVariables::SetStream (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_stream = stream;
  m_mainObjectSizeRng->SetStream (stream);
  m_embeddedObjectSizeRng->SetStream (stream);
}


int64_t
HttpVariables::GetStream () const
{
  return m_stream;
}


// MAIN OBJECT SIZE ATTRIBUTES SETTER AND GETTER METHODS //////////////////////


void
HttpVariables::SetMainObjectSizeMean (uint32_t mean)
{
  NS_LOG_FUNCTION (this << mean);
  if (mean == 0)
    {
      NS_FATAL_ERROR ("Mean shall not be zero");
    }
  m_mainObjectSizeMean = mean;
  RefreshMainObjectSizeDistribution ();
}


uint32_t
HttpVariables::GetMainObjectSizeMean () const
{
  return m_mainObjectSizeMean;
}


void
HttpVariables::SetMainObjectSizeStdDev (uint32_t stdDev)
{
  NS_LOG_FUNCTION (this << stdDev);
  m_mainObjectSizeStdDev = stdDev;
  RefreshMainObjectSizeDistribution ();
}


uint32_t
HttpVariables::GetMainObjectSizeStdDev () const
{
  return m_mainObjectSizeStdDev;
}


void
HttpVariables::SetMainObjectSizeMin (uint32_t min)
{
  NS_LOG_FUNCTION (this << min);
  m_mainObjectSizeMin = min;
}


uint32_t
HttpVariables::GetMainObjectSizeMin () const
{
  return m_mainObjectSizeMin;
}


void
HttpVariables::SetMainObjectSizeMax (uint32_t max)
{
  NS_LOG_FUNCTION (this << max);
  m_mainObjectSizeMax = max;
}


uint32_t
HttpVariables::GetMainObjectSizeMax () const
{
  return m_mainObjectSizeMax;
}


double
HttpVariables::GetMainObjectSizeMu () const
{
  NS_ASSERT (m_mainObjectSizeMean > 0);
  double eqA1 = pow (m_mainObjectSizeStdDev, 2);
  double eqA2 = pow (m_mainObjectSizeMean, 2);
  double eqA = log (1 + (eqA1 / eqA2));
  double eqB = log (m_mainObjectSizeMean);
  return eqB - (0.5 * eqA);
}


double
HttpVariables::GetMainObjectSizeSigma () const
{
  NS_ASSERT (m_mainObjectSizeMean > 0);
  double eqA1 = pow (m_mainObjectSizeStdDev, 2);
  double eqA2 = pow (m_mainObjectSizeMean, 2);
  double eqA = log (1 + (eqA1 / eqA2));
  return sqrt (eqA);
}


// EMBEDDED OBJECT SIZE ATTRIBUTES SETTER AND GETTER METHODS //////////////////


void
HttpVariables::SetEmbeddedObjectSizeMean (uint32_t mean)
{
  NS_LOG_FUNCTION (this << mean);
  if (mean == 0)
    {
      NS_FATAL_ERROR ("Mean shall not be zero");
    }
  m_embeddedObjectSizeMean = mean;
  RefreshEmbeddedObjectSizeDistribution ();
}


uint32_t
HttpVariables::GetEmbeddedObjectSizeMean () const
{
  return m_embeddedObjectSizeMean;
}


void
HttpVariables::SetEmbeddedObjectSizeStdDev (uint32_t stdDev)
{
  NS_LOG_FUNCTION (this << stdDev);
  m_embeddedObjectSizeStdDev = stdDev;
  RefreshEmbeddedObjectSizeDistribution ();
}


uint32_t
HttpVariables::GetEmbeddedObjectSizeStdDev () const
{
  return m_embeddedObjectSizeStdDev;
}


void
HttpVariables::SetEmbeddedObjectSizeMin (uint32_t min)
{
  NS_LOG_FUNCTION (this << min);
  m_embeddedObjectSizeMin = min;
}


uint32_t
HttpVariables::GetEmbeddedObjectSizeMin () const
{
  return m_embeddedObjectSizeMin;
}


void
HttpVariables::SetEmbeddedObjectSizeMax (uint32_t max)
{
  NS_LOG_FUNCTION (this << max);
  m_embeddedObjectSizeMax = max;
}


uint32_t
HttpVariables::GetEmbeddedObjectSizeMax () const
{
  return m_embeddedObjectSizeMax;
}


double
HttpVariables::GetEmbeddedObjectSizeMu () const
{
  NS_ASSERT (m_embeddedObjectSizeMean > 0);
  double eqA1 = pow (m_embeddedObjectSizeStdDev, 2);
  double eqA2 = pow (m_embeddedObjectSizeMean, 2);
  double eqA = log (1 + (eqA1 / eqA2));
  double eqB = log (m_embeddedObjectSizeMean);
  return eqB - (0.5 * eqA);
}


double
HttpVariables::GetEmbeddedObjectSizeSigma () const
{
  NS_ASSERT (m_embeddedObjectSizeMean > 0);
  double eqA1 = pow (m_embeddedObjectSizeStdDev, 2);
  double eqA2 = pow (m_embeddedObjectSizeMean, 2);
  double eqA = log (1 + (eqA1 / eqA2));
  return sqrt (eqA);
}


// NUMBER OF EMBEDDED OBJECTS ATTRIBUTES SETTER AND GETTER METHODS ////////////


void
HttpVariables::SetNumEmbeddedObjectsMean (uint32_t mean)
{
  NS_LOG_FUNCTION (this << mean);
  if (mean == 0)
    {
      NS_FATAL_ERROR ("Mean shall not be zero");
    }
  m_numEmbeddedObjectsMean = mean;
  //RefreshNumEmbeddedObjectsDistribution ();
}


uint32_t
HttpVariables::GetNumEmbeddedObjectsMean () const
{
  return m_numEmbeddedObjectsMean;
}


// INTERNAL METHODS ///////////////////////////////////////////////////////////


void
HttpVariables::RefreshMainObjectSizeDistribution ()
{
  NS_LOG_FUNCTION (this);
  double mu = GetMainObjectSizeMu ();
  double sigma = GetMainObjectSizeSigma ();
  NS_LOG_INFO (this << " MainObjectSize Mu= " << mu << " Sigma= " << sigma);
  m_mainObjectSizeRng->SetAttribute ("Mu", DoubleValue (mu));
  m_mainObjectSizeRng->SetAttribute ("Sigma", DoubleValue (sigma));
}


void
HttpVariables::RefreshEmbeddedObjectSizeDistribution ()
{
  NS_LOG_FUNCTION (this);
  double mu = GetEmbeddedObjectSizeMu ();
  double sigma = GetEmbeddedObjectSizeSigma ();
  NS_LOG_INFO (this << " EmbeddedObjectSize Mu= " << mu << " Sigma= " << sigma);
  m_embeddedObjectSizeRng->SetAttribute ("Mu", DoubleValue (mu));
  m_embeddedObjectSizeRng->SetAttribute ("Sigma", DoubleValue (sigma));
}


} // end of `namespace ns3`

