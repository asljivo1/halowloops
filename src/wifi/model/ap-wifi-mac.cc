/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006, 2009 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Mirko Banchi <mk.banchi@gmail.com>
 */

#include "ap-wifi-mac.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"
#include "qos-tag.h"
#include "wifi-phy.h"
#include "dcf-manager.h"
#include "mac-rx-middle.h"
#include "mac-tx-middle.h"
#include "mgt-headers.h"
//#include "extension-headers.h"
#include "mac-low.h"
#include "amsdu-subframe-header.h"
#include "msdu-aggregator.h"
#include "ns3/uinteger.h"
#include "wifi-mac-queue.h"
#include <map>
#include "s1g-raw-control.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ApWifiMac");

NS_OBJECT_ENSURE_REGISTERED(ApWifiMac);

#define LOG_TRAFFIC(msg)	if(true) NS_LOG_DEBUG(Simulator::Now().GetMicroSeconds() << " " << msg << std::endl);

TypeId ApWifiMac::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::ApWifiMac").SetParent<RegularWifiMac>().SetGroupName(
					"Wifi").AddConstructor<ApWifiMac>().AddAttribute(
					"BeaconInterval", "Delay between two beacons",
					TimeValue(MicroSeconds(102400)),
					MakeTimeAccessor(&ApWifiMac::GetBeaconInterval,
							&ApWifiMac::SetBeaconInterval), MakeTimeChecker()).AddAttribute(
					"BeaconJitter",
					"A uniform random variable to cause the initial beacon starting time (after simulation time 0) "
							"to be distributed between 0 and the BeaconInterval.",
					StringValue("ns3::UniformRandomVariable"),
					MakePointerAccessor(&ApWifiMac::m_beaconJitter),
					MakePointerChecker<UniformRandomVariable>()).AddAttribute(
					"Outputpath",
					"output path to store info of sensors and offload stations",
					StringValue("stationfile"),
					MakeStringAccessor(&ApWifiMac::m_outputpath),
					MakeStringChecker()).AddAttribute("EnableBeaconJitter",
					"If beacons are enabled, whether to jitter the initial send event.",
					BooleanValue(false),
					MakeBooleanAccessor(&ApWifiMac::m_enableBeaconJitter),
					MakeBooleanChecker()).AddAttribute("BeaconGeneration",
					"Whether or not beacons are generated.", BooleanValue(true),
					MakeBooleanAccessor(&ApWifiMac::SetBeaconGeneration,
							&ApWifiMac::GetBeaconGeneration),
					MakeBooleanChecker()).AddAttribute("ChannelWidth",
					"Channel width of the stations ", UintegerValue(0),
					MakeUintegerAccessor(&ApWifiMac::GetChannelWidth,
							&ApWifiMac::SetChannelWidth),
					MakeUintegerChecker<uint32_t>()).AddAttribute(
					"NRawGroupStas", "Number of stations in one Raw Group",
					UintegerValue(6000),
					MakeUintegerAccessor(&ApWifiMac::GetRawGroupInterval,
							&ApWifiMac::SetRawGroupInterval),
					MakeUintegerChecker<uint32_t>()).AddAttribute(
					"NRawStations", "Number of total stations support RAW",
					UintegerValue(100),
					MakeUintegerAccessor(&ApWifiMac::GetTotalStaNum,
							&ApWifiMac::SetTotalStaNum),
					MakeUintegerChecker<uint32_t>()).AddAttribute("SlotFormat",
					"Slot format", UintegerValue(1),
					MakeUintegerAccessor(&ApWifiMac::GetSlotFormat,
							&ApWifiMac::SetSlotFormat),
					MakeUintegerChecker<uint32_t>()).AddAttribute(
					"SlotCrossBoundary", "cross slot boundary or not",
					UintegerValue(1),
					MakeUintegerAccessor(&ApWifiMac::GetSlotCrossBoundary,
							&ApWifiMac::SetSlotCrossBoundary),
					MakeUintegerChecker<uint32_t>()).AddAttribute(
					"SlotDurationCount", "slot duration count",
					UintegerValue(1000),
					MakeUintegerAccessor(&ApWifiMac::GetSlotDurationCount,
							&ApWifiMac::SetSlotDurationCount),
					MakeUintegerChecker<uint32_t>()).AddAttribute("SlotNum",
					"Number of slot", UintegerValue(2),
					MakeUintegerAccessor(&ApWifiMac::GetSlotNum,
							&ApWifiMac::SetSlotNum),
					MakeUintegerChecker<uint32_t>()).AddAttribute("RPSsetup",
					"configuration of RAW", RPSVectorValue(),
					MakeRPSVectorAccessor(&ApWifiMac::m_rpsset),
					MakeRPSVectorChecker()).AddTraceSource(
					"S1gBeaconBroadcasted",
					"Fired when a beacon is transmitted",
					MakeTraceSourceAccessor(&ApWifiMac::m_transmitBeaconTrace),
					"ns3::ApWifiMac::S1gBeaconTracedCallback").AddTraceSource(
					"RpsIndex", "Fired when RPS index changes",
					MakeTraceSourceAccessor(&ApWifiMac::m_rpsIndexTrace),
					"ns3::TracedValueCallback::Uint16").AddTraceSource(
					"RawGroup", "Fired when RAW group index changes",
					MakeTraceSourceAccessor(&ApWifiMac::m_rawGroupTrace),
					"ns3::TracedValueCallback::Uint8").AddTraceSource("RawSlot",
					"Fired when RAW slot index changes",
					MakeTraceSourceAccessor(&ApWifiMac::m_rawSlotTrace),
					"ns3::TracedValueCallback::Uint8")
			/*.AddTraceSource("RAWSlotStarted",
			 "Fired when a RAW slot has started",
			 MakeTraceSourceAccessor(&ApWifiMac::m_rawSlotStarted),
			 "ns3::S1gApWifiMac::RawSlotStartedCallback")*/
			.AddTraceSource("PacketToTransmitReceivedFromUpperLayer",
					"Fired when packet is received from the upper layer",
					MakeTraceSourceAccessor(
							&ApWifiMac::m_packetToTransmitReceivedFromUpperLayer),
					"ns3::S1gApWifiMac::PacketToTransmitReceivedFromUpperLayerCallback").AddAttribute(
					"PageSliceSet", "configuration of PageSlice",
					pageSliceValue(),
					MakepageSliceAccessor(&ApWifiMac::m_pageslice),
					MakepageSliceChecker()).AddAttribute("PageSlicingActivated",
					"Whether or not page slicing is activated.",
					BooleanValue(true),
					MakeBooleanAccessor(&ApWifiMac::SetPageSlicingActivated,
							&ApWifiMac::GetPageSlicingActivated),
					MakeBooleanChecker()).AddAttribute("TIMSet",
					"configuration of TIM", TIMValue(),
					MakeTIMAccessor(&ApWifiMac::m_TIM), MakeTIMChecker());
	/*
	 .AddAttribute ("DTIMPeriod", "TIM number in one of DTIM",
	 UintegerValue (4),
	 MakeUintegerAccessor (&ApWifiMac::GetDTIMPeriod,
	 &ApWifiMac::SetDTIMPeriod),
	 MakeUintegerChecker<uint8_t> ());
	 */
	return tid;
}

ApWifiMac::ApWifiMac() {
	NS_LOG_FUNCTION(this);
	m_beaconDca = CreateObject<DcaTxop>();
	m_beaconDca->SetAifsn(1);
	m_beaconDca->SetMinCw(0);
	m_beaconDca->SetMaxCw(0);
	m_beaconDca->SetLow(m_low);
	m_beaconDca->SetManager(m_dcfManager);
	m_beaconDca->SetTxMiddle(m_txMiddle);

	//Let the lower layers know that we are acting as an AP.
	SetTypeOfStation(AP);

	m_enableBeaconGeneration = false;
	AuthenThreshold = 0;
	currentRawGroup = 0;
	//m_SlotFormat = 0;
	m_AidToMacAddr.clear();
	m_accessList.clear();
	m_supportPageSlicingList.clear();
	m_sleepList.clear();
	m_DTIMCount = 0;
	m_updateRps = false;
	//m_DTIMOffset = 0;
}

ApWifiMac::~ApWifiMac() {
	NS_LOG_FUNCTION(this);
}

void ApWifiMac::DoDispose() {
	NS_LOG_FUNCTION(this);
	m_beaconDca = 0;
	m_enableBeaconGeneration = false;
	m_beaconEvent.Cancel();
	for (uint32_t i = 0; i < m_rawSlotsDca.size(); i++) {
		m_rawSlotsDca[i]->Dispose();
		for (EdcaQueues::iterator it = m_rawSlotsEdca[i].begin();
				it != m_rawSlotsEdca[i].end(); ++it) {
			it->second = 0;
		}
	}
	RegularWifiMac::DoDispose();
}

void ApWifiMac::SetAddress(Mac48Address address) {
	NS_LOG_FUNCTION(this << address);
	//As an AP, our MAC address is also the BSSID. Hence we are
	//overriding this function and setting both in our parent class.
	RegularWifiMac::SetAddress(address);
	RegularWifiMac::SetBssid(address);
}

void ApWifiMac::SetPageSlicingActivated(bool activate) {
	NS_LOG_FUNCTION(this);
	m_pageSlicingActivated = activate;
}

bool ApWifiMac::GetPageSlicingActivated(void) const {
	NS_LOG_FUNCTION(this);
	return m_pageSlicingActivated;
}

void ApWifiMac::SetBeaconGeneration(bool enable) {
	NS_LOG_FUNCTION(this << enable);
	if (!enable) {
		m_beaconEvent.Cancel();
	} else if (enable && !m_enableBeaconGeneration) {
		m_beaconEvent = Simulator::ScheduleNow(&ApWifiMac::SendOneBeacon, this);
	}
	m_enableBeaconGeneration = enable;
}

bool ApWifiMac::GetBeaconGeneration(void) const {
	NS_LOG_FUNCTION(this);
	return m_enableBeaconGeneration;
}

Time ApWifiMac::GetBeaconInterval(void) const {
	NS_LOG_FUNCTION(this);
	return m_beaconInterval;
}

uint32_t ApWifiMac::GetRawGroupInterval(void) const {
	NS_LOG_FUNCTION(this);
	return m_rawGroupInterval;
}

uint32_t ApWifiMac::GetTotalStaNum(void) const {
	NS_LOG_FUNCTION(this);
	return m_totalStaNum;
}

uint32_t ApWifiMac::GetSlotFormat(void) const {
	return m_SlotFormat;
}

uint32_t ApWifiMac::GetSlotCrossBoundary(void) const {
	return m_slotCrossBoundary;
}

uint32_t ApWifiMac::GetSlotDurationCount(void) const {
	return m_slotDurationCount;
}

uint32_t ApWifiMac::GetSlotNum(void) const {
	return m_slotNum;
}

uint8_t ApWifiMac::GetDTIMPeriod(void) const {
	return m_DTIMPeriod;
}

void ApWifiMac::SetDTIMPeriod(uint8_t period) {
	m_DTIMPeriod = period;
	//m_DTIMOffset = m_DTIMPeriod - 1;
}

void ApWifiMac::SetWifiRemoteStationManager(
		Ptr<WifiRemoteStationManager> stationManager) {
	NS_LOG_FUNCTION(this << stationManager);
	m_beaconDca->SetWifiRemoteStationManager(stationManager);
	for (uint32_t i = 0; i < m_rawSlotsDca.size(); i++) //auto& p : m_rawSlotsDca
			{
		m_rawSlotsDca[i]->SetWifiRemoteStationManager(stationManager);
		for (EdcaQueues::iterator it = m_rawSlotsEdca[i].begin();
				it != m_rawSlotsEdca[i].end(); ++it) {
			it->second->SetWifiRemoteStationManager(stationManager);
		}
	}
	RegularWifiMac::SetWifiRemoteStationManager(stationManager);
}

