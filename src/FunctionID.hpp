/**
 * jadaq (Just Another DAQ)
 * Copyright (C) 2017  Troels Blum <troels@blum.dk>
 * Contributions from  Jonas Bardino <bardino@nbi.ku.dk>
 *
 * @file
 * @author Troels Blum <troels@blum.dk>
 * @section LICENSE
 * This program is free software: you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 *         but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 * Map JADAQ function names to configuration helpers.
 *
 */

#ifndef JADAQ_FUNCTIONID_HPP
#define JADAQ_FUNCTIONID_HPP

#include <string>

/*
 * NOTE: Do not change the ordering  in FunctionID unless you absolutely know
 * what you are doing.
 */
enum FunctionID {
  // Special functions - will be skipped by the begin to end iterater
  Register,
  // Global i.e. no channel/group
  MaxNumEventsBLT,
  ChannelEnableMask,
  GroupEnableMask,
  DecimationFactor,
  PostTriggerSize,
  IOlevel,
  AcquisitionMode,
  ExternalTriggerMode,
  SWTriggerMode,
  RunSynchronizationMode,
  OutputSignalMode,
  DESMode,
  ZeroSuppressionMode,
  AnalogMonOutput,
  AnalogInspectionMonParams,
  EventPackaging,
  TriggerCountingMode,
  FastTriggerDigitizing,
  FastTriggerMode,
  DRS4SamplingFrequency,
  DPPAcquisitionMode,
  DPPTriggerMode,
  MaxNumAggregatesBLT,
  SAMCorrectionLevel,
  SAMSamplingFrequency,
  SAMAcquisitionMode,
  TriggerLogic,
  BoardConfiguration,
  DPPAggregateOrganization,
  AcquisitionControl,
  AcquisitionStatus,
  GlobalTriggerMask,
  FrontPanelTRGOUTEnableMask,
  FrontPanelIOControl,
  ROCFPGAFirmwareRevision,
  EventSize,
  FanSpeedControl,
  DPPDisableExternalTrigger,
  RunStartStopDelay,
  ReadoutControl,
  ReadoutStatus,
  Scratch,
  DPPAggregateNumberPerBLT,
  // Channel/group optional
  DPPPreTriggerSize,
  RecordLength,
  NumEventsPerAggregate,
  DPPGateWidth,
  DPPGateOffset,
  DPPFixedBaseline,
  DPPAlgorithmControl,
  DPPTriggerHoldOffWidth,
  DPPShapedTriggerWidth,
  // Channel/group mandatory
  ChannelDCOffset,
  GroupDCOffset,
  AMCFirmwareRevision,
  ChannelSelfTrigger,
  GroupSelfTrigger,
  ChannelTriggerThreshold,
  GroupTriggerThreshold,
  ChannelGroupMask,
  TriggerPolarity,
  GroupFastTriggerThreshold,
  GroupFastTriggerDCOffset,
  ChannelPulsePolarity,
  ChannelZSParams,
  SAMPostTriggerSize,
  SAMTriggerCountVetoParam,
  FunctionID_SIZE // This MUST be the last FunctionID
};

static inline bool takeIndex(FunctionID id) { return id >= DPPPreTriggerSize; }
static inline bool needIndex(FunctionID id) { return id >= ChannelDCOffset; }
static inline FunctionID functionIDbegin() { return MaxNumEventsBLT; }
static inline FunctionID functionIDend() { return FunctionID_SIZE; }

static inline FunctionID &operator++(FunctionID &id) {
  id = (FunctionID)((int)id + 1);
  return id;
}
static inline FunctionID operator++(FunctionID &id, int) {
  FunctionID copy = id;
  id = (FunctionID)((int)id + 1);
  return copy;
}
static inline FunctionID &operator--(FunctionID &id) {
  id = (FunctionID)((int)id - 1);
  return id;
}
static inline FunctionID operator--(FunctionID &id, int) {
  FunctionID copy = id;
  id = (FunctionID)((int)id - 1);
  return copy;
}

FunctionID functionID(std::string s);
const std::string to_string(FunctionID id);

#endif // JADAQ_FUNCTIONID_HPP
