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

#include "nrtv-variables.h"
#include <ns3/log.h>
#include <ns3/integer.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <ns3/rng-stream.h>
#include <ns3/traffic-bounded-pareto-variable.h>


NS_LOG_COMPONENT_DEFINE ("NrtvVariables");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (NrtvVariables);


NrtvVariables::NrtvVariables ()
  : m_frameIntervalRng             (CreateObject<ConstantRandomVariable> ()),
    m_numOfSlicesRng               (CreateObject<ConstantRandomVariable> ()),
    m_sliceSizeRng                 (CreateObject<TrafficBoundedParetoVariable> ()),
    m_sliceEncodingDelayRng        (CreateObject<TrafficBoundedParetoVariable> ()),
    m_dejitterBufferWindowSizeRng  (CreateObject<ConstantRandomVariable> ())
{
  NS_LOG_FUNCTION (this);
}


TypeId
NrtvVariables::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrtvVariables")
    .SetParent<Object> ()
    .AddConstructor<NrtvVariables> ()
    .AddAttribute ("Stream",
                   "The stream number for the underlying random number generators stream. "
                   "-1 means \"allocate a stream automatically\".",
                   IntegerValue (-1),
                   MakeIntegerAccessor (&NrtvVariables::SetStream),
                   MakeIntegerChecker<int64_t> ())

    // FRAME INTERVAL
    .AddAttribute ("FrameInterval",
                   "The constant length of time between frames. The default "
                   "value of 100 ms is equivalent with 10 frames per second",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&NrtvVariables::SetFrameInterval),
                   MakeTimeChecker ())

    // NUMBER OF SLICES PER FRAME
    .AddAttribute ("NumOfSlices",
                   "The constant number of slices (packets) per frame.",
                   UintegerValue (8),
                   MakeUintegerAccessor (&NrtvVariables::SetNumOfSlices),
                   MakeUintegerChecker<uint16_t> ())

    // SLICE SIZE
    .AddAttribute ("SliceSizeMax",
                   "The upper bound parameter of Pareto distribution for the "
                   "slice size. The actual maximum value is this value "
                   "subtracted by the scale parameter",
                   UintegerValue (270),
                   MakeUintegerAccessor (&NrtvVariables::SetSliceSizeMax,
                                         &NrtvVariables::GetSliceSizeMax),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SliceSizeShape",
                   "The shape parameter of Pareto distribution for the slice size.",
                   DoubleValue (1.2),
                   MakeDoubleAccessor (&NrtvVariables::SetSliceSizeShape),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SliceSizeScale",
                   "The scale parameter of Pareto distribution for the slice size.",
                   DoubleValue (20.0),
                   MakeDoubleAccessor (&NrtvVariables::SetSliceSizeScale),
                   MakeDoubleChecker<double> ())

    // SLICE ENCODING DELAY
    .AddAttribute ("SliceEncodingDelayMax",
                   "The upper bound parameter of Pareto distribution for the "
                   "slice size. The actual maximum value is this value "
                   "subtracted by the scale parameter",
                   TimeValue (MilliSeconds (15)),
                   MakeTimeAccessor (&NrtvVariables::SetSliceEncodingDelayMax,
                                     &NrtvVariables::GetSliceEncodingDelayMax),
                   MakeTimeChecker ())
    .AddAttribute ("SliceEncodingDelayShape",
                   "The shape parameter of Pareto distribution for the slice size.",
                   DoubleValue (1.2),
                   MakeDoubleAccessor (&NrtvVariables::SetSliceEncodingDelayShape),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SliceEncodingDelayScale",
                   "The scale parameter of Pareto distribution for the slice size.",
                   DoubleValue (2.5),
                   MakeDoubleAccessor (&NrtvVariables::SetSliceEncodingDelayScale),
                   MakeDoubleChecker<double> ())

    // DE-JITTER BUFFER WINDOW SIZE
    .AddAttribute ("DejitterBufferWindowSize",
                   "The constant length of NRTV client's de-jitter buffer "
                   "window size.",
                   TimeValue (Seconds (5)),
                   MakeTimeAccessor (&NrtvVariables::SetDejitterBufferWindowSize),
                   MakeTimeChecker ())
  ;
  return tid;

} // end of `TypeId NrtvVariables::GetTypeId ()`


Time
NrtvVariables::GetFrameInterval ()
{
  return Seconds (m_frameIntervalRng->GetValue ());
}