void ApWifiMac::SetLinkUpCallback(Callback<void> linkUp) {
	NS_LOG_FUNCTION(this << &linkUp);
	RegularWifiMac::SetLinkUpCallback(linkUp);

	//The approach taken here is that, from the point of view of an AP,
	//the link is always up, so we immediately invoke the callback if
	//one is set
	linkUp();
}

void ApWifiMac::SetBeaconInterval(Time interval) {
	NS_LOG_FUNCTION(this << interval);
	if ((interval.GetMicroSeconds() % 1024) != 0) {
		NS_LOG_WARN(
				"beacon interval should be multiple of 1024us (802.11 time unit), see IEEE Std. 802.11-2012");
	}
	m_beaconInterval = interval;
	//NS_LOG_UNCOND("beacon interval = " << m_beaconInterval);
}

void ApWifiMac::SetRawGroupInterval(uint32_t interval) {
	NS_LOG_FUNCTION(this << interval);
	m_rawGroupInterval = interval;
}

void ApWifiMac::SetTotalStaNum(uint32_t num) {
	NS_LOG_FUNCTION(this << num);
	m_totalStaNum = num;
	//m_S1gRawCtr.RAWGroupping (m_totalStaNum, 1, m_beaconInterval.GetMicroSeconds ());
	//m_S1gRawCtr.configureRAW ();

}

void ApWifiMac::SetSlotFormat(uint32_t format) {
	NS_ASSERT(format <= 1);
	m_SlotFormat = format;
}

void ApWifiMac::SetSlotCrossBoundary(uint32_t cross) {
	NS_ASSERT(cross <= 1);
	m_slotCrossBoundary = cross;
}

void ApWifiMac::SetSlotDurationCount(uint32_t count) {
	NS_ASSERT(
			(!m_SlotFormat & (count < 256)) || (m_SlotFormat & (count < 2048)));
	m_slotDurationCount = count;
}

void ApWifiMac::SetSlotNum(uint32_t count) {
	NS_ASSERT((!m_SlotFormat & (count < 64)) || (m_SlotFormat & (count < 8)));
	m_slotNum = count;
}

void ApWifiMac::SetChannelWidth(uint32_t width) {
	m_channelWidth = width;
}

uint32_t ApWifiMac::GetChannelWidth(void) const {
	//NS_LOG_UNCOND (GetAddress () << ", GetChannelWidth " << m_channelWidth );
	return m_channelWidth;
}

void ApWifiMac::StartBeaconing(void) {
	NS_LOG_FUNCTION(this);
	SendOneBeacon();
}

int64_t ApWifiMac::AssignStreams(int64_t stream) {
	NS_LOG_FUNCTION(this << stream);
	m_beaconJitter->SetStream(stream);
	return 1;
}

void ApWifiMac::ForwardDown(Ptr<const Packet> packet, Mac48Address from,
		Mac48Address to) {
	NS_LOG_FUNCTION(this << packet << from << to);
	//If we are not a QoS AP then we definitely want to use AC_BE to
	//transmit the packet. A TID of zero will map to AC_BE (through \c
	//QosUtilsMapTidToAc()), so we use that as our default here.
	uint8_t tid = 0;

	//If we are a QoS AP then we attempt to get a TID for this packet
	if (m_qosSupported) {
		tid = QosUtilsGetTidForPacket(packet);
		//Any value greater than 7 is invalid and likely indicates that
		//the packet had no QoS tag, so we revert to zero, which'll
		//mean that AC_BE is used.
		if (tid > 7) {
			tid = 0;
		}
	}

	ForwardDown(packet, from, to, tid);
}

void ApWifiMac::ForwardDown(Ptr<const Packet> packet, Mac48Address from,
		Mac48Address to, uint8_t tid) {
	NS_LOG_FUNCTION(
			this << packet << from << to << static_cast<uint32_t> (tid));
	WifiMacHeader hdr;

	//For now, an AP that supports QoS does not support non-QoS
	//associations, and vice versa. In future the AP model should
	//support simultaneously associated QoS and non-QoS STAs, at which
	//point there will need to be per-association QoS state maintained
	//by the association state machine, and consulted here.
	if (m_qosSupported) {
		hdr.SetType(WIFI_MAC_QOSDATA);
		hdr.SetQosAckPolicy(WifiMacHeader::NORMAL_ACK);
		hdr.SetQosNoEosp();
		hdr.SetQosNoAmsdu();
		//Transmission of multiple frames in the same TXOP is not
		//supported for now
		hdr.SetQosTxopLimit(0);
		//Fill in the QoS control field in the MAC header
		hdr.SetQosTid(tid);
	} else {
		hdr.SetTypeData();
	}

	if (m_htSupported) {
		hdr.SetNoOrder();
	}
	hdr.SetAddr1(to);
	hdr.SetAddr2(GetAddress());
	hdr.SetAddr3(from);
	hdr.SetDsFrom();
	hdr.SetDsNotTo();

	int aid = 0;

	do {
		aid++;
	} while (m_AidToMacAddr.find(aid)->second != to); //TODO optimize search

	NS_LOG_INFO(
			Simulator::Now().GetMicroSeconds() << " ms: AP to forward data for [aid=" << aid << "]");

	if (m_qosSupported) {
		//Sanity check that the TID is valid
		NS_ASSERT(tid < 8);
		uint32_t targetSlot = GetNextSlotNumFromAid(aid);
		//std::cout << ">>AP enqueues to dst aid=" << (int)aid << "targetSlot=" << targetSlot << std::endl;
		//m_rawSlotsEdca[GetSlotNumFromAid (aid)].find(QosUtilsMapTidToAc (tid))->second->Queue(packet, hdr);

		// if paged both in DTIM and in TIM enqueue immediately
		//if (IsPagedInDtim (aid))
		if (IsPagedInDtim (aid))
		{
			//NS_LOG_UNCOND("++++++++++++++++++ aid=" << aid << " PAGED");
			m_rawSlotsEdca[targetSlot][QosUtilsMapTidToAc(tid)]->Queue(packet, hdr);
		}
		else
		{

			if (targetSlot > GetAllSlotNumbersFromAid(aid)[0])
				m_rawSlotsEdca[GetAllSlotNumbersFromAid(aid)[0]][QosUtilsMapTidToAc(tid)]->Queue(packet, hdr);
			else
				NS_LOG_UNCOND("NOT IMPLEMENTED CASE in ApWifiMac::ForwardDown. Delay for the next beacon interval because AID is not paged in the current one.");

			//NS_LOG_UNCOND("++++++++++++++++++ aid=" << aid << " NOT PAGED");
		}
		//else enqueue after it is paged (next DTIM)

		//m_edca[QosUtilsMapTidToAc (tid)]->Queue (packet, hdr);
	} else {
		// queue the packet in the specific raw slot period DCA
		m_rawSlotsDca[GetNextSlotNumFromAid(aid)]->Queue(packet, hdr);
		//m_dca->Queue (packet, hdr);
	}

}

bool
ApWifiMac::IsPagedInDtim (uint32_t aid)
{
	//NS_LOG_UNCOND("+++++++++++++++++++++++GetPageBitmapLength=" << (int)m_pageslice.GetPageBitmapLength());
	if (!m_pageslice.GetPageBitmapLength())
		return false;

	uint32_t aid_block = (aid >> 6) & 0x1f;
	uint32_t aid_page = aid >> 11;

	if (aid_page != m_pageslice.GetPageindex())
		return false;

	if (aid_block < m_pageslice.GetBlockOffset())
		return false;

	uint8_t Ind, offset;
	bool paged;
	Ind = aid_block / 8;
	offset = aid_block % 8;
	paged = m_pageslice.GetPageBitmap()[Ind] & (1 << offset);
	return paged;
}

bool ApWifiMac::IsPagedInTim (uint32_t aid)
{
	uint32_t aid_block = (aid >> 6) & 0x1f;
	uint32_t aid_subblock = (aid >> 3) & 0x07;
	uint32_t aid_page = aid >> 11;
	uint32_t aid_PageSliceNumber = (aid_block - m_pageslice.GetBlockOffset()) / m_pageslice.GetPageSliceLen();

	uint8_t * partialVBitmap = m_TIM.GetPartialVBitmap();
	if(m_TIM.GetInformationFieldSize() < 5)
		return false;

	uint8_t pos = 0;
	do
	{
		uint8_t blockind = ((*partialVBitmap) >> 3) & 0x1f;
		partialVBitmap++;
		pos++;
		uint8_t blockbitmap = *partialVBitmap;

		if (blockind == aid_block)
		{
			if ((blockbitmap & (1 << aid_subblock)) == 0) //no packets in subblock
			{
				return false;
			}
			else
			{
				for (uint8_t j = 0; j <= aid_subblock; j++)
				{
					if ((blockbitmap & (0x01 << j)))//==1 is incorrect because sometimes it can be 0000 0010 e.g. 2
					{
						partialVBitmap++;
						pos++;
					}
				}
				uint8_t subblockind = *partialVBitmap;
				if ((subblockind & (1 << (aid & 0x0007))) == 0) //no packet for me
				{
					return false;
				}
				else
				{
					return true;
				}
			}
		}
		else
		{
			for (uint8_t k = 0; k <= 7; k++)
			{
				if ((blockbitmap & (1 << k)) == 1)
				{
					partialVBitmap++;
					pos++;
				}
			}
		}
	}
	while (pos < m_TIM.GetInformationFieldSize() - 3);
}

std::vector<uint32_t>
ApWifiMac::GetAllSlotNumbersFromAid (uint16_t aid) const
{
	NS_ASSERT(aid > 0 && aid < 8192);
	uint32_t numTim = 0;
	if (m_pageslice.GetPageSliceCount() == 0)
		numTim = 1;
	else
		numTim = m_pageslice.GetPageSliceCount();

	uint32_t myslot = 0;
	std::vector <uint32_t> myslotsVector;
	//bool found = false;


	for (uint32_t i = 0; i < numTim; i++)
	{
		//std::cout << "i=" << i << " has numRawGrups=" << (int)m_rpsset.rpsset.at(i)->GetNumberOfRawGroups() << std::endl;
		for (uint32_t j = 0; j < m_rpsset.rpsset.at(i)->GetNumberOfRawGroups();	j++)
		{
			RPS::RawAssignment ass = m_rpsset.rpsset.at(i)->GetRawAssigmentObj(j);
			//std::cout << "    j="<<j<< ", aidStart=" <<(int)ass.GetRawGroupAIDStart() << ", aidEnd=" << (int)ass.GetRawGroupAIDEnd() <<std::endl;

			if (ass.GetRawGroupAIDStart() <= aid && aid <= ass.GetRawGroupAIDEnd()) {
				myslot += aid % ass.GetSlotNum();
				myslotsVector.push_back(myslot);
				myslot += ass.GetSlotNum();
				myslot -= aid % ass.GetSlotNum();

				//found = true;
				//break;
			} else
				myslot += ass.GetSlotNum();
		}
	}
	/*std::cout << "\n aid=" << (int)aid << ", slots=";
	for (auto slotnum : myslotsVector)
		std::cout <<slotnum << ", ";
	std::cout << std::endl;*/
	return myslotsVector;
}

