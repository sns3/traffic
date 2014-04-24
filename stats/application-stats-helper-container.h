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

#ifndef APPLICATION_STATS_HELPER_CONTAINER_H
#define APPLICATION_STATS_HELPER_CONTAINER_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/node-container.h>
#include <ns3/application-container.h>
#include <ns3/application-stats-helper.h>
#include <list>
#include <map>


namespace ns3 {

/*
 * The macro definitions following this comment block are used to declare the
 * majority of methods in this class. Below is the list of the class methods
 * created using this C++ pre-processing approach.
 *
 * - Add [Global,PerReceiver,PerSender] Throughput
 * - Add [Global,PerReceiver,PerSender] Delay
 *
 * Also check the Doxygen documentation of this class for more information.
 */

#define APPLICATION_STATS_METHOD_DECLARATION(id)                              \
  void AddGlobal ## id (ApplicationStatsHelper::OutputType_t outputType);     \
  void AddPerReceiver ## id (ApplicationStatsHelper::OutputType_t outputType);\
  void AddPerSender ## id (ApplicationStatsHelper::OutputType_t outputType);

/**
 * \ingroup applicationstats
 * \brief Container of ApplicationStatsHelper instances.
 *
 * The container is initially empty upon creation. ApplicationStatsHelper
 * instances can be added into the container using attributes or class methods.
 *
 * The value of the attributes and the arguments of the class methods are the
 * desired output type (e.g., scalar, scatter, histogram, files, plots, etc.).
 *
 * The output files will be named in a certain pattern using the name set in
 * the `Name` attribute or SetName() method. The default name is "stat", e.g.,
 * which will produce output files with the names such as
 * `stat-per-receiver-throughput-scalar.txt`,
 * `stat-per-receiver-delay-cdf-receiver-1.txt`, etc.
 */
class ApplicationStatsHelperContainer : public Object
{
public:
  /**
   * \brief Creates a new instance of container.
   */
  ApplicationStatsHelperContainer ();

  // inherited from ObjectBase base class
  static TypeId GetTypeId ();

  /**
   * \param name a string prefix to be prepended on every output file name.
   */
  void SetName (std::string name);

  /**
   * \return a string prefix prepended on every output file name.
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

  // Throughput statistics.
  APPLICATION_STATS_METHOD_DECLARATION (Throughput)

  // Throughput statistics.
  APPLICATION_STATS_METHOD_DECLARATION (Delay)

  /**
   * \param outputType an arbitrary output type.
   * \return a string suffix to be appended at the end of the corresponding
   *         output file for this output type.
   */
  static std::string GetOutputTypeSuffix (ApplicationStatsHelper::OutputType_t outputType);

  // SENDER APPLICATIONS //////////////////////////////////////////////////////

  /**
   * \brief
   * \param container
   * \param identifier
   */
  void AddSenderApplication (Ptr<Application> application,
                             std::string identifier = "");

  /**
   * \brief
   * \param container
   * \param isGroup
   * \param groupIdentifier
   */
  void AddSenderApplications (ApplicationContainer container,
                              bool isGroup = false,
                              std::string groupIdentifier = "");

  /**
   * \brief
   * \param container
   * \param isGroup
   * \param groupIdentifier
   */
  void AddSenderNode (Ptr<Node> node,
                      bool isGroup = false,
                      std::string groupIdentifier = "");

  /**
   * \brief
   * \param container
   * \param isGroup
   * \param groupIdentifier
   */
  void AddSenderNodes (NodeContainer container,
                       bool isGroup = false,
                       std::string groupIdentifier = "");

  // RECEIVER APPLICATIONS ////////////////////////////////////////////////////

  /**
   * \brief
   * \param container
   * \param identifier
   */
  void AddReceiverApplication (Ptr<Application> application,
                               std::string identifier = "");

  /**
   * \brief
   * \param container
   * \param isGroup
   * \param groupIdentifier
   */
  void AddReceiverApplications (ApplicationContainer container,
                                bool isGroup = false,
                                std::string groupIdentifier = "");

  /**
   * \brief
   * \param container
   * \param isGroup
   * \param groupIdentifier
   */
  void AddReceiverNode (Ptr<Node> node,
                        bool isGroup = false,
                        std::string groupIdentifier = "");

  /**
   * \brief
   * \param container
   * \param isGroup
   * \param groupIdentifier
   */
  void AddReceiverNodes (NodeContainer container,
                         bool isGroup = false,
                         std::string groupIdentifier = "");

protected:
  // Inherited from Object base class
  virtual void DoDispose ();

private:
  /// Prefix of every ApplicationStatsHelper instance names and every output file.
  std::string m_name;

  /// The name of the application's trace source which produce the required information.
  std::string m_traceSourceName;

  /// Maintains the active ApplicationStatsHelper instances which have created.
  std::list<Ptr<const ApplicationStatsHelper> > m_stats;

  ///
  std::map<std::string, ApplicationContainer> m_senderInfo;

  ///
  std::map<std::string, ApplicationContainer> m_receiverInfo;

}; // end of class ApplicationStatsHelperContainer


} // end of namespace ns3


#endif /* APPLICATION_STATS_HELPER_CONTAINER_H */
