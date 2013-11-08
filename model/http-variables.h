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
class HttpBoundedLogNormalVariable : public LogNormalRandomVariable
{
public:
  /// Create a new instance of random variable using the default parameters.
  HttpBoundedLogNormalVariable ();

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
   * \return the standard deviation of the values value that the random
   *         distribution is producing
   */
  uint32_t GetStdDev () const;

private:
  /**
   * \brief Internal function to update the mu and sigma of the underlying
   *        LogNormal distribution, based on the provided mean and standard
   *        deviation.
   */
  void RefreshBaseParameters ();

  uint32_t m_min;
  uint32_t m_max;
  uint32_t m_mean;
  uint32_t m_stdDev;

}; // end of `class HttpBoundedLogNormalVariable`


/**
 * \brief Wrapper of ParetoRandomVariable for use in HTTP traffic model.
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
 *     Ptr<HttpBoundedParetoVariable> x = CreateObject<HttpBoundedParetoVariable> ();
 *     SetAttribute ("Bound", DoubleValue (100.0));
 *
 * \warning The scale parameter must not be greater than the Bound attribute.
 *          This is the case in the default configuration of the class.
 */
class HttpBoundedParetoVariable : public ParetoRandomVariable
{
public:
  /// Create a new instance of random variable using the default parameters.
  HttpBoundedParetoVariable ();

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
   * \return the scale parameter value of the underying Pareto random
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

  double m_scale;

}; // end of `class HttpBoundedParetoVariable`


/**
 * \brief Container of various random variables for HTTP traffic model.
 *
 * The default configuration of the underlying random distributions are
 * according to the default configured using the parameters defined in
 * IEEE 802.16 [1], NGMN [2], and 3GPP2 [3] specifications.
 *
 * The available random values to be retrieved are:
 * - HTTP version --- 1.0 (burst mode) or 1.1 (persistent mode);
 * - MTU size --- 536 bytes or 1460 bytes;
 * - request size --- constant 350 bytes;
 * - delay in generating a main object --- 0 second;
 * - main object size --- truncated LogNormal distribution with a mean of 10710
 *   bytes;
 * - delay in generating an embedded object --- 0 second;
 * - embedded object size (in bytes) --- truncated LogNormal distribution with
 *   a mean of 7758 bytes;
 * - number of embedded object per web page --- truncated Pareto distribution
 *   with a mean of 3.9 (after truncation);
 * - length of reading time (in seconds) --- unbounded exponential distribution
 *   with a mean of 30 seconds; and
 * - length of parsing time (in seconds) --- unbounded exponential distribution
 *   with a mean of 0.13 seconds.
 *
 * Most parameters of the random distributions are configurable via attributes
 * and methods of this class.
 *
 * References:
 * [1] IEEE 802.16m, "Evaluation Methodology Document (EMD)",
 *     IEEE 802.16m-08/004r5.
 * [2] NGMN Alliance, "NGMN Radio Access Performance Evaluation Methodology",
 *     v1.0.
 * [3] 3GPP2-TSGC5, "HTTP, FTP and TCP models for 1xEV-DV simulations".
 *
 */
class HttpVariables : public Object
{
public:
  /// Create a new instance with default configuration of random distributions.
  HttpVariables ();

  // Inherited from ObjectBase base class
  static TypeId GetTypeId ();

  // THE MORE USEFUL METHODS

  /**
   * \brief Get a random true/false value indicating whether an HTTP client
   *        shall use HTTP 1.0 (burst mode) or HTTP 1.1 (persistent mode).
   * \return true if HTTP 1.0, or false if HTTP 1.1
   *
   * By default, HTTP 1.0 and HTTP 1.1 have fifty-fifty chances.
   */
  bool IsBurstMode ();

  /**
   * \brief
   */
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

  // THE REST ARE THE NOT-SO-USEFUL METHODS

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