uint32_t ApWifiMac::GetNextSlotNumFromAid(uint16_t aid) const {
	NS_ASSERT(aid > 0 && aid < 8192);
	uint32_t numTim = 0;
	if (m_pageslice.GetPageSliceCount() == 0)
		numTim = 1;
	else
		numTim = m_pageslice.GetPageSliceCount();

	uint32_t myslot = 0;
	std::vector <uint32_t> myslotsVector;
	//bool found = false;


	for (uint32_t i = 0; i < numTim; i++)
	{
		//std::cout << "i=" << i << " has numRawGrups=" << (int)m_rpsset.rpsset.at(i)->GetNumberOfRawGroups() << std::endl;
		for (uint32_t j = 0; j < m_rpsset.rpsset.at(i)->GetNumberOfRawGroups();	j++)
		{
			RPS::RawAssignment ass = m_rpsset.rpsset.at(i)->GetRawAssigmentObj(j);
			//std::cout << "    j="<<j<< ", aidStart=" <<(int)ass.GetRawGroupAIDStart() << ", aidEnd=" << (int)ass.GetRawGroupAIDEnd() <<std::endl;

			if (ass.GetRawGroupAIDStart() <= aid
					&& aid <= ass.GetRawGroupAIDEnd()) {
				myslot += aid % ass.GetSlotNum();
				myslotsVector.push_back(myslot);
				myslot += ass.GetSlotNum();
				myslot -= aid % ass.GetSlotNum();

				//found = true;
				//break;
			} else
				myslot += ass.GetSlotNum();
		}
		//if (found)
			//break;
	}
	uint32_t currentSlot = this->GetSlotNumFromRpsRawSlot(m_rpsIndexTrace, m_rawGroupTrace, m_rawSlotTrace);
	bool found = false;
	for (auto targetslot : myslotsVector)
	{
		if (targetslot > currentSlot)
		{
			//schedule downlink to the first next slot STA is assigned to
			if (aid==1)
			std::cout << "  " << targetslot;
			myslot = targetslot;
			found = true;
			break;
		}
	}
	// if all the next slots until the next BI are not STA slots, schedule to the first slot after the next BI
	if (!found)
		myslot = myslotsVector[0];
	if (aid==1)
	//std::cout << "\n aid=" << (int)aid << ", targetslot=" << myslot<< " current slot=" << currentSlot << std::endl;

	return myslot;
}

uint32_t ApWifiMac::GetSlotNumFromRpsRawSlot(uint16_t rps, uint8_t rawg,
		uint8_t slot) const {
	uint32_t myslot = 0;
	bool found = false;
	for (uint32_t i = 0; i < rps; i++) {
		for (uint32_t j = 0; j < m_rpsset.rpsset.at(i)->GetNumberOfRawGroups();
				j++) {
			RPS::RawAssignment ass = m_rpsset.rpsset.at(i)->GetRawAssigmentObj(
					j);
			if (j == rawg - 1 && i == rps - 1) {
				//my raw group
				myslot += slot - 1;
				found = true;
				break;
			} else
				myslot += ass.GetSlotNum();
		}
		if (found)
			break;
	}
	return myslot;
	/*return GetSlotNumFromAid(
			m_rpsset.rpsset.at(rps)->GetRawAssigmentObj(rawg).GetSlotNum())
			+ slot;*/
}

Time ApWifiMac::GetSlotDurationFromAid(uint16_t aid) const {
	uint16_t raw_len =
			(*m_rpsset.rpsset.at(GetTimFromAid(aid))).GetInformationFieldSize();

	uint16_t rawAssignment_len = 6;
	if (raw_len % rawAssignment_len != 0) {
		NS_ASSERT("RAW configuration incorrect!");
	}
	uint8_t RAW_number = raw_len / rawAssignment_len;
	for (uint8_t raw_index = 0; raw_index < RAW_number; raw_index++) {
		RPS::RawAssignment ass =
				m_rpsset.rpsset.at(GetTimFromAid(aid))->GetRawAssigmentObj(
						raw_index);
		if (ass.GetRawGroupAIDStart() <= aid
				&& aid <= ass.GetRawGroupAIDEnd()) {
			NS_LOG_DEBUG(
					"[aid=" << aid << "] is located in RAW " << (int)raw_index + 1 << ". RAW slot duration = " << 500 + ass.GetSlotDurationCount() * 120 << " us.");
			return MicroSeconds(500 + ass.GetSlotDurationCount() * 120);
		}
	}
}

Time ApWifiMac::GetSlotStartTimeFromAid(uint16_t aid) const {
	//std::cout << "aid=" << (int)aid << ", toTim=" << (int)GetTimFromAid(aid) << std::endl;
	uint16_t raw_len =
			(*m_rpsset.rpsset.at(GetTimFromAid(aid))).GetInformationFieldSize();

	uint16_t rawAssignment_len = 6;
	if (raw_len % rawAssignment_len != 0) {
		NS_ASSERT("RAW configuration incorrect!");
	}
	uint8_t RAW_number = raw_len / rawAssignment_len;

	uint16_t slotDurationCount = 0;
	uint16_t slotNum = 0;
	uint64_t currentRAW_start = 0;
	Time lastRawDurationus = MicroSeconds(0);
	int x = 0;
	for (uint8_t raw_index = 0; raw_index < RAW_number; raw_index++) {
		RPS::RawAssignment ass =
				m_rpsset.rpsset.at(GetTimFromAid(aid))->GetRawAssigmentObj(
						raw_index);
		currentRAW_start += (500 + slotDurationCount * 120) * slotNum;
		slotDurationCount = ass.GetSlotDurationCount();
		slotNum = ass.GetSlotNum();
		Time slotDuration = MicroSeconds(500 + slotDurationCount * 120);
		lastRawDurationus += slotDuration * slotNum;
		if (ass.GetRawGroupAIDStart() <= aid
				&& aid <= ass.GetRawGroupAIDEnd()) {
			uint16_t statRawSlot = (aid & 0x03ff) % slotNum;
			Time start = MicroSeconds(
					(500 + slotDurationCount * 120) * statRawSlot
							+ currentRAW_start);
			NS_LOG_DEBUG(
					"[aid=" << aid << "] is located in RAW " << (int)raw_index + 1 << " in slot " << statRawSlot + 1 << ". RAW slot start time relative to the beacon = " << start.GetMicroSeconds() << " us.");
			x = 1;
			return start;
		}
	}
	// AIDs that are not assigned to any RAW group can sleep through all the RAW groups
	// For station that does not belong to anz RAW group, return the time after all RAW groups
	/*currentRAW_start += (500 + slotDurationCount * 120) * slotNum;
	 NS_LOG_DEBUG ("[aid=" << aid << "] is located outside all RAWs. It can start contending " << currentRAW_start << " us after the beacon.");*/
	NS_ASSERT(x);
	return MicroSeconds(currentRAW_start);
}

bool ApWifiMac::IsInCurrentTim(uint16_t aid) const {
	uint16_t pageIndex = (aid >> 11) & 0x03;
	uint16_t block = (aid >> 6) & 0x1F;
	uint16_t subblock = (aid >> 3) & 0x07;
	uint16_t stationIndex = aid & 0x07;
	if (pageIndex == m_pageslice.GetPageindex()) {

	}

	return false;
}

uint16_t ApWifiMac::GetTimFromAid(uint16_t aid) const {
	if (m_pageslice.GetPageSliceLen() == 1
			&& m_pageslice.GetPageSliceCount() == 0) {
		// There is only one TIM and all stations are included in that TIM.
		return 0;
	} else {
		uint8_t block = (aid >> 6) & 0x001f;
		NS_ASSERT(block >= m_pageslice.GetBlockOffset());
		uint8_t toTim = 0;

		for (uint32_t i = 0; i < m_pageslice.GetPageSliceCount(); i++) {
			if (i == m_pageslice.GetPageSliceCount() - 1) {
				//last page slice
				if (i * m_pageslice.GetPageSliceLen() <= block && block <= 31)
					toTim = i;
			} else {
				if (i * m_pageslice.GetPageSliceLen() <= block
						&& block < (i + 1) * m_pageslice.GetPageSliceLen()) {
					if (i == 0)
						continue;
					toTim++;
				}
			}

		}
		return toTim;
	}
}

void ApWifiMac::Enqueue(Ptr<const Packet> packet, Mac48Address to, Mac48Address from)
{
	NS_LOG_FUNCTION(this << packet << to << from);
	if (to.IsBroadcast() || m_stationManager->IsAssociated(to))
	{
		/*bool schedulePacketForNextSlot = true;
		Time timeRemaining = Time(0);
		bool inSlot = false;

		uint32_t aid = 0;
		do
		{
			aid++;
		}
		while (m_AidToMacAddr.find(aid)->second != to); //TODO optimize search

		//uint32_t timGroup = aid;
		if (!m_alwaysScheduleForNextSlot && staIsActiveDuringCurrentCycle[aid - 1])
		{
			//std::cout << "sta aid=" << aid << " is active during current cycle." << std::endl;
			// station is active in its respective slot until at least the next DTIM beacon is sent
			// calculate if we are still inside the appropriate slot and transmit immediately if we are
			// that way we can avoid having to wait an entire cycle until the next slot comes up
			auto tim = this->GetTimFromAid(aid);
			if (tim == this->m_DTIMCount)
			{
				// we're still in the correct TIM group, let's see if we're still inside the slot
				Time currentOffsetSinceLastBeacon = (Simulator::Now() - this->m_lastBeaconTime);
				std::cout << "-------last beacon time======= " << this->m_lastBeaconTime.GetMicroSeconds() << " us" << std::endl;
				if (currentOffsetSinceLastBeacon >= this->GetSlotStartTimeFromAid(aid) + m_bufferTimeToAllowBeaconToBeReceived && currentOffsetSinceLastBeacon <= this->GetSlotStartTimeFromAid(aid) + this->GetSlotDurationFromAid(aid) + m_bufferTimeToAllowBeaconToBeReceived)
				{
					// still inside the slot too!, send packet immediately, if there is still enough time
					timeRemaining = this->GetSlotStartTimeFromAid(aid) + this->GetSlotDurationFromAid(aid) + m_bufferTimeToAllowBeaconToBeReceived - currentOffsetSinceLastBeacon;
					inSlot = true;
					std::cout << "+++++++++++++TIME REMAINING======= "
							<< timeRemaining.GetMicroSeconds() << "us, NOW="
							<< Simulator::Now().GetMicroSeconds() << std::endl;
					if (timeRemaining > m_scheduleTransmissionForNextSlotIfLessThan)
					{
						schedulePacketForNextSlot = false;
						NS_LOG_DEBUG(Simulator::Now().GetMicroSeconds() << " Data for [" << aid << "] is transmitted immediately because AP can still get it out during the STA slot, in which the STA is actively listening, there's " << timeRemaining.GetMicroSeconds() << "µs remaining until slot is over");
					}
					else
					{
						//std::cout << "AP can't send the transmission directly, not enough time left (" << timeRemaining.GetMicroSeconds() << "µs while " << m_scheduleTransmissionForNextSlotIfLessThan.GetMicroSeconds() << " was required " << std::endl;
					}
				}

			}

		}
		m_packetToTransmitReceivedFromUpperLayer(packet, to, schedulePacketForNextSlot, inSlot, timeRemaining);

		if (schedulePacketForNextSlot)
		{

		}
		*/
			ForwardDown(packet, from, to);

	}
}

void ApWifiMac::Enqueue(Ptr<const Packet> packet, Mac48Address to) {
	NS_LOG_FUNCTION(this << packet << to);
	//We're sending this packet with a from address that is our own. We
	//get that address from the lower MAC and make use of the
	//from-spoofing Enqueue() method to avoid duplicated code.
	Enqueue(packet, to, m_low->GetAddress());
}

bool ApWifiMac::SupportsSendFrom(void) const {
	NS_LOG_FUNCTION(this);
	return true;
}

SupportedRates ApWifiMac::GetSupportedRates(void) const {
	NS_LOG_FUNCTION(this);
	SupportedRates rates;
	//If it is an HT-AP then add the BSSMembershipSelectorSet
	//which only includes 127 for HT now. The standard says that the BSSMembershipSelectorSet
	//must have its MSB set to 1 (must be treated as a Basic Rate)
	//Also the standard mentioned that at leat 1 element should be included in the SupportedRates the rest can be in the ExtendedSupportedRates
	if (m_htSupported) {
		for (uint32_t i = 0; i < m_phy->GetNBssMembershipSelectors(); i++) {
			//NS_LOG_UNCOND (GetAddress () << " GetSupportedRates ");
			rates.SetBasicRate(m_phy->GetBssMembershipSelector(i)); //not sure it's needed
		}
	}
	//Send the set of supported rates and make sure that we indicate
	//the Basic Rate set in this set of supported rates.
	// NS_LOG_LOGIC ("ApWifiMac::GetSupportedRates  1 " ); //for test
	for (uint32_t i = 0; i < m_phy->GetNModes(); i++) {
		WifiMode mode = m_phy->GetMode(i);
		rates.AddSupportedRate(mode.GetDataRate());
		//Add rates that are part of the BSSBasicRateSet (manufacturer dependent!)
		//here we choose to add the mandatory rates to the BSSBasicRateSet,
		//exept for 802.11b where we assume that only the two lowest mandatory rates are part of the BSSBasicRateSet
		if (mode.IsMandatory()
				&& ((mode.GetModulationClass() != WIFI_MOD_CLASS_DSSS)
						|| mode == WifiPhy::GetDsssRate1Mbps()
						|| mode == WifiPhy::GetDsssRate2Mbps())) {
			m_stationManager->AddBasicMode(mode);
		}
	}
	// NS_LOG_LOGIC ("ApWifiMac::GetSupportedRates  2 " ); //for test
	//set the basic rates
	for (uint32_t j = 0; j < m_stationManager->GetNBasicModes(); j++) {
		WifiMode mode = m_stationManager->GetBasicMode(j);
		rates.SetBasicRate(mode.GetDataRate());
	}
	//NS_LOG_LOGIC ("ApWifiMac::GetSupportedRates   " ); //for test
	return rates;
}

