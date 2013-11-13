/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Budiarto Herman
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

#ifndef TRAFFIC_BOUNDED_PARETO_VARIABLE_H
#define TRAFFIC_BOUNDED_PARETO_VARIABLE_H

#include <ns3/random-variable-stream.h>


namespace ns3 {


/**
 * \ingroup traffic
 * \brief Wrapper of ParetoRandomVariable for use in traffic models.
 *
 * Provides configurability using the scale parameter instead of the regular
 * mean parameter. In addition, the GetBoundedInteger() returns values which are
 * normalized (i.e., substracted by the scale parameter) and truncated within a
 * given range of [0..(max - scale)].
 *
 * \warning Random numbers produced by calling the base class methods GetValue()
 *          and GetInteger() are not be truncated in this way.
 *
 * The scale parameter is configurable by calling SetScale() method. The max
 * parameter is, however, an attribute of the parent class, so it should be set
 * as the following example:
 *
 *     Ptr<TrafficBoundedParetoVariable> x = CreateObject<TrafficBoundedParetoVariable> ();
 *     SetAttribute ("Bound", DoubleValue (100.0));
 *
 * \warning The scale parameter must not be greater than the Bound attribute.
 *          This is the case in the default configuration of the class.
 */
class TrafficBoundedParetoVariable : public ParetoRandomVariable
{
public:
  /// Create a new instance of random variable using the default parameters.
  TrafficBoundedParetoVariable ();

  // Inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \brief Return a random integer from the underlying Pareto distribution,
   *        bounded to the configured range.
   * \return a random unsigned integer, guaranteed to be inside the range
   *         [0..(max - scale)]
   */
  virtual uint32_t GetBoundedInteger ();

  /**
   * \param scale the scale parameter of the underlying Pareto random
   *              distribution, must be greater than zero
   *
   * \warning The scale parameter value must be greater than zero, otherwise an
   *          error would be raised.
   */
  void SetScale (double scale);

  /**
   * \return the scale parameter value of the underlying Pareto random
   *         distribution
   */
  double GetScale () const;

private:
  /**
   * \brief Internal function to update the mu and sigma of the underlying
   *        LogNormal distribution, based on the provided mean and standard
   *        deviation.
   */
  void RefreshBaseParameters ();

  double m_scale; ///< The scale parameter of the underlying Pareto random distribution.

}; // end of `class TrafficBoundedParetoVariable`


} // end of `namespace ns3`


#endif /* TRAFFIC_BOUNDED_PARETO_VARIABLE_H */
