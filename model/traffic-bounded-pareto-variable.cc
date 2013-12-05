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

#include "traffic-bounded-pareto-variable.h"
#include <ns3/log.h>
#include <ns3/double.h>
#include <cmath>


NS_LOG_COMPONENT_DEFINE ("TrafficBoundedParetoVariable");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TrafficBoundedParetoVariable);


TrafficBoundedParetoVariable::TrafficBoundedParetoVariable ()
{
  NS_LOG_FUNCTION (this);
}


TypeId
TrafficBoundedParetoVariable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TrafficBoundedParetoVariable")
    .SetParent<ParetoRandomVariable> ()
    .AddConstructor<TrafficBoundedParetoVariable> ()
  ;
  return tid;
}


uint32_t
TrafficBoundedParetoVariable::GetBoundedInteger ()
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

  return ret;
}


uint32_t
TrafficBoundedParetoVariable::GetBoundedNormalizedInteger ()
{
  NS_LOG_FUNCTION (this);

  uint32_t x = GetBoundedInteger ();
  NS_ASSERT (x >= m_scale);
  return (x - m_scale);
}


void
TrafficBoundedParetoVariable::SetScale (double scale)
{
  NS_LOG_FUNCTION (this << scale);

  NS_ASSERT_MSG (scale > 0.0, "Scale parameter must be greater than zero");
  m_scale = scale;
  RefreshBaseParameters ();
}


double
TrafficBoundedParetoVariable::GetScale () const
{
  return m_scale;
}


void
TrafficBoundedParetoVariable::RefreshBaseParameters ()
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


} // end of `namespace ns3`