HtCapabilities ApWifiMac::GetHtCapabilities(void) const {
	HtCapabilities capabilities;
	capabilities.SetHtSupported(1);
	capabilities.SetLdpc(m_phy->GetLdpc());
	capabilities.SetShortGuardInterval20(m_phy->GetGuardInterval());
	capabilities.SetGreenfield(m_phy->GetGreenfield());
	for (uint8_t i = 0; i < m_phy->GetNMcs(); i++) {
		capabilities.SetRxMcsBitmask(m_phy->GetMcs(i));
	}
	return capabilities;
}

S1gCapabilities ApWifiMac::GetS1gCapabilities(void) const {
	S1gCapabilities capabilities;
	capabilities.SetS1gSupported(1);
	//capabilities.SetStaType (GetStaType ()); //do not need for AP
	capabilities.SetChannelWidth(GetChannelWidth());

	return capabilities;
}

void ApWifiMac::SendProbeResp(Mac48Address to) {
	NS_LOG_FUNCTION(this << to);
	WifiMacHeader hdr;
	hdr.SetProbeResp();
	hdr.SetAddr1(to);
	hdr.SetAddr2(GetAddress());
	hdr.SetAddr3(GetAddress());
	hdr.SetDsNotFrom();
	hdr.SetDsNotTo();
	Ptr<Packet> packet = Create<Packet>();
	MgtProbeResponseHeader probe;
	probe.SetSsid(GetSsid());
	probe.SetSupportedRates(GetSupportedRates());
	probe.SetBeaconIntervalUs(m_beaconInterval.GetMicroSeconds());
	if (m_htSupported) {
		probe.SetHtCapabilities(GetHtCapabilities());
		hdr.SetNoOrder();
	}
	packet->AddHeader(probe);

	//The standard is not clear on the correct queue for management
	//frames if we are a QoS AP. The approach taken here is to always
	//use the DCF for these regardless of whether we have a QoS
	//association or not.
	m_dca->Queue(packet, hdr);
}

void ApWifiMac::SendAssocResp(Mac48Address to, bool success, uint8_t staType,
		bool makeAdditionalAid) {
	NS_LOG_FUNCTION(this << to << success);
	WifiMacHeader hdr;
	hdr.SetAssocResp();
	hdr.SetAddr1(to);
	hdr.SetAddr2(GetAddress());
	hdr.SetAddr3(GetAddress());
	hdr.SetDsNotFrom();
	hdr.SetDsNotTo();
	Ptr<Packet> packet = Create<Packet>();
	MgtAssocResponseHeader assoc;

	uint8_t mac[6];
	to.CopyTo(mac);
	uint16_t aid;
	if (!makeAdditionalAid) {
		uint8_t aid_l = mac[5];
		uint8_t aid_h = mac[4] & 0x1f;
		aid = (aid_h << 8) | (aid_l << 0); //assign mac address as AID
		assoc.SetAID(aid); //
	} else {
		aid = (m_AidToMacAddr.end())->first + 1;
		assoc.SetAID(aid);
	}
	m_AidToMacAddr.insert(std::pair<uint16_t, Mac48Address>(aid, to));

	StatusCode code;
	if (success) {
		code.SetSuccess();
	} else {
		code.SetFailure();
	}
	assoc.SetSupportedRates(GetSupportedRates());
	assoc.SetStatusCode(code);

	if (m_htSupported) {
		assoc.SetHtCapabilities(GetHtCapabilities());
		hdr.SetNoOrder();
	}
	//NS_LOG_UNCOND ("ApWifiMac::SendAssocResp =" );

	if (m_s1gSupported && success) {
		assoc.SetS1gCapabilities(GetS1gCapabilities());
		//assign AID based on station type, to do.
		if (staType == 1) {
			for (std::vector<uint16_t>::iterator it = m_sensorList.begin();
					it != m_sensorList.end(); it++) {
				if (*it == aid)
					goto Addheader;
			}
			m_sensorList.push_back(aid);
			NS_LOG_INFO("m_sensorList =" << m_sensorList.size ());

		} else if (staType == 2) {
			for (std::vector<uint16_t>::iterator it = m_OffloadList.begin();
					it != m_OffloadList.end(); it++) {
				if (*it == aid)
					goto Addheader;
			}
			m_OffloadList.push_back(aid);
			NS_LOG_INFO("m_OffloadList =" << m_OffloadList.size ());
		}
	}
	Addheader: packet->AddHeader(assoc);

	//The standard is not clear on the correct queue for management
	//frames if we are a QoS AP. The approach taken here is to always
	//use the DCF for these regardless of whether we have a QoS
	//association or not.
	m_dca->Queue(packet, hdr);
}

//For now, to avoid adjust pageslicecount and pageslicecount dynamicly,   page bitmap is always 4 bytes
uint32_t ApWifiMac::HasPacketsToPage(uint8_t blockstart, uint8_t Page) {
	uint8_t blockBitmap;
	uint32_t PageBitmap;
	PageBitmap = 0;
	uint32_t numBlocks;
	if (m_pageslice.GetPageSliceCount() == 0)
		numBlocks = 31;
	else
		numBlocks = m_pageslice.GetPageSliceLen();

	//printf("		ApWifiMac::HasPacketsToPage --- Page Bitmap includes blocks from %d to %d\n", blockstart, blockstart + numBlocks - 1);
	for (uint32_t i = blockstart; i < blockstart + numBlocks; i++) {
		blockBitmap = HasPacketsToBlock(i, Page);
		if (blockBitmap != 0) {
			PageBitmap = PageBitmap | (1 << i);
		}
		//printf("		ApWifiMac::HasPacketsToPage --- Block Bitmap = %x\n", blockBitmap);
	}
	//printf("		ApWifiMac::HasPacketsToPage --- Page Bitmap before >> blockstart = %x\n", PageBitmap);
	PageBitmap = PageBitmap >> blockstart;
	//printf("		ApWifiMac::HasPacketsToPage --- Page Bitmap after >> blockstart = %x\n", PageBitmap);
	return PageBitmap;
}

uint8_t ApWifiMac::HasPacketsToBlock(uint16_t blockInd, uint16_t PageInd) {
	uint16_t sta_aid, subblock, block;
	uint8_t blockBitmap;

	blockBitmap = 0;
	block = (PageInd << 11) | (blockInd << 6); // TODO check

	for (uint16_t i = 0; i <= 7; i++) //8 subblock in each block.
			{
		subblock = block | (i << 3);
		for (uint16_t j = 0; j <= 7; j++) //8 stations in each subblock
				{
			sta_aid = subblock | j;
			if (m_stationManager->IsAssociated(
					m_AidToMacAddr.find(sta_aid)->second)) {
				if (HasPacketsInQueueTo(m_AidToMacAddr.find(sta_aid)->second)) {
					blockBitmap = blockBitmap | (1 << i);
					NS_LOG_DEBUG("[aid=" << sta_aid << "] " << "paged");
					staIsActiveDuringCurrentCycle[sta_aid - 1] = true;
					// if there is at least one station associated with AP that has FALSE for PageSlicingImplemented within this page then m_PageSliceNum = 31
					if (!m_supportPageSlicingList.at(m_AidToMacAddr[sta_aid]))
						m_PageSliceNum = 31;
					break;
				} else {
					staIsActiveDuringCurrentCycle[sta_aid - 1] = false;
				}
			}
		}
	}

	return blockBitmap;
}

uint8_t ApWifiMac::HasPacketsToSubBlock(uint16_t subblockInd, uint16_t blockInd,
		uint16_t PageInd) {
	uint16_t sta_aid, subblock;
	uint8_t subblockBitmap;
	subblockBitmap = 0;

	subblock = (PageInd << 11) | (blockInd << 6) | (subblockInd << 3);
	for (uint16_t j = 0; j <= 7; j++) //8 stations in each subblock
			{
		sta_aid = subblock | j;
		if (m_stationManager->IsAssociated(
				m_AidToMacAddr.find(sta_aid)->second)) {
			if (HasPacketsInQueueTo(m_AidToMacAddr.find(sta_aid)->second)) {
				subblockBitmap = subblockBitmap | (1 << j);
				//m_sleepList[m_AidToMacAddr.find(sta_aid)->second] = false;
			}
		}
	}
	return subblockBitmap;
}

bool ApWifiMac::HasPacketsInQueueTo(Mac48Address dest) {
	NS_LOG_FUNCTION(this);
	//check also if ack received
	Ptr<const Packet> peekedPacket_VO, peekedPacket_VI, peekedPacket_BE,
			peekedPacket_BK;
	WifiMacHeader peekedHdr;
	Time tstamp;

	uint32_t aid = 0;
	do {
		aid++;
	} while (m_AidToMacAddr.find(aid)->second != dest); //TODO optimize search

	std::vector<uint32_t> allTargetSlots = GetAllSlotNumbersFromAid (aid);
	for (auto targetSlot : allTargetSlots)
	{
		peekedPacket_VO =
				m_rawSlotsEdca[targetSlot].find(AC_VO)->second->GetEdcaQueue()->PeekByAddress(
						WifiMacHeader::ADDR1, dest);
		peekedPacket_VI =
				m_rawSlotsEdca[targetSlot].find(AC_VI)->second->GetEdcaQueue()->PeekByAddress(
						WifiMacHeader::ADDR1, dest);
		peekedPacket_BE =
				m_rawSlotsEdca[targetSlot].find(AC_BE)->second->GetEdcaQueue()->PeekByAddress(
						WifiMacHeader::ADDR1, dest);
		peekedPacket_BK =
				m_rawSlotsEdca[targetSlot].find(AC_BK)->second->GetEdcaQueue()->PeekByAddress(
						WifiMacHeader::ADDR1, dest);
		if (peekedPacket_VO != 0 || peekedPacket_VI != 0 || peekedPacket_BE != 0 || peekedPacket_BK != 0)
			return true;
		//NS_LOG_UNCOND("TRUE");
	}
	/*peekedPacket_VO = m_edca.find(AC_VO)->second->GetEdcaQueue()->PeekByAddress (WifiMacHeader::ADDR1, dest);
	 peekedPacket_VI = m_edca.find(AC_VI)->second->GetEdcaQueue()->PeekByAddress (WifiMacHeader::ADDR1, dest);
	 peekedPacket_BE = m_edca.find(AC_BE)->second->GetEdcaQueue()->PeekByAddress (WifiMacHeader::ADDR1, dest);
	 peekedPacket_BK = m_edca.find(AC_BK)->second->GetEdcaQueue()->PeekByAddress (WifiMacHeader::ADDR1, dest);
	 */
	/*if (peekedPacket_VO != 0 || peekedPacket_VI != 0 || peekedPacket_BE != 0 || peekedPacket_BK != 0) {
		return true;
	}*/
	//NS_LOG_UNCOND("FALSE");
	return false;

}

