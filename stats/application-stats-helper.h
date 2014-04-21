/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions
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

#ifndef APPLICATION_STATS_HELPER_H
#define APPLICATION_STATS_HELPER_H

#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/type-id.h>
#include <ns3/callback.h>
#include <ns3/application-container.h>
#include <ns3/collector-map.h>
#include <ns3/probe.h>
#include <list>
#include <map>


namespace ns3 {

class DataCollectionObject;
class Address;

/**
 * \ingroup traffic
 * \defgroup applicationstats Application Statistics
 *
 * Data Collection Framework (DCF) implementation on Application module. For
 * usage in simulation script, see ApplicationStatsHelperContainer.
 *
 * \warning ApplicationStatsHelperContainer takes care of setting the attributes
 *          `Name`, `IdentifierType`, and `OutputType`. Thus it's *not*
 *          recommended to manually set the values of these attributes while
 *          using ApplicationStatsHelperContainer.
 */

/**
 * \ingroup applicationstats
 * \brief Abstract class.
 */
class ApplicationStatsHelper : public Object
{
public:

  // COMMON ENUM DATA TYPES ///////////////////////////////////////////////////

  /**
   * \enum IdentifierType_t
   * \brief
   */
  typedef enum
  {
    IDENTIFIER_GLOBAL = 0,
    IDENTIFIER_SENDER,
    IDENTIFIER_RECEIVER
  } IdentifierType_t;

  /**
   * \param identifierType
   * \return
   */
  static std::string GetIdentifierTypeName (IdentifierType_t identifierType);

  /**
   * \enum OutputType_t
   * \brief
   */
  typedef enum
  {
    OUTPUT_NONE = 0,
    OUTPUT_SCALAR_FILE,
    OUTPUT_SCATTER_FILE,
    OUTPUT_HISTOGRAM_FILE,
    OUTPUT_PDF_FILE,        // probability distribution function
    OUTPUT_CDF_FILE,        // cumulative distribution function
    OUTPUT_SCALAR_PLOT,
    OUTPUT_SCATTER_PLOT,
    OUTPUT_HISTOGRAM_PLOT,
    OUTPUT_PDF_PLOT,        // probability distribution function
    OUTPUT_CDF_PLOT,        // cumulative distribution function
  } OutputType_t;

  /**
   * \param outputType
   * \return
   */
  static std::string GetOutputTypeName (OutputType_t outputType);

  // CONSTRUCTOR AND DESTRUCTOR ///////////////////////////////////////////////

  /**
   * \brief
   * \param identifierList
   */
  ApplicationStatsHelper ();

  /// Destructor.
  virtual ~ApplicationStatsHelper ();

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();


  // PUBLIC METHODS ///////////////////////////////////////////////////////////

  /**
   * \brief
   * \param info
   */
  void SetSenderInformation (std::map<std::string, ApplicationContainer> info);

  /**
   * \brief
   * \param info
   */
  void SetReceiverInformation (std::map<std::string, ApplicationContainer> info);

  /**
   * \brief Install probes, collectors, and aggregators.
   *
   * Behaviour should be implemented by child class in DoInstall().
   */
  void Install ();

  // SETTER AND GETTER METHODS ////////////////////////////////////////////////

  /**
   * \param name
   */
  void SetName (std::string name);

  /**
   * \return
   */
  std::string GetName () const;

  /**
   * \param traceSourceName
   */
  void SetTraceSourceName (std::string traceSourceName);

  /**
   * \return
   */
  std::string GetTraceSourceName () const;

  /**
   * \param identifierType
   * \warning Does not have any effect if invoked after Install().
   */
  void SetIdentifierType (IdentifierType_t identifierType);

  /**
   * \return
   */
  IdentifierType_t GetIdentifierType () const;

  /**
   * \param outputType
   * \warning Does not have any effect if invoked after Install().
   */
  void SetOutputType (OutputType_t outputType);

  /**
   * \return
   */
  OutputType_t GetOutputType () const;

  /**
   * \return
   */
  bool IsInstalled () const;

protected:
  /**
   * \brief
   */
  virtual void DoInstall () = 0;

  /**
   * \brief Create the aggregator according to the output type.
   * \param aggregatorTypeId the type of aggregator to be created.
   * \param n1 the name of the attribute to be set on the aggregator created.
   * \param v1 the value of the attribute to be set on the aggregator created.
   * \param n2 the name of the attribute to be set on the aggregator created.
   * \param v2 the value of the attribute to be set on the aggregator created.
   * \param n3 the name of the attribute to be set on the aggregator created.
   * \param v3 the value of the attribute to be set on the aggregator created.
   * \param n4 the name of the attribute to be set on the aggregator created.
   * \param v4 the value of the attribute to be set on the aggregator created.
   * \param n5 the name of the attribute to be set on the aggregator created.
   * \param v5 the value of the attribute to be set on the aggregator created.
   * \return the created aggregator.
   *
   * The created aggregator is stored in #m_aggregator. It can be retrieved
   * from outside using GetAggregator().
   */
  Ptr<DataCollectionObject> CreateAggregator (std::string aggregatorTypeId,
                                              std::string n1 = "",
                                              const AttributeValue &v1 = EmptyAttributeValue (),
                                              std::string n2 = "",
                                              const AttributeValue &v2 = EmptyAttributeValue (),
                                              std::string n3 = "",
                                              const AttributeValue &v3 = EmptyAttributeValue (),
                                              std::string n4 = "",
                                              const AttributeValue &v4 = EmptyAttributeValue (),
                                              std::string n5 = "",
                                              const AttributeValue &v5 = EmptyAttributeValue ());

