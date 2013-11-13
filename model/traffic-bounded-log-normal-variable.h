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

#ifndef TRAFFIC_BOUNDED_LOG_NORMAL_VARIABLE_H
#define TRAFFIC_BOUNDED_LOG_NORMAL_VARIABLE_H

#include <ns3/random-variable-stream.h>


namespace ns3 {


/**
 * \ingroup traffic
 * \brief Wrapper of LogNormalRandomVariable for use in traffic models.
 *
 * Provides configurability using mean and standard deviation instead of the
 * regular mu (\f$ \mu \f$) and sigma (\f$ \sigma \f$). In addition, the
 * GetBoundedInteger() returns values which are truncated within a given range
 * of [min..max].
 *
 * \warning Random numbers produced by calling the base class methods GetValue()
 *          and GetInteger() are not truncated in this way.
 *
 * The mean, standard deviation, min, and max are configurable by calling the
 * corresponding class methods SetMean(), SetStdDev(), SetMin(), and SetMax(),
 * respectively.
 */
class TrafficBoundedLogNormalVariable : public LogNormalRandomVariable
{
public:
  /// Create a new instance of random variable using the default parameters.
  TrafficBoundedLogNormalVariable ();

  // Inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \brief Return a random integer from the underlying LogNormal distribution,
   *        bounded to the configured range.
   * \return a random unsigned integer, guaranteed to be inside the range
   *         [min..max]
   */
  uint32_t GetBoundedInteger ();

  /**
   * \brief Set the lower bound of the LogNormal random distribution.
   * \param min the minimum value that the random distribution can produce
   *
   * \warning Upper bound must be greater than lower bound.
   */
  void SetMin (uint32_t min);

  /**
   * \return the minimum value that the random distribution can produce
   */
  uint32_t GetMin () const;

  /**
   * \brief Set the upper bound of the LogNormal random distribution.
   * \param max the maximum value that the random distribution can produce
   *
   * \warning Upper bound must be greater than lower bound.
   */
  void SetMax (uint32_t max);

  /**
   * \return the maximum value that the random distribution can produce
   */
  uint32_t GetMax () const;

  /**
   * \brief Set the mean of the LogNormal random distribution.
   * \param mean the mean of the values that the random distribution will
   *             produce, must be greater than zero
   *
   * \warning Mean value must be greater than zero, otherwise an error would be
   *          raised.
   */
  void SetMean (uint32_t mean);

  /**
   * \return the mean of the values value that the random distribution is
   *         producing
   */
  uint32_t GetMean () const;

  /**
   * \brief Set the standard deviation of the LogNormal random distribution.
   * \param stdDev the standard deviation of the values that the random
   *               distribution will produce
   */
  void SetStdDev (uint32_t stdDev);

  /**
   * \return the standard deviation of the values that the random distribution
   *         is producing
   */
  uint32_t GetStdDev () const;

private:
  /**
   * \brief Internal function to update the mu and sigma of the underlying
   *        LogNormal distribution, based on the provided mean and standard
   *        deviation.
   */
  void RefreshBaseParameters ();

  uint32_t m_min;    ///< The minimum value that the random distribution can produce.
  uint32_t m_max;    ///< The maximum value that the random distribution can produce.
  uint32_t m_mean;   ///< The mean value that the random distribution can produce.
  uint32_t m_stdDev; ///< The standard deviation of the values that the random distribution is producing.

}; // end of `class TrafficBoundedLogNormalVariable`


} // end of `namespace ns3`


#endif /* TRAFFIC_BOUNDED_LOG_NORMAL_VARIABLE_H */