uint16_t ApWifiMac::RpsIndex = 0;
void ApWifiMac::SetaccessList(std::map<Mac48Address, bool> list) {
	NS_LOG_FUNCTION(this);
	Mac48Address stasAddr;

	if (list.size() == 0) {
		return;
	}

	uint32_t aid = 0;
	for (auto& pair : list) {
		stasAddr = pair.first;
		for (int k = 1; k <= m_totalStaNum; k++) {
			if (stasAddr == m_AidToMacAddr.find(k)->second) {
				aid = m_AidToMacAddr.find(k)->first;
				//NS_LOG_UNCOND ("aid " << aid << " stasAddr " << stasAddr);
			}
		}
		/*uint32_t targetSlot = GetNextSlotNumFromAid(aid);
		//std::cout << "________________aid=" << (int)aid << ", targetslot=" << targetSlot << std::endl;*/

		std::vector <uint32_t> allTargetSlots = GetAllSlotNumbersFromAid (aid);
		for (auto targetSlot : allTargetSlots)
		{
			m_rawSlotsEdca[targetSlot].find(AC_VO)->second->SetaccessList(list);
			m_rawSlotsEdca[targetSlot].find(AC_VI)->second->SetaccessList(list);
			m_rawSlotsEdca[targetSlot].find(AC_BE)->second->SetaccessList(list);
			m_rawSlotsEdca[targetSlot].find(AC_BK)->second->SetaccessList(list);
		}
		//NS_LOG_UNCOND ("aid " << aid << "stasAddr " << stasAddr);
		//NS_LOG_UNCOND ( "aid "  << k << ", send " << list.find(stasAddr)->second << ", at " << Simulator::Now () << ", size " << list.size ());

	}

	/*for (auto& q : m_rawSlotsEdca)
	 {
	 q.find (AC_VO)->second->SetaccessList (list);
	 q.find (AC_VI)->second->SetaccessList (list);
	 q.find (AC_BE)->second->SetaccessList (list);
	 q.find (AC_BK)->second->SetaccessList (list);
	 }*/

	/*m_edca.find (AC_VO)->second->SetaccessList (list);
	 m_edca.find (AC_VI)->second->SetaccessList (list);
	 m_edca.find (AC_BE)->second->SetaccessList (list);
	 m_edca.find (AC_BK)->second->SetaccessList (list);*/
}