  /**
   * \brief
   * \param collectorMap
   * \return number of collectors created.
   */
  uint32_t CreateCollectorPerIdentifier (CollectorMap &collectorMap) const;

  /**
   * \brief
   * \param probeOutputName
   * \param collectorMap
   * \param collectorTraceSink
   * \param probeList
   * \return number of probes created.
   */
  template<typename P, typename Q, typename R, typename C>
  uint32_t SetupProbesAtReceiver (std::string probeOutputName,
                                  CollectorMap & collectorMap,
                                  R (C::*collectorTraceSink) (Q, Q),
                                  std::list<Ptr<Probe> > & probeList);

  /**
   * \brief
   * \param cb
   * \return number of trace sources connected with the callback.
   */
  template<typename Q>
  uint32_t SetupListenersAtReceiver (Callback<void, Q, const Address &> cb);

  ///
  std::map<std::string, ApplicationContainer> m_senderInfo;

  ///
  std::map<std::string, ApplicationContainer> m_receiverInfo;

private:
  std::string       m_name;             ///<
  IdentifierType_t  m_identifierType;   ///<
  OutputType_t      m_outputType;       ///<
  std::string       m_traceSourceName;  ///<
  bool              m_isInstalled;      ///<

}; // end of class ApplicationStatsHelper


// TEMPLATE METHOD DEFINITIONS ////////////////////////////////////////////////

template<typename P, typename Q, typename R, typename C>
uint32_t
ApplicationStatsHelper::SetupProbesAtReceiver (std::string probeOutputName,
                                               CollectorMap & collectorMap,
                                               R (C::*collectorTraceSink) (Q, Q),
                                               std::list<Ptr<Probe> > & probeList)
{
  if (P::GetTypeId ().GetParent () != TypeId::LookupByName ("ns3::Probe"))
    {
      NS_FATAL_ERROR ("Invalid probe type");
    }

  uint32_t n = 0;
  uint32_t identifier = 0;
  NS_ASSERT (m_identifierType == ApplicationStatsHelper::IDENTIFIER_GLOBAL
             || m_identifierType == ApplicationStatsHelper::IDENTIFIER_RECEIVER);

  std::map<std::string, ApplicationContainer>::const_iterator it1;
  for (it1 = m_receiverInfo.begin (); it1 != m_receiverInfo.end (); ++it1)
    {
      for (ApplicationContainer::Iterator it2 = it1->second.Begin ();
           it2 != it1->second.End (); ++it2)
        {
          if ((*it2)->GetInstanceTypeId ().LookupTraceSourceByName (m_traceSourceName) != 0)
            {
              // Create the probe.
              Ptr<P> probe = CreateObject<P> ();
              probe->SetName (it1->first);

              // Connect the object to the probe.
              if (probe->ConnectByObject (m_traceSourceName, (*it2))
                  && collectorMap.ConnectWithProbe (probe,
                                                    probeOutputName,
                                                    identifier,
                                                    collectorTraceSink))
                {
                  probeList.push_back (probe->GetObject<Probe> ());
                  n++;
                }
            }

        } // end of `for (it2 = it1->second)`

      if (m_identifierType == ApplicationStatsHelper::IDENTIFIER_RECEIVER)
        {
          identifier++;
        }

    } // end of `for (it1 = m_receiverInfo)`

  return n;
}


template<typename Q>
uint32_t
ApplicationStatsHelper::SetupListenersAtReceiver (Callback<void, Q, const Address &> cb)
{
  uint32_t n = 0;

  std::map<std::string, ApplicationContainer>::const_iterator it1;
  for (it1 = m_receiverInfo.begin (); it1 != m_receiverInfo.end (); ++it1)
    {
      for (ApplicationContainer::Iterator it2 = it1->second.Begin ();
           it2 != it1->second.End (); ++it2)
        {
          if ((*it2)->GetInstanceTypeId ().LookupTraceSourceByName (m_traceSourceName) != 0
              && (*it2)->TraceConnectWithoutContext (m_traceSourceName, cb))
            {
              n++;
            }
        }
    }

  return n;
}


} // end of namespace ns3


#endif /* APPLICATION_STATS_HELPER_H */
