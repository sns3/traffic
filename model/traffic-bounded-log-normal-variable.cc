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

#include "traffic-bounded-log-normal-variable.h"
#include <ns3/log.h>
#include <ns3/double.h>
#include <cmath>


NS_LOG_COMPONENT_DEFINE ("TrafficBoundedLogNormalVariable");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TrafficBoundedLogNormalVariable);


TrafficBoundedLogNormalVariable::TrafficBoundedLogNormalVariable ()
  : LogNormalRandomVariable (),
    m_mean (0),
    m_stdDev (0)
{
  NS_LOG_FUNCTION (this);
}


TypeId
TrafficBoundedLogNormalVariable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TrafficBoundedLogNormalVariable")
    .SetParent<LogNormalRandomVariable> ()
    .AddConstructor<TrafficBoundedLogNormalVariable> ()
  ;
  return tid;
}


uint32_t
TrafficBoundedLogNormalVariable::GetBoundedInteger ()
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
TrafficBoundedLogNormalVariable::SetMin (uint32_t min)
{
  NS_LOG_FUNCTION (this << min);
  m_min = min;
}


uint32_t
TrafficBoundedLogNormalVariable::GetMin () const
{
  return m_min;
}


void
TrafficBoundedLogNormalVariable::SetMax (uint32_t max)
{
  NS_LOG_FUNCTION (this << max);
  m_max = max;
}


uint32_t
TrafficBoundedLogNormalVariable::GetMax () const
{
  return m_max;
}


void
TrafficBoundedLogNormalVariable::SetMean (uint32_t mean)
{
  NS_LOG_FUNCTION (this << mean);
  NS_ASSERT_MSG (mean > 0, "Mean must be greater than zero");
  m_mean = mean;
  RefreshBaseParameters ();
}


uint32_t
TrafficBoundedLogNormalVariable::GetMean () const
{
  return m_mean;
}


void
TrafficBoundedLogNormalVariable::SetStdDev (uint32_t stdDev)
{
  NS_LOG_FUNCTION (this << stdDev);
  m_stdDev = stdDev;
  RefreshBaseParameters ();
}


uint32_t
TrafficBoundedLogNormalVariable::GetStdDev () const
{
  return m_stdDev;
}


void
TrafficBoundedLogNormalVariable::RefreshBaseParameters ()
{
  NS_LOG_FUNCTION (this);

  const double a1 = pow (m_stdDev, 2);
  const double a2 = pow (m_mean, 2);
  const double a = log (1 + (a1 / a2));

  const double mu = log (m_mean) - (0.5 * a);
  const double sigma = sqrt (a);
  NS_LOG_INFO (this << " Mu= " << mu << " Sigma= " << sigma);

  // updating attributes of parent class
  SetAttribute ("Mu", DoubleValue (mu));
  SetAttribute ("Sigma", DoubleValue (sigma));
}


} // end of `namespace ns3`