void ApWifiMac::SendOneBeacon(void) {
	NS_LOG_FUNCTION(this);
	WifiMacHeader hdr;

	m_lastBeaconTime = Simulator::Now();

	if (m_s1gSupported) {
		hdr.SetS1gBeacon();
		hdr.SetAddr1(Mac48Address::GetBroadcast());
		hdr.SetAddr2(GetAddress()); // for debug, not accordance with draft, need change
		hdr.SetAddr3(GetAddress()); // for debug
		Ptr<Packet> packet = Create<Packet>();
		S1gBeaconHeader beacon;
		S1gBeaconCompatibility compatibility;
		compatibility.SetBeaconInterval(m_beaconInterval.GetMicroSeconds());
		beacon.SetBeaconCompatibility(compatibility);

		RPS *m_rps;
		if (m_updateRps) {
			m_updateRps = false;
			S1gRawCtr rawCtrl;
			RPS* newRps = new RPS;
			//rawCtrl.UpdateRAWGroupping(this->m_accessList,);



			for (auto& rps : m_rpsset.rpsset) {
				//Assumption 1: First RAW is always loop-RAW and all loop-RAWs have the same duration
				Time loopSlotDurationTotal = 2 * m_loopAids.size()
						* rps->GetRawAssigmentObj(0).GetSlotDuration();

				//Assumption 2: In intial RAW setup, all sensors are in a single RAW
				Time remainingTime = m_beaconInterval - loopSlotDurationTotal
						- MicroSeconds(5600);
				NS_ASSERT(remainingTime > Time());

				// finds number of slots total in sensors' RAWs
				uint16_t numSlots = 0;
				for (uint32_t g = 0; g < rps->GetNumberOfRawGroups(); g++) {
					auto rawg = rps->GetRawAssigmentObj(g);
					if (rawg.GetRawGroupAIDStart()
							!= rawg.GetRawGroupAIDEnd()) {
						numSlots += rawg.GetSlotNum();
					}
				}
				uint16_t sensorCount = (remainingTime.GetMicroSeconds()
						/ numSlots - 500) / 120;
				RPS::RawAssignment *newRaw = new RPS::RawAssignment;
				newRaw->SetRawControl(
						rps->GetRawAssigmentObj(0).GetRawControl());
				newRaw->SetSlotCrossBoundary(
						rps->GetRawAssigmentObj(0).GetSlotCrossBoundary());
				newRaw->SetSlotFormat(
						rps->GetRawAssigmentObj(0).GetSlotFormat());
				uint32_t sensor_aid_start =
						rps->GetRawAssigmentObj(1).GetRawGroupAIDStart();
				uint32_t sensor_aid_end =
						rps->GetRawAssigmentObj(1).GetRawGroupAIDEnd();
				uint32_t numSensors = sensor_aid_end - sensor_aid_start + 1;
				for (uint32_t t = 0; t < 2; t++) {
					// loops
					for (std::vector<uint32_t>& aidVec : m_loopAids) {
						newRaw->SetSlotDurationCount(
								rps->GetRawAssigmentObj(0).GetSlotDurationCount()); //in initial RAW config 0th was loop
						newRaw->SetSlotNum(1);
						uint32_t page =
								rps->GetRawAssigmentObj(0).GetRawGroupPage();
						uint32_t aid_start = aidVec[t];
						uint32_t aid_end = aidVec[t];
						uint32_t rawinfo = (aid_end << 13) | (aid_start << 2)
								| page;
						newRaw->SetRawGroup(rawinfo);

						std::cout << "*** Loop aid: " << aid_start << "-"
								<< aid_end << std::endl;
					}

					// sensors
					newRaw->SetSlotDurationCount(sensorCount);
					uint16_t ns = numSlots > 1 ? numSlots / 2 : numSlots;
					if (numSlots % 2 != 0 && numSlots > 1 && t > 0)
						ns++;
					newRaw->SetSlotNum(ns);
					uint32_t page =
							rps->GetRawAssigmentObj(0).GetRawGroupPage();
					uint32_t aid_start =
							t == 0 ? sensor_aid_start : numSensors / 2 + 1;
					uint32_t aid_end = t == 0 ? numSensors / 2 : sensor_aid_end;
					uint32_t rawinfo = (aid_end << 13) | (aid_start << 2)
							| page;
					newRaw->SetRawGroup(rawinfo);
					std::cout << "*** Sensor aid: " << aid_start << "-"
							<< aid_end << std::endl;
					std::cout << "         count= " << (int) sensorCount
							<< ", numslots=" << ns << std::endl;
					newRps->SetRawAssignment(*newRaw);
				}
				delete newRaw;
			}
			//std::cout << "PRIJE m_rpsset.rpsset.size()=" << m_rpsset.rpsset.size() << std::endl;
			uint32_t loopCount =
					m_rpsset.rpsset.at(0)->GetRawAssigmentObj(0).GetSlotDurationCount();
			m_rpsset.rpsset.clear();
			m_rpsset.rpsset.push_back(newRps);
			//delete newRps;
			//std::cout << "POSLIJE m_rpsset.rpsset.size()=" << m_rpsset.rpsset.size() << std::endl;
		}
		/*for (auto& rps : m_rpsset.rpsset)
		 {
		 for (int i=0; i<rps->GetNumberOfRawGroups(); i++)
		 {
		 std::cout << "RAW nr " << i << std::endl;
		 std::cout << "	control=" << (int)rps->GetRawAssigmentObj(i).GetRawControl();
		 std::cout << "	csb=" << (int)rps->GetRawAssigmentObj(i).GetSlotCrossBoundary();
		 std::cout << "	format=" << (int)rps->GetRawAssigmentObj(i).GetSlotFormat();
		 std::cout << "	count=" << (int)rps->GetRawAssigmentObj(i).GetSlotDurationCount();
		 std::cout << "	numslots=" << (int)rps->GetRawAssigmentObj(i).GetSlotNum();
		 std::cout << "	aidstart=" << (int)rps->GetRawAssigmentObj(i).GetRawGroupAIDStart();
		 std::cout << "	aidend=" << (int)rps->GetRawAssigmentObj(i).GetRawGroupAIDEnd();
		 std::cout << std::endl;
		 }
		 }*/

		if (RpsIndex < m_rpsset.rpsset.size()) {
			m_rps = m_rpsset.rpsset.at(RpsIndex);
			NS_LOG_INFO("< RpsIndex =" << RpsIndex);
			RpsIndex++;
		} else {
			m_rps = m_rpsset.rpsset.at(0);
			NS_LOG_INFO("RpsIndex =" << RpsIndex);
			RpsIndex = 1;
		}
		beacon.SetRPS(*m_rps);

		Mac48Address stasleepAddr;
		for (auto i = m_AidToMacAddr.begin(); i != m_AidToMacAddr.end(); ++i) {
			// assume all station sleep, then change some to awake state based on downlink data
			//This implementation is temporary, should be removed if ps-poll is supported

			stasleepAddr = i->second;
			if (m_stationManager->IsAssociated(stasleepAddr)) {
				//m_sleepList[stasleepAddr] = true;
			}
		}

		if (m_DTIMCount == 0 && GetPageSlicingActivated()) // TODO filter when GetPageSlicingActivated() is false
				{
			NS_LOG_DEBUG(
					"***DTIM*** starts at " << Simulator::Now().GetSeconds() << " s");
			m_pagebitmap = HasPacketsToPage(m_pageslice.GetBlockOffset(),
					m_pageslice.GetPageindex()); //TODO check set m_PageSliceNum = 31
			if (m_pagebitmap) //for now, only configure Page Bit map based on real-time traffic, other parameters configured beforehand.
				NS_LOG_DEBUG("m_pagebitmap (0-4 bytes) = " << m_pagebitmap);
			m_pageslice.SetPageBitmap(m_pagebitmap);
			//NS_LOG_DEBUG("	Page bitmap is " << (int)m_pageslice.GetPageBitmapLength() << " bytes long.");
			//For now, page bitmap is always 4 bytes
			beacon.SetpageSlice(m_pageslice);
		} else if (m_DTIMCount != 0 && GetPageSlicingActivated()) {
			NS_LOG_DEBUG(
					"***TIM" << (int)m_DTIMCount << "*** starts at " << Simulator::Now().GetSeconds() << " s");
		}

		m_DTIMPeriod = m_TIM.GetDTIMPeriod();
		m_TIM.SetDTIMCount(m_DTIMCount);
		NS_ASSERT(
				m_pageslice.GetTIMOffset() + m_pageslice.GetPageSliceCount()
						<= m_DTIMPeriod);
		m_TrafficIndicator = 0; //for group addressed MSDU/MMPDU, not supported.
		m_TIM.SetTafficIndicator(m_TrafficIndicator); //from page slice

		m_PageSliceNum = 0;
		if (m_pageslice.GetPageSliceCount() == 0) {
			m_PageSliceNum = 31;
		} else {
			if (m_DTIMCount == m_pageslice.GetTIMOffset()) //first page slice start at TIM offset
					{
				m_PageSliceNum = 0;
			} else {
				m_PageSliceNum++;
			}
			NS_ASSERT(m_PageSliceNum < m_pageslice.GetPageSliceCount()); //or do not use m_PageSliceNum when it's larger (equal) than slice count.
		}

		m_PageIndex = m_pageslice.GetPageindex();
		//m_TIM.SetPageIndex (m_PageIndex);
		uint64_t numPagedStas(0);

		for (auto it = m_supportPageSlicingList.begin();
				it != m_supportPageSlicingList.end(); ++it) {
			if (m_stationManager->IsAssociated(it->first)
					&& HasPacketsInQueueTo(it->first)) {
				numPagedStas++;
			}
		}

		//if (!m_DTIMCount && numPagedStas) NS_LOG_UNCOND ("Paged stations: " << (int)numPagedStas);
		/*if (m_pageslice.GetPageSliceCount() == 0 && numPagedStas > 0)// special case
		 {
		 if (m_pageslice.GetPageSliceLen() > 1)
		 {
		 // 32nd TIM in this DTIM can contain DL information for (1) STAs that do not support page slicing or (2)STAs that support page slicing and
		 // whose AID is within 32nd block of this page
		 m_PageSliceNum = 31;
		 }
		 else if (m_pageslice.GetPageSliceLen() == 1)
		 {
		 m_PageSliceNum = 31;
		 // Standard 10.47 page 325-326
		 }
		 }
		 else if (m_pageslice.GetPageSliceCount () == 0 && numPagedStas == 0)
		 {
		 m_PageSliceNum = 0;
		 }*/

		uint8_t NumEncodedBlock;
		if (m_PageSliceNum != (m_pageslice.GetPageSliceCount() - 1)
				&& m_PageSliceNum != 31) // convenient overflow if count==0
						{
			NumEncodedBlock = m_pageslice.GetPageSliceLen();
		} else if (m_pageslice.GetPageBitmapLength() > 0) //Last Page slice has max value of 31 (5 bits)
				{
			// PSlast = 8 * PageBitmap_length - (PScount-1) * PSlength
			if (m_pageslice.GetPageSliceCount() > 0)
				NumEncodedBlock = 0x1f
						& (8 * 4
								- (m_pageslice.GetPageSliceCount() - 1)
										* m_pageslice.GetPageSliceLen());
			else if (m_pageslice.GetPageSliceCount() == 0)
				NumEncodedBlock = 31;
			//As page bitmap of page slice element is fixed to 4 bytes for now, "m_pageslice.GetInformationFieldSize ()" is alwawys 8.
			//Section 9.4.2.193 oage slice element, Draft 802.11ah_D9.0
			NS_LOG_DEBUG(
					"Last page slice has " << (int)NumEncodedBlock << " blocks.");
		}
		m_TIM.SetPageSliceNum(m_PageSliceNum); //from page slice

		if (m_PageSliceNum == 0 || m_PageSliceNum == 31) {
			m_blockoffset = m_pageslice.GetBlockOffset();
		}

		m_TIM.m_length = 0; // every beacon can have up to NumEncodedBlock encoded blocks
		if (m_pageslice.GetPageBitmapLength()) {
			//uint8_t numBlocksToEncode = m_pageslice.GetPageBitmapLength();

			for (uint8_t i = 0; i < NumEncodedBlock; i++) {
				TIM::EncodedBlock * m_encodedBlock = new TIM::EncodedBlock;
				m_encodedBlock->SetBlockOffset(m_blockoffset & 0x1f); //TODO check
				uint8_t m_blockbitmap = HasPacketsToBlock(m_blockoffset & 0x1f,
						m_PageIndex);
				m_encodedBlock->SetBlockBitmap(m_blockbitmap);

				uint8_t subblocklength = 0;
				uint8_t * m_subblock;
				m_subblock = new uint8_t[8]; //can be released after SetPartialVBitmap
				for (uint16_t j = 0; j <= 7; j++) // at most 8 subblock
						{
					if (m_blockbitmap & (1 << j)) {
						*m_subblock = HasPacketsToSubBlock(j,
								m_blockoffset & 0x1f, m_PageIndex);
						subblocklength++;
						m_subblock++;
					}
				}
				for (uint32_t j = 0; j < subblocklength; j++)
					m_subblock--;
				m_encodedBlock->SetEncodedInfo(m_subblock, subblocklength);

				m_blockoffset++; //actually block id
				NS_ASSERT(
						m_blockoffset
								<= m_pageslice.GetBlockOffset()
										+ m_pageslice.GetInformationFieldSize()
												* 8);
				//block id cannot exceeds the max defined in the page slice  element
				m_TIM.SetPartialVBitmap(*m_encodedBlock);
				if (m_encodedBlock)
					delete m_encodedBlock;
			}

		}

		beacon.SetTIM(m_TIM);
		/* if (m_DTIMOffset == m_DTIMPeriod - 1)
		 {
		 m_DTIMOffset = 0;
		 }
		 else
		 {
		 m_DTIMOffset++;
		 }
		 */
		if (m_DTIMCount == m_DTIMPeriod - 1) {
			m_DTIMCount = 0;
		} else {
			m_DTIMCount++;
		}
		//NS_ASSERT (m_DTIMPeriod - m_DTIMCount + m_DTIMOffset == m_DTIMPeriod || (m_DTIMCount == 0 && m_DTIMOffset == 0));

		//set sleep list, temporary, removed if ps-poll supported

		/*for (auto &it : m_rawSlotsEdca) {
			it.find(AC_VO)->second->SetsleepList(m_sleepList);
			it.find(AC_VI)->second->SetsleepList(m_sleepList);
			it.find(AC_BE)->second->SetsleepList(m_sleepList);
			it.find(AC_BK)->second->SetsleepList(m_sleepList);
		}*/
		/*m_edca.find (AC_VO)->second->SetsleepList (m_sleepList);
		 m_edca.find (AC_VI)->second->SetsleepList (m_sleepList);
		 m_edca.find (AC_BE)->second->SetsleepList (m_sleepList);
		 m_edca.find (AC_BK)->second->SetsleepList (m_sleepList);*/

		AuthenticationCtrl AuthenCtrl;
		AuthenCtrl.SetControlType(false); //centralized
		Ptr<WifiMacQueue> MgtQueue = m_dca->GetQueue();
		uint32_t MgtQueueSize = MgtQueue->GetSize();
		if (MgtQueueSize < 10) {
			if (AuthenThreshold <= 950) {
				AuthenThreshold += 50;
			}
		} else {
			if (AuthenThreshold > 50) {
				AuthenThreshold -= 50;
			}
		}
		AuthenCtrl.SetThreshold(AuthenThreshold); //centralized
		beacon.SetAuthCtrl(AuthenCtrl);
		packet->AddHeader(beacon);
		m_beaconDca->Queue(packet, hdr);

		m_transmitBeaconTrace(beacon, m_rps->GetRawAssigmentObj());

		MacLowTransmissionParameters params;
		params.DisableRts();
		params.DisableAck();
		params.DisableNextData();
		Time bufferTimeToAllowBeaconToBeReceived =
				m_low->CalculateOverallTxTime(packet, &hdr, params);
		m_bufferTimeToAllowBeaconToBeReceived =
				bufferTimeToAllowBeaconToBeReceived;
		NS_LOG_DEBUG(
				"Transmission of beacon will take " << bufferTimeToAllowBeaconToBeReceived << ", delaying RAW start for that amount");

		m_rawSlotsEdca[0].find(AC_VO)->second->SetRawSlotDuration(
				m_sharedSlotDuration + m_bufferTimeToAllowBeaconToBeReceived);

		auto nRaw = m_rps->GetNumberOfRawGroups();
		currentRawGroup = (currentRawGroup + 1) % nRaw;

		uint16_t startaid;
		uint16_t endaid;
		Mac48Address stasAddr;
		//uint16_t offset;
		uint16_t statsPerSlot;
		uint16_t statRawSlot;

		//NS_LOG_UNCOND ("ap send beacon at " << Simulator::Now ());

		m_accessList.clear();
		for (uint16_t i = 1; i <= m_totalStaNum; i++) {
			// assume all station sleeps, then change some to awake state based on downlink data
			//This implementation is temporary, should be removed if ps-poll is supported
			if (m_AidToMacAddr.size() == 0) {
				break;
			}
			stasAddr = m_AidToMacAddr.find(i)->second;

			if (m_stationManager->IsAssociated(stasAddr)) {
				m_accessList[stasAddr] = false;
			}
		}
		//NS_LOG_UNCOND ("m_accessList.size " << m_accessList.size ());

		// schedule the slot start
		Time timeToSlotStart = Time();
		for (uint32_t g = 0; g < nRaw; g++) {
			if (m_AidToMacAddr.size() == 0) {
				break;
			}

			startaid = m_rps->GetRawAssigmentObj(g).GetRawGroupAIDStart();
			endaid = m_rps->GetRawAssigmentObj(g).GetRawGroupAIDEnd();

			//offset =0; // for test
			//m_slotNum=m_rps->GetRawAssigmentObj(g).GetSlotNum();
			statsPerSlot = (endaid - startaid + 1)
					/ m_rps->GetRawAssigmentObj(g).GetSlotNum();

			for (uint32_t i = 0; i < m_rps->GetRawAssigmentObj(g).GetSlotNum();
					i++) {
				for (uint32_t k = startaid; k <= endaid; k++) {

					statRawSlot = (k & 0x03ff)
							% m_rps->GetRawAssigmentObj(g).GetSlotNum(); //slot that the station k will be
							// station is in sot i
					if (statRawSlot == i) {
						stasAddr = m_AidToMacAddr.find(k)->second;
						if (m_stationManager->IsAssociated(stasAddr)) {
							m_accessList[stasAddr] = true;
						}
					}

				}
				Simulator::Schedule(
						bufferTimeToAllowBeaconToBeReceived + timeToSlotStart,
						&ApWifiMac::SetaccessList, this, m_accessList);

				Simulator::Schedule(
						bufferTimeToAllowBeaconToBeReceived + timeToSlotStart,
						&ApWifiMac::OnRAWSlotStart, this, RpsIndex, g + 1,
						i + 1);

				Time totalTimeToRawEnd =
						bufferTimeToAllowBeaconToBeReceived + timeToSlotStart
								+ MicroSeconds(
										m_rpsset.rpsset.at(RpsIndex - 1)->GetRawAssigmentObj(
												g).GetSlotDurationCount() * 120
												+ 500);
				Simulator::Schedule(totalTimeToRawEnd, &ApWifiMac::OnRAWSlotEnd,
						this, RpsIndex, g + 1, i + 1);
				timeToSlotStart +=
						MicroSeconds(
								500
										+ m_rps->GetRawAssigmentObj(g).GetSlotDurationCount()
												* 120);

				for (uint16_t i = 1; i <= m_totalStaNum; i++) {
					stasAddr = m_AidToMacAddr.find(i)->second;

					if (m_stationManager->IsAssociated(stasAddr)) {
						m_accessList[stasAddr] = false;
					}
				}
			}
		}
		//NS_LOG_UNCOND(GetAddress () << ", " << startaid << "\t" << endaid << ", at " << Simulator::Now () << ", bufferTimeToAllowBeaconToBeReceived " << bufferTimeToAllowBeaconToBeReceived);
	} else {
		m_receivedAid.clear(); //release storage
		hdr.SetBeacon();
		hdr.SetAddr1(Mac48Address::GetBroadcast());
		hdr.SetAddr2(GetAddress());
		hdr.SetAddr3(GetAddress());
		hdr.SetDsNotFrom();
		hdr.SetDsNotTo();
		Ptr<Packet> packet = Create<Packet>();
		MgtBeaconHeader beacon;
		beacon.SetSsid(GetSsid());
		beacon.SetSupportedRates(GetSupportedRates());
		beacon.SetBeaconIntervalUs(m_beaconInterval.GetMicroSeconds());
		if (m_htSupported) {
			beacon.SetHtCapabilities(GetHtCapabilities());
			hdr.SetNoOrder();
		}
		packet->AddHeader(beacon);
		//The beacon has it's own special queue, so we load it in there
		m_beaconDca->Queue(packet, hdr);
	}
	m_beaconEvent = Simulator::Schedule(m_beaconInterval,
			&ApWifiMac::SendOneBeacon, this);
}

void ApWifiMac::OnRAWSlotEnd(uint16_t rps, uint8_t rawGroup, uint8_t slot) {
	NS_LOG_FUNCTION(this);
	LOG_TRAFFIC(
			"AP RAW SLOT END FOR TIM GROUP " << int(rawGroup) << " SLOT " << int(slot));
	uint32_t targetSlot = this->GetSlotNumFromRpsRawSlot(rps, rawGroup, slot);
	//Time slotDuration = MicroSeconds(m_rpsset.rpsset.at(rps - 1)->GetRawAssigmentObj(rawGroup - 1).GetSlotDurationCount() * 120 + 500);
	//std::cout << "OnRAWSlotEnd Access DENIED to targetSlot=" << targetSlot << std::endl;

	Time totalRawDuration = Time();
	uint32_t numberOfRawGroupsInThisRps = m_rpsset.rpsset.at(
			m_rpsIndexTrace - 1)->GetNumberOfRawGroups();
	for (uint32_t i = 0; i < numberOfRawGroupsInThisRps; i++) {
		auto ass = m_rpsset.rpsset.at(m_rpsIndexTrace - 1)->GetRawAssigmentObj(
				i);
		totalRawDuration += ass.GetSlotDuration() * ass.GetSlotNum();
	}
	// TODO: When RAW start time can be arbitrary, then adjust sharedSlotDuration in between sequential RAW groups
	// RAW start time is NOT configurable now. It is hardcoded: next RAW group starts after the current one OR after the beacon
	// Therefore shared slot in between sequential RAW groups is 0
	// There can only be enough time for contention after the last RAW group in the beacon interval
	uint32_t numOfSlotsInLastGroup =
			m_rpsset.rpsset.at(m_rpsIndexTrace - 1)->GetRawAssigmentObj(
					numberOfRawGroupsInThisRps - 1).GetSlotNum();
	Time sharedSlotDuration =
			rawGroup == numberOfRawGroupsInThisRps
					&& slot == numOfSlotsInLastGroup ?
					GetBeaconInterval() - totalRawDuration : Time();

	// for shared slot after all RAW groups, its duration is reduced for the delay of the first RAW group
	// (first raw group is delayed, it starts after the beaconTxTime)
	// shared slot in between RAW groups has duration of 0, that one we do not reduce
	sharedSlotDuration =
			sharedSlotDuration > Time() ?
					sharedSlotDuration - m_bufferTimeToAllowBeaconToBeReceived :
					Time();
	NS_ASSERT(sharedSlotDuration >= 0); // if sharedSlotDuration < 0 then there is even not enough time to receive beacon, RAW config is incorrect
	m_sharedSlotDuration = sharedSlotDuration;
	bool csb = false;
	// Station cannot cross shared slot's boundary ONLY if they are not allowed to cross the boundary of the next RAW
	// If cross-slot-boundary equals 0 for the next RAW, forbid transmissions longer than sharedSlotDuration
	if (rawGroup < m_rpsset.rpsset.at(rps - 1)->GetNumberOfRawGroups()) {
		// take csb value of the next RAW group in the same RPS
		csb =
				m_rpsset.rpsset.at(rps - 1)->GetRawAssigmentObj(rawGroup).GetSlotCrossBoundary()
						== 0x01;
	} else if (rps < m_rpsset.rpsset.size()) {
		// given that this RAW group is the last one in this RPS
		// take csb value of the first RAW group in the next RPS
		csb =
				m_rpsset.rpsset.at(rps)->GetRawAssigmentObj(0).GetSlotCrossBoundary()
						== 0x01;
	} else {
		// this is the last RAW group in the last RPS
		// take csb value of the first RAW group of the first RPS
		csb =
				m_rpsset.rpsset.at(0)->GetRawAssigmentObj(0).GetSlotCrossBoundary()
						== 0x01;
	}

	if (m_qosSupported) {
		NotifyEdcaOfCsb(Simulator::Now(), m_sharedSlotDuration, csb);
		m_rawSlotsEdca[targetSlot].find(AC_VO)->second->AccessAllowedIfRaw(
				false);
		m_rawSlotsEdca[targetSlot].find(AC_VI)->second->AccessAllowedIfRaw(
				false);
		m_rawSlotsEdca[targetSlot].find(AC_BE)->second->AccessAllowedIfRaw(
				false);
		m_rawSlotsEdca[targetSlot].find(AC_BK)->second->AccessAllowedIfRaw(
				false);
		m_rawSlotsEdca[targetSlot].find(AC_VO)->second->OutsideRawStart();
		m_rawSlotsEdca[targetSlot].find(AC_VI)->second->OutsideRawStart();
		m_rawSlotsEdca[targetSlot].find(AC_BE)->second->OutsideRawStart();
		m_rawSlotsEdca[targetSlot].find(AC_BK)->second->OutsideRawStart();
	} else {
		m_rawSlotsDca[targetSlot]->AccessAllowedIfRaw(false);
		m_rawSlotsDca[targetSlot]->OutsideRawStart();
	}
}

void ApWifiMac::NotifyEdcaOfCsb(Time rawSlotStart, Time slotDuration,
		bool csb) {
	// Since the following methods are static, it doesn-t matter which member of m_rawSlotsEdca vector we use
	m_rawSlotsEdca[0].find(AC_VO)->second->SetRawStartTime(rawSlotStart);
	m_rawSlotsEdca[0].find(AC_VO)->second->SetRawSlotDuration(slotDuration);
	m_rawSlotsEdca[0].find(AC_VO)->second->SetCrossSlotBoundary(csb);
	m_rawSlotsEdca[0].find(AC_VI)->second->SetRawStartTime(rawSlotStart);
	m_rawSlotsEdca[0].find(AC_VI)->second->SetRawSlotDuration(slotDuration);
	m_rawSlotsEdca[0].find(AC_VI)->second->SetCrossSlotBoundary(csb);
	m_rawSlotsEdca[0].find(AC_BE)->second->SetRawStartTime(rawSlotStart);
	m_rawSlotsEdca[0].find(AC_BE)->second->SetRawSlotDuration(slotDuration);
	m_rawSlotsEdca[0].find(AC_BE)->second->SetCrossSlotBoundary(csb);
	m_rawSlotsEdca[0].find(AC_BK)->second->SetRawStartTime(rawSlotStart);
	m_rawSlotsEdca[0].find(AC_BK)->second->SetRawSlotDuration(slotDuration);
	m_rawSlotsEdca[0].find(AC_BK)->second->SetCrossSlotBoundary(csb);
}

void ApWifiMac::OnRAWSlotStart(uint16_t rps, uint8_t rawGroup, uint8_t slot) {
	NS_LOG_FUNCTION(this);
	LOG_TRAFFIC(
			"AP RAW SLOT START FOR RAW GROUP " << (int)rawGroup << " SLOT " << (int)slot);
	m_rpsIndexTrace = rps;
	m_rawGroupTrace = rawGroup;
	m_rawSlotTrace = slot;

	m_rawSlotStarted(rawGroup - 1, slot);

	uint32_t targetSlot = this->GetSlotNumFromRpsRawSlot(rps, rawGroup, slot);
	Time slotDuration =
			MicroSeconds(
					m_rpsset.rpsset.at(rps - 1)->GetRawAssigmentObj(
							rawGroup - 1).GetSlotDurationCount() * 120 + 500);
	bool csb =
			m_rpsset.rpsset.at(rps - 1)->GetRawAssigmentObj(rawGroup - 1).GetSlotCrossBoundary()
					== 0x01;
	//std::cout << "OnRawStart Access allowed to targetSlot=" << targetSlot << ", duration=" << slotDuration.GetMicroSeconds() << " us, csb=" << csb << std::endl;
	//std::cout << "rps=" << (int)rps-1 << ", rawGroup=" << (int)rawGroup-1 << ", slot=" << (int)slot-1 << std::endl;
	if (m_qosSupported) {
		NotifyEdcaOfCsb(Simulator::Now(), slotDuration, csb);
		m_rawSlotsEdca[targetSlot].find(AC_VO)->second->AccessAllowedIfRaw(
				true);
		m_rawSlotsEdca[targetSlot].find(AC_VI)->second->AccessAllowedIfRaw(
				true);
		m_rawSlotsEdca[targetSlot].find(AC_BE)->second->AccessAllowedIfRaw(
				true);
		m_rawSlotsEdca[targetSlot].find(AC_BK)->second->AccessAllowedIfRaw(
				true);
		m_rawSlotsEdca[targetSlot].find(AC_VO)->second->RawStart();
		m_rawSlotsEdca[targetSlot].find(AC_VI)->second->RawStart();
		m_rawSlotsEdca[targetSlot].find(AC_BE)->second->RawStart();
		m_rawSlotsEdca[targetSlot].find(AC_BK)->second->RawStart();
		//m_rawSlotsEdca[targetSlot][AC_BE]->RawStart();
	} else {
		m_rawSlotsDca[targetSlot]->AccessAllowedIfRaw(true);
		m_rawSlotsDca[targetSlot]->RawStart(slotDuration,
				m_rpsset.rpsset.at(rps - 1)->GetRawAssigmentObj(rawGroup - 1).GetSlotCrossBoundary());
	}
	//NS_LOG_UNCOND("AP RAW SLOT START FOR RAW GROUP " << std::to_string(rawGroup) << " SLOT " << std::to_string(slot));
}

void ApWifiMac::TxOk(const WifiMacHeader &hdr) {
	NS_LOG_FUNCTION(this);
	RegularWifiMac::TxOk(hdr);

	if (hdr.IsAssocResp()
			&& m_stationManager->IsWaitAssocTxOk(hdr.GetAddr1())) {
		NS_LOG_DEBUG("associated with sta=" << hdr.GetAddr1 ());
		m_stationManager->RecordGotAssocTxOk(hdr.GetAddr1());

		//virtualaid change RAW config
		std::vector<uint32_t> aidList;
		for (auto it = m_AidToMacAddr.begin(); it != m_AidToMacAddr.end();
				it++) {
			if (it->second == hdr.GetAddr1())
				aidList.push_back(it->first);
		}
		if (aidList.size() > 1) {
			m_loopAids.push_back(aidList);
			m_updateRps = true;
		}
	}
}

void ApWifiMac::TxFailed(const WifiMacHeader &hdr) {
	NS_LOG_FUNCTION(this);
	RegularWifiMac::TxFailed(hdr);

	if (hdr.IsAssocResp()
			&& m_stationManager->IsWaitAssocTxOk(hdr.GetAddr1())) {
		NS_LOG_DEBUG("assoc failed with sta=" << hdr.GetAddr1 ());
		m_stationManager->RecordGotAssocTxFailed(hdr.GetAddr1());
	}
}

