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


class TrafficBoundedLogNormalVariable;
class TrafficBoundedParetoVariable;


/**
 * \ingroup traffic
 * \brief Container of various random variables for assisting the generation of
 *        interactive traffic pattern by the HTTP (web browsing) traffic model.
 *
 * The default configuration of the underlying random distributions are
 * according to IEEE 802.16 [1], NGMN [2], and 3GPP2 [3] specifications.
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
 *   with a mean of approximately 3.95 (after truncation);
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
 *     IEEE 802.16m-08/004r5, July 2008.
 * [2] NGMN Alliance, "NGMN Radio Access Performance Evaluation Methodology",
 *     v1.0, January 2008.
 * [3] 3GPP2-TSGC5, "HTTP, FTP and TCP models for 1xEV-DV simulations", 2001.
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
   * Both HTTP 1.0 and HTTP 1.1 have fifty-fifty chances.
   */
  bool IsBurstMode ();

  /**
   * \brief Get a random value of Maximum Transmission Unit (MTU) size in bytes.
   * \return MTU size in bytes
   *
   * The possible MTU sizes are 1460 bytes and 536 bytes with 76% and 24%
   * chances, respectively. The selected value is typically used by the sockets
   * of HTTP servers to send the response packets (both main objects and
   * embedded objects) to the requesting HTTP clients.
   */
  uint32_t GetMtuSize ();

  /**
   * \brief Get the constant HTTP request size in bytes.
   * \return request size in bytes
   *
   * By default, HTTP request size is 350 bytes, which can be modified by
   * setting the `RequestSize` attribute or calling the SetRequestSize() method.
   * This value should apply to requests by HTTP client for main objects and
   * embedded objects alike.
   */
  uint32_t GetRequestSize ();

  /**
   * \brief Get the constant length of time needed by an HTTP server to generate
   *        a main object.
   * \return the delay for generating a main object
   *
   * By default, main objects are generated instantly, i.e., zero delay. This
   * can be modified by setting the `MainObjectGenerationDelay` attribute or by
   * calling the SetMainObjectGenerationDelay() method.
   */
  Time GetMainObjectGenerationDelay ();

  /**
   * \brief Get a random size (in bytes) of a main object to be sent by an HTTP
   *        server.
   * \return main object size in bytes
   *
   * The size of main objects are determined by a truncated log-normal random
   * distribution. The default distribution settings produces random integers
   * with a mean of 10710 bytes and a standard deviation of 25032 bytes, and
   * then truncated to fit between 100 bytes and 2 MB. These default settings
   * can be modified via attributes or class methods.
   */
  uint32_t GetMainObjectSize ();

  /**
   * \brief Get the constant length of time needed by an HTTP server to generate
   *        an embedded object.
   * \return the delay for generating an embedded object
   *
   * By default, embedded objects are generated instantly, i.e., zero delay.
   * This can be modified by setting the `EmbeddedObjectGenerationDelay`
   * attribute or by calling the SetEmbeddedObjectGenerationDelay() method.
   */
  Time GetEmbeddedObjectGenerationDelay ();

  /**
   * \brief Get a random size (in bytes) of an embedded object to be sent by an
   *        HTTP server.
   * \return embedded object size in bytes
   *
   * The size of embedded objects are determined by a truncated log-normal
   * random distribution. The default distribution settings produces random
   * integers with a mean of 7758 bytes and a standard deviation of 126168
   * bytes, and then truncated to fit between 100 bytes and 2 MB. These default
   * settings can be modified via attributes or class methods.
   */
  uint32_t GetEmbeddedObjectSize ();

  /**
   * \brief Get a random integer indicating the number of embedded objects in a
   *        main object.
   * \return the number of embedded objects
   *
   * The number of embedded objects in a main object is typically discovered
   * when the HTTP client is parsing the main object in question. This number is
   * determined by a truncated Pareto distribution. The default distribution
   * settings produces (after truncation) random integers between 0 and 53,
   * with an actual mean of approximately 3.95.
   */
  uint32_t GetNumOfEmbeddedObjects ();

  /**
   * \brief Get a random length of time which is spent by a hypothetical human
   *        user (HTTP client) to read a web page before transitioning to
   *        another web page.
   * \return time interval for reading a web page
   *
   * Reading time is determined by an exponential distribution. The default
   * distribution settings produces random values with a mean of 30 seconds
   * without any maximum bound. The mean can be modified by setting the
   * `ReadingTimeMean` attribute or by calling the SetReadingTimeMean() method.
   */
  Time GetReadingTime ();

  /**
   * \brief Get a random length of time which simulate the small delay caused
   *        by HTTP client looking for any embedded objects within the received
   *        main object.
   * \return time interval for parsing a main object
   *
   * Parsing time is determined by an exponential distribution. The default
   * distribution settings produces random values with a mean of 130 ms without
   * any maximum bound. The mean can be modified by setting the
   * `ParsingTimeMean` attribute or by calling the SetParsingTimeMean() method.
   */
  Time GetParsingTime ();

  /// Equivalent with GetReadingTime(), but only for plotting purpose.
  double GetReadingTimeSeconds ();

  /// Equivalent with GetParsingTime(), but only for plotting purpose.
  double GetParsingTimeSeconds ();

  /**
   * \brief Set a fixed random variable stream number to the random variables
   *        used by this model.
   * \param stream the stream index to use.
   *
   * Different random variable stream number makes random number generators to
   * produce different set of random values, thus may also produce different
   * simulation results. However, two identical simulations which use same
   * stream number should produce identical results as well (the repeatability
   * property of ns-3 simulation).
   */
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

  Ptr<UniformRandomVariable>            m_httpVersionRng;
  Ptr<UniformRandomVariable>            m_mtuSizeRng;
  Ptr<ConstantRandomVariable>           m_requestSizeRng;
  Ptr<ConstantRandomVariable>           m_mainObjectGenerationDelayRng;
  Ptr<TrafficBoundedLogNormalVariable>  m_mainObjectSizeRng;
  Ptr<ConstantRandomVariable>           m_embeddedObjectGenerationDelayRng;
  Ptr<TrafficBoundedLogNormalVariable>  m_embeddedObjectSizeRng;
  Ptr<TrafficBoundedParetoVariable>     m_numOfEmbeddedObjectsRng;
  Ptr<ExponentialRandomVariable>        m_readingTimeRng;
  Ptr<ExponentialRandomVariable>        m_parsingTimeRng;

}; // end of `class HttpVariables`


} // end of `namespace ns3`

#endif /* HTTP_VARIABLES_H */

