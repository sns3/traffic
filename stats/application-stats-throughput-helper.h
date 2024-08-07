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

#ifndef APPLICATION_STATS_THROUGHPUT_HELPER_H
#define APPLICATION_STATS_THROUGHPUT_HELPER_H

#include "application-stats-helper.h"

#include <ns3/address.h>
#include <ns3/collector-map.h>
#include <ns3/ptr.h>

#include <list>
#include <map>

namespace ns3
{

// BASE CLASS /////////////////////////////////////////////////////////////////

class Application;
class Time;
class DataCollectionObject;
class DistributionCollector;

/**
 * \ingroup applicationstats
 * \brief
 */
class ApplicationStatsThroughputHelper : public ApplicationStatsHelper
{
  public:
    // inherited from ApplicationStatsHelper base class
    ApplicationStatsThroughputHelper();

    /// Destructor.
    virtual ~ApplicationStatsThroughputHelper();

    // inherited from ObjectBase base class
    static TypeId GetTypeId();

    /**
     * \param averagingMode average all samples before passing them to aggregator.
     */
    void SetAveragingMode(bool averagingMode);

    /**
     * \return the currently active averaging mode.
     */
    bool GetAveragingMode() const;

    /**
     * \brief Receive inputs from trace sources and determine the right collector
     *        to forward the inputs to.
     * \param packet received packet data.
     * \param from the address of the sender of the packet.
     *
     * Utilized to replace the role of probes when `SENDER` identifier is active.
     * The second argument contains the address of the packet sender, which is
     * then matched with the internal lookup table (pre-filled during Install())
     * to get an identifier value. The packet size is then forwarded to the
     * collector which has the same identifier.
     */
    void RxCallback(Ptr<const Packet> packet, const Address& from);

  protected:
    // inherited from ApplicationStatsHelper base class
    virtual void DoInstall();

  private:
    /**
     * \brief Associate the given application's IPv4 address with the given
     *        identifier.
     * \param application an application instance.
     * \param identifier the number to be associated with.
     *
     * Any IPv4 address(es) which belong to the Node of the given application
     * will be saved in the #m_identifierMap member variable. Used only with
     * `SENDER` identifier.
     */
    void SaveAddressAndIdentifier(Ptr<Application> application, uint32_t identifier);

    /// Maintains a list of probes created by this helper.
    std::list<Ptr<Probe>> m_probes;

    /// Maintains a list of first-level collectors created by this helper.
    CollectorMap m_conversionCollectors;

    /// Maintains a list of second-level collectors created by this helper.
    CollectorMap m_terminalCollectors;

    /// The final collector utilized in averaged output (histogram, PDF, and CDF).
    Ptr<DistributionCollector> m_averagingCollector;

    /// The aggregator created by this helper.
    Ptr<DataCollectionObject> m_aggregator;

    /// Map of address and the `SENDER` identifier associated with it.
    std::map<const Address, uint32_t> m_identifierMap;

    bool m_averagingMode; ///< `AveragingMode` attribute.

}; // end of class ApplicationStatsThroughputHelper

} // end of namespace ns3

#endif /* APPLICATION_STATS_THROUGHPUT_HELPER_H */