void ApWifiMac::Receive(Ptr<Packet> packet, const WifiMacHeader *hdr) {
	NS_LOG_FUNCTION(this << packet << hdr);
	//uint16_t segg =  hdr->GetFrameControl (); // for test
	//NS_LOG_UNCOND ("AP waiting   " << segg); //for test
	Mac48Address from = hdr->GetAddr2();

	if (m_stationManager->IsAssociated(from)) {
		int aid = 0;
		do {
			aid++;
			//std::cout << "aid=" << aid << std::endl;
		} while (m_AidToMacAddr.find(aid)->second != from); //TODO optimize search
		staIsActiveDuringCurrentCycle[aid - 1] = true;
	}
	if (hdr->IsData()) {
		Mac48Address bssid = hdr->GetAddr1();
		if (!hdr->IsFromDs() && hdr->IsToDs() && bssid == GetAddress()
				&& m_stationManager->IsAssociated(from)) {
			Mac48Address to = hdr->GetAddr3();
			if (to == GetAddress()) {
				NS_LOG_DEBUG("frame for me from=" << from);
				if (hdr->IsQosData()) {
					if (hdr->IsQosAmsdu()) {
						NS_LOG_DEBUG(
								"Received A-MSDU from=" << from << ", size=" << packet->GetSize ());
						DeaggregateAmsduAndForward(packet, hdr);
						packet = 0;
					} else {
						ForwardUp(packet, from, bssid);
					}
				} else {
					ForwardUp(packet, from, bssid);
				}
				uint8_t mac[6];
				from.CopyTo(mac);
				uint8_t aid_l = mac[5];
				uint8_t aid_h = mac[4] & 0x1f;
				uint16_t aid = (aid_h << 8) | (aid_l << 0); //assign mac address as AID
				m_receivedAid.push_back(aid); //to change
			} else if (to.IsGroup() || m_stationManager->IsAssociated(to)) {
				NS_LOG_DEBUG("forwarding frame from=" << from << ", to=" << to);
				Ptr<Packet> copy = packet->Copy();

				//If the frame we are forwarding is of type QoS Data,
				//then we need to preserve the UP in the QoS control
				//header...
				if (hdr->IsQosData()) {
					ForwardDown(packet, from, to, hdr->GetQosTid());
				} else {
					ForwardDown(packet, from, to);
				}
				ForwardUp(copy, from, to);
			} else {
				ForwardUp(packet, from, to);
			}
		} else if (hdr->IsFromDs() && hdr->IsToDs()) {
			//this is an AP-to-AP frame
			//we ignore for now.
			NotifyRxDrop(packet, DropReason::MacAPToAPFrame);
		} else {
			//we can ignore these frames since
			//they are not targeted at the AP
			NotifyRxDrop(packet, DropReason::MacNotForAP);
			NS_LOG_UNCOND("not assciate, drop, from=" << from);
		}
		return;
	} else if (hdr->IsMgt()) {
		if (hdr->IsProbeReq()) {
			NS_ASSERT(hdr->GetAddr1().IsBroadcast());
			SendProbeResp(from);
			return;
		} else if (hdr->GetAddr1() == GetAddress()) {
			if (hdr->IsAssocReq()) {
				std::vector<uint16_t> aidList;
				bool makeAdditionalAid(false);
				if (m_stationManager->IsAssociated(from)) {

					for (auto it = m_AidToMacAddr.begin();
							it != m_AidToMacAddr.end(); it++) {
						if (it->second == from)
							aidList.push_back(it->first);
					}
					if (aidList.size() > 2)
						return;  //test, avoid repeate assoc
					else
						makeAdditionalAid = true;
				}
				//NS_LOG_LOGIC ("Received AssocReq "); // for test
				//first, verify that the the station's supported
				//rate set is compatible with our Basic Rate set
				MgtAssocRequestHeader assocReq;

				packet->RemoveHeader(assocReq);

				SupportedRates rates = assocReq.GetSupportedRates();
				bool problem = false;
				for (uint32_t i = 0; i < m_stationManager->GetNBasicModes();
						i++) {
					WifiMode mode = m_stationManager->GetBasicMode(i);
					if (!rates.IsSupportedRate(mode.GetDataRate())) {
						problem = true;
						break;
					}
				}

				if (m_htSupported) {
					//check that the STA supports all MCSs in Basic MCS Set
					HtCapabilities htcapabilities =
							assocReq.GetHtCapabilities();
					for (uint32_t i = 0; i < m_stationManager->GetNBasicMcs();
							i++) {
						uint8_t mcs = m_stationManager->GetBasicMcs(i);
						if (!htcapabilities.IsSupportedMcs(mcs)) {
							problem = true;
							break;
						}
					}

				}


				if (problem) {
					//One of the Basic Rate set mode is not
					//supported by the station. So, we return an assoc
					//response with an error status.
					SendAssocResp(hdr->GetAddr2(), false, 0, makeAdditionalAid);
				} else {
					//station supports all rates in Basic Rate Set.
					//record all its supported modes in its associated WifiRemoteStation
					for (uint32_t j = 0; j < m_phy->GetNModes(); j++) {
						WifiMode mode = m_phy->GetMode(j);
						if (rates.IsSupportedRate(mode.GetDataRate())) {
							m_stationManager->AddSupportedMode(from, mode);
						}
					}
					if (m_htSupported) {
						HtCapabilities htcapabilities =
								assocReq.GetHtCapabilities();
						m_stationManager->AddStationHtCapabilities(from,
								htcapabilities);
						for (uint32_t j = 0; j < m_phy->GetNMcs(); j++) {
							uint8_t mcs = m_phy->GetMcs(j);
							if (htcapabilities.IsSupportedMcs(mcs)) {
								m_stationManager->AddSupportedMcs(from, mcs);
							}
						}
					}

					m_stationManager->RecordWaitAssocTxOk(from);

					if (m_s1gSupported) {
						S1gCapabilities s1gcapabilities =
								assocReq.GetS1gCapabilities();
						m_stationManager->AddStationS1gCapabilities(from,
								s1gcapabilities);
						uint8_t sta_type = s1gcapabilities.GetStaType();
						bool pageSlicingSupported =
								s1gcapabilities.GetPageSlicingSupport() != 0;
						m_supportPageSlicingList[hdr->GetAddr2()] =
								pageSlicingSupported;

						if (m_aidRequestSupported)
						{
							AidRequest aidreq = assocReq.GetAidRequest();
							//std::cout << "-----aidreq.GetServiceCharacteristic()=" << (int)aidreq.GetServiceCharacteristic() << std::endl;
						}

						SendAssocResp(hdr->GetAddr2(), true, sta_type,
								makeAdditionalAid);
					} else {
						//send assoc response with success status.
						SendAssocResp(hdr->GetAddr2(), true, 0,
								makeAdditionalAid);
					}

				}
				return;
			} else if (hdr->IsDisassociation()) {
				m_stationManager->RecordDisassociated(from);
				uint8_t mac[6];
				from.CopyTo(mac);
				uint8_t aid_l = mac[5];
				uint8_t aid_h = mac[4] & 0x1f;
				uint16_t aid = (aid_h << 8) | (aid_l << 0);
				NS_LOG_UNCOND("Disassociation request from aid " << aid);

				for (std::vector<uint16_t>::iterator it = m_sensorList.begin();
						it != m_sensorList.end(); it++) {
					if (*it == aid) {
						m_sensorList.erase(it); //remove from association list
						NS_LOG_UNCOND(
								"erase aid " << aid << " by Ap from m_sensorList ");
						break;
					}
				}
				return;
			}
		}
	}

	//Invoke the receive handler of our parent class to deal with any
	//other frames. Specifically, this will handle Block Ack-related
	//Management Action frames.
	RegularWifiMac::Receive(packet, hdr);
}

void ApWifiMac::DeaggregateAmsduAndForward(Ptr<Packet> aggregatedPacket,
		const WifiMacHeader *hdr) {
	NS_LOG_FUNCTION(this << aggregatedPacket << hdr);
	MsduAggregator::DeaggregatedMsdus packets = MsduAggregator::Deaggregate(
			aggregatedPacket);

	for (MsduAggregator::DeaggregatedMsdusCI i = packets.begin();
			i != packets.end(); ++i) {
		if ((*i).second.GetDestinationAddr() == GetAddress()) {
			ForwardUp((*i).first, (*i).second.GetSourceAddr(),
					(*i).second.GetDestinationAddr());
		} else {
			Mac48Address from = (*i).second.GetSourceAddr();
			Mac48Address to = (*i).second.GetDestinationAddr();
			NS_LOG_DEBUG("forwarding QoS frame from=" << from << ", to=" << to);
			ForwardDown((*i).first, from, to, hdr->GetQosTid());
		}
	}
}

void ApWifiMac::SetupEdcaQueue(enum AcIndex ac, EdcaQueues& edcaqueues) {
	NS_LOG_FUNCTION(this << ac);

	//Our caller shouldn't be attempting to setup a queue that is
	//already configured.
	NS_ASSERT(edcaqueues.find(ac) == edcaqueues.end());

	Ptr<EdcaTxopN> edca = CreateObject<EdcaTxopN>();
	edca->SetLow(m_low);
	edca->SetManager(m_dcfManager);
	edca->SetTxMiddle(m_txMiddle);
	edca->SetTxOkCallback(MakeCallback(&ApWifiMac::TxOk, this));
	edca->SetTxFailedCallback(MakeCallback(&ApWifiMac::TxFailed, this));
	edca->SetAccessCategory(ac);

	edca->SetWifiRemoteStationManager(m_stationManager);

	edca->CompleteConfig();
	edcaqueues.insert(std::make_pair(ac, edca));

	edca->GetEdcaQueue()->TraceConnect("PacketDropped", "",
			MakeCallback(&ApWifiMac::OnQueuePacketDropped, this));
	edca->TraceConnect("Collision", "",
			MakeCallback(&ApWifiMac::OnCollision, this));
	edca->TraceConnect("TransmissionWillCrossRAWBoundary", "",
			MakeCallback(&ApWifiMac::OnTransmissionWillCrossRAWBoundary, this));
}

void ApWifiMac::DoInitialize(void) {
	NS_LOG_FUNCTION(this);
	m_beaconDca->Initialize();
	m_beaconEvent.Cancel();
	if (m_enableBeaconGeneration) {
		if (m_enableBeaconJitter) {
			int64_t jitter = m_beaconJitter->GetValue(0,
					m_beaconInterval.GetMicroSeconds());
			NS_LOG_DEBUG(
					"Scheduling initial beacon for access point " << GetAddress () << " at time " << jitter << " microseconds");
			m_beaconEvent = Simulator::Schedule(MicroSeconds(jitter),
					&ApWifiMac::SendOneBeacon, this);
		} else {
			NS_LOG_DEBUG(
					"Scheduling initial beacon for access point " << GetAddress () << " at time 0");
			m_beaconEvent = Simulator::ScheduleNow(&ApWifiMac::SendOneBeacon,
					this);
		}
	}
	/*m_edca.find(AC_VO)->second->GetEdcaQueue()->TraceConnect("PacketDropped", "", MakeCallback(&ApWifiMac::OnQueuePacketDropped, this));
	 m_edca.find(AC_VI)->second->GetEdcaQueue()->TraceConnect("PacketDropped", "", MakeCallback(&ApWifiMac::OnQueuePacketDropped, this));
	 m_edca.find(AC_BE)->second->GetEdcaQueue()->TraceConnect("PacketDropped", "", MakeCallback(&ApWifiMac::OnQueuePacketDropped, this));
	 m_edca.find(AC_BK)->second->GetEdcaQueue()->TraceConnect("PacketDropped", "", MakeCallback(&ApWifiMac::OnQueuePacketDropped, this));
	 */
	uint32_t numTim = 0;
	if (m_pageslice.GetPageSliceCount() == 0)
		numTim = 1;
	else
		numTim = m_pageslice.GetPageSliceCount();

	uint32_t totalSlots = 0;
	for (uint32_t i = 0; i < numTim; i++) {
		for (uint32_t j = 0; j < m_rpsset.rpsset.at(i)->GetNumberOfRawGroups();
				j++) {
			RPS::RawAssignment ass = m_rpsset.rpsset.at(i)->GetRawAssigmentObj(
					j);
			totalSlots += ass.GetSlotNum();
		}
	}

	m_rawSlotsDca = std::vector<Ptr<DcaTxop>>();
	for (uint32_t i = 0; i < totalSlots; i++) {

		Ptr<DcaTxop> dca = CreateObject<DcaTxop>();
		dca->SetLow(m_low);
		dca->SetManager(m_dcfManager);
		dca->SetTxMiddle(m_txMiddle);
		dca->SetTxOkCallback(MakeCallback(&ApWifiMac::TxOk, this));
		dca->SetTxFailedCallback(MakeCallback(&ApWifiMac::TxFailed, this));

		dca->SetWifiRemoteStationManager(m_stationManager);

		dca->GetQueue()->TraceConnect("PacketDropped", "",
				MakeCallback(&ApWifiMac::OnQueuePacketDropped, this));

		dca->TraceConnect("Collision", "",
				MakeCallback(&ApWifiMac::OnCollision, this));

		// ensure queues don't expire too fast
		dca->GetQueue()->SetMaxDelay(MilliSeconds(10000)); //todo hardcoded value
		dca->Initialize();
		ConfigureDcf(dca, 15, 1023, AC_BE_NQOS);
		m_rawSlotsDca.push_back(dca);

		EdcaQueues edca;
		SetupEdcaQueue(AC_VO, edca);
		SetupEdcaQueue(AC_VI, edca);
		SetupEdcaQueue(AC_BE, edca);
		SetupEdcaQueue(AC_BK, edca);
		for (EdcaQueues::iterator i = edca.begin(); i != edca.end(); ++i) {
			i->second->Initialize();
		}
		m_rawSlotsEdca.push_back(edca);
	}
	staIsActiveDuringCurrentCycle = std::vector<bool>(m_totalStaNum, false);
	RegularWifiMac::DoInitialize();
}

} //namespace ns3