uint16_t
NrtvVariables::GetNumOfSlices ()
{
  return m_numOfSlicesRng->GetInteger ();
}


uint32_t
NrtvVariables::GetSliceSize ()
{
  return m_sliceSizeRng->GetBoundedInteger ();
}


Time
NrtvVariables::GetSliceEncodingDelay ()
{
  return MilliSeconds (m_sliceEncodingDelayRng->GetBoundedInteger ());
}


uint64_t
NrtvVariables::GetSliceEncodingDelayMilliSeconds ()
{
  return m_sliceEncodingDelayRng->GetBoundedInteger ();
}


Time
NrtvVariables::GetDejitterBufferWindowSize ()
{
  return Seconds (m_dejitterBufferWindowSizeRng->GetValue ());
}


void
NrtvVariables::SetStream (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);

  m_frameIntervalRng->SetStream (stream);
  m_numOfSlicesRng->SetStream (stream);
  m_sliceSizeRng->SetStream (stream);
  m_sliceEncodingDelayRng->SetStream (stream);
}


// FRAME INTERVAL SETTER METHODS //////////////////////////////////////////////


void
NrtvVariables::SetFrameInterval (Time constant)
{
  NS_LOG_FUNCTION (this << constant.GetSeconds ());
  m_frameIntervalRng->SetAttribute ("Constant",
                                    DoubleValue (constant.GetSeconds ()));
}


// NUMBER OF SLICES PER FRAME SETTER METHODS //////////////////////////////////


void
NrtvVariables::SetNumOfSlices (uint16_t constant)
{
  NS_LOG_FUNCTION (this << constant);
  m_numOfSlicesRng->SetAttribute ("Constant",
                                  DoubleValue (static_cast<double> (constant)));
}


// SLICE SIZE ATTRIBUTES SETTER AND GETTER METHODS ////////////////////////////


void
NrtvVariables::SetSliceSizeMax (uint32_t max)
{
  NS_LOG_FUNCTION (this << max);
  m_sliceSizeRng->SetAttribute ("Bound",
                                DoubleValue (static_cast<double> (max)));
}


void
NrtvVariables::SetSliceSizeShape (double shape)
{
  NS_LOG_FUNCTION (this << shape);
  m_sliceSizeRng->SetAttribute ("Shape", DoubleValue (shape));
}


void
NrtvVariables::SetSliceSizeScale (double scale)
{
  NS_LOG_FUNCTION (this << scale);
  m_sliceSizeRng->SetScale (scale);
}


double
NrtvVariables::GetSliceSizeMean () const
{
  // extract value from parent class
  return m_sliceSizeRng->GetMean ();
}


uint32_t
NrtvVariables::GetSliceSizeMax () const
{
  // extract value from parent class
  return static_cast<uint32_t> (m_sliceSizeRng->GetBound ());
}


// SLICE ENCODING DELAY ATTRIBUTES SETTER AND GETTER METHODS //////////////////


void
NrtvVariables::SetSliceEncodingDelayMax (Time max)
{
  NS_LOG_FUNCTION (this << max.GetSeconds ());
  m_sliceEncodingDelayRng->SetAttribute ("Bound",
                                         DoubleValue (static_cast<double> (max.GetMilliSeconds ())));
}


void
NrtvVariables::SetSliceEncodingDelayShape (double shape)
{
  NS_LOG_FUNCTION (this << shape);
  m_sliceEncodingDelayRng->SetAttribute ("Shape", DoubleValue (shape));
}


void
NrtvVariables::SetSliceEncodingDelayScale (double scale)
{
  NS_LOG_FUNCTION (this << scale);
  m_sliceEncodingDelayRng->SetScale (scale);
}


Time
NrtvVariables::GetSliceEncodingDelayMean () const
{
  // extract value from parent class
  return MilliSeconds (m_sliceEncodingDelayRng->GetMean ());
}


Time
NrtvVariables::GetSliceEncodingDelayMax () const
{
  // extract value from parent class
  return MilliSeconds (m_sliceEncodingDelayRng->GetBound ());
}


// DE-JITTER BUFFER WINDOW SIZE SETTER METHODS ////////////////////////////////


void
NrtvVariables::SetDejitterBufferWindowSize (Time constant)
{
  NS_LOG_FUNCTION (this << constant.GetSeconds ());
  m_dejitterBufferWindowSizeRng->SetAttribute ("Constant",
                                               DoubleValue (constant.GetSeconds ()));
}


} // end of `namespace ns3`