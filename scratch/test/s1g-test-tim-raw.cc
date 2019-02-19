/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "s1g-test-tim-raw.h"

NS_LOG_COMPONENT_DEFINE("s1g-wifi-network-tim-raw");

uint32_t AssocNum = 0;
int64_t AssocTime = 0;
uint32_t StaNum = 0;
NetDeviceContainer staDeviceCont;
const int MaxSta = 8000;

Configuration config;
Statistics stats;
SimulationEventManager eventManager;

class assoc_record
{
public:
	assoc_record();
	bool GetAssoc();
	void SetAssoc(std::string context, Mac48Address address);
	void UnsetAssoc(std::string context, Mac48Address address);
	void setstaid(uint16_t id);
private:
	bool assoc;
	uint16_t staid;
};

assoc_record::assoc_record() {
	assoc = false;
	staid = 65535;
}

void assoc_record::setstaid(uint16_t id) {
	staid = id;
}

void assoc_record::SetAssoc(std::string context, Mac48Address address) {
	assoc = true;
}

void assoc_record::UnsetAssoc(std::string context, Mac48Address address) {
	assoc = false;
}

bool assoc_record::GetAssoc() {
	return assoc;
}

typedef std::vector<assoc_record *> assoc_recordVector;
assoc_recordVector assoc_vector;

uint32_t GetAssocNum() {
	AssocNum = 0;
	for (assoc_recordVector::const_iterator index = assoc_vector.begin();
			index != assoc_vector.end(); index++) {
		if ((*index)->GetAssoc()) {
			AssocNum++;
		}
	}
	return AssocNum;
}

void PopulateArpCache() {
	Ptr<ArpCache> arp = CreateObject<ArpCache>();
	arp->SetAliveTimeout(Seconds(3600 * 24 * 365));
	for (NodeList::Iterator i = NodeList::Begin(); i != NodeList::End(); ++i) {
		Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol>();
		NS_ASSERT(ip != 0);
		ObjectVectorValue interfaces;
		ip->GetAttribute("InterfaceList", interfaces);
		for (ObjectVectorValue::Iterator j = interfaces.Begin();
				j != interfaces.End(); j++) {
			Ptr<Ipv4Interface> ipIface =
					(j->second)->GetObject<Ipv4Interface>();
			NS_ASSERT(ipIface != 0);
			Ptr<NetDevice> device = ipIface->GetDevice();
			NS_ASSERT(device != 0);
			Mac48Address addr = Mac48Address::ConvertFrom(device->GetAddress());
			for (uint32_t k = 0; k < ipIface->GetNAddresses(); k++) {
				Ipv4Address ipAddr = ipIface->GetAddress(k).GetLocal();
				if (ipAddr == Ipv4Address::GetLoopback())
					continue;
				ArpCache::Entry * entry = arp->Add(ipAddr);
				entry->MarkWaitReply(0);
				entry->MarkAlive(addr);
				std::cout << "Arp Cache: Adding the pair (" << addr << ","
						<< ipAddr << ")" << std::endl;
			}
		}
	}
	for (NodeList::Iterator i = NodeList::Begin(); i != NodeList::End(); ++i) {
		Ptr<Ipv4L3Protocol> ip = (*i)->GetObject<Ipv4L3Protocol>();
		NS_ASSERT(ip != 0);
		ObjectVectorValue interfaces;
		ip->GetAttribute("InterfaceList", interfaces);
		for (ObjectVectorValue::Iterator j = interfaces.Begin();
				j != interfaces.End(); j++) {
			Ptr<Ipv4Interface> ipIface =
					(j->second)->GetObject<Ipv4Interface>();
			ipIface->SetAttribute("ArpCache", PointerValue(arp));
		}
	}
}

uint16_t ngroup;
uint16_t nslot;
RPSVector configureRAW (RPSVector rpslist, string RAWConfigFile)
{
	uint16_t NRPS = 0;
	uint16_t NRAWPERBEACON = 0;
	uint16_t Value = 0;
	uint32_t page = 0;
	uint32_t aid_start = 0;
	uint32_t aid_end = 0;
	uint32_t rawinfo = 0;

	ifstream myfile(RAWConfigFile);
	//1. get info from config file

	//2. define RPS
	if (myfile.is_open())
	{
		myfile >> NRPS;
		int totalNumSta = 0;
		uint16_t prevaidstart, prevaidend;
		uint16_t biggestAid = 0;
		for (uint16_t kk = 0; kk < NRPS; kk++) // number of beacons covering all raw groups
		{
			RPS *m_rps = new RPS;
			myfile >> NRAWPERBEACON;
			ngroup = NRAWPERBEACON;
			config.NGroup = ngroup;
			for (uint16_t i = 0; i < NRAWPERBEACON; i++) // raw groups in one beacon
			{
				//RPS *m_rps = new RPS;
				RPS::RawAssignment *m_raw = new RPS::RawAssignment;

				myfile >> Value;
				m_raw->SetRawControl(Value);  //support paged STA or not
				myfile >> Value;
				m_raw->SetSlotCrossBoundary(Value);
				config.crossSlotBoundary = Value;
				myfile >> Value;
				m_raw->SetSlotFormat(Value);
				myfile >> Value;
				m_raw->SetSlotDurationCount(Value);
				myfile >> Value;
				nslot = Value;
				config.NRawSlotNum = nslot;
				m_raw->SetSlotNum(Value);
				myfile >> page;
				myfile >> aid_start;
				myfile >> aid_end;
				rawinfo = (aid_end << 13) | (aid_start << 2) | page;
				m_raw->SetRawGroup(rawinfo);
				if (biggestAid < aid_end)
					biggestAid = aid_end;
				totalNumSta += aid_end - aid_start + 1;
				m_rps->SetRawAssignment(*m_raw);
				delete m_raw;
			}
			rpslist.rpsset.push_back(m_rps);
			//config.nRawGroupsPerRpsList.push_back(NRAWPERBEACON);
		}
		myfile.close();
		config.NRawSta = biggestAid;//rpslist.rpsset[rpslist.rpsset.size()-1]->GetRawAssigmentObj(NRAWPERBEACON-1).GetRawGroupAIDEnd();
	}
	else
	{
		cerr << "Unable to open RAW configuration file \n";
		NS_ASSERT (false);
	}


	return rpslist;
}

/*
pageslice element and TIM(DTIM) together accomplish page slicing.

Prior knowledge:
802.11ah support up to 8192 stations, they are constructed into: page, block,
 subblock, sta.
there are 13 bit represent the AID of stations.
 AID[11-12] represent page.
 AID[6-10] represent block.
 AID[3-5] represent subblock.
 AID[0-2] represent sta.

A TIM(DTIM) element only support one page
A Page slice element only support one page

 Concept of page slicing:
 Between two DTIM beacon, there are many TIM beacons, only allow a TIM beacon include some blocks of one page is called page slice. One TIM beacon is called a page slice.
 Page slcie element specify number of page slice between two DTIM, number of blocks in each
 page slice.
 Page slice element only appears together with DTIM.

 Details:
 Page slice element also indicates AP has buffered data for which block, if a station is in that block, the station should first sleep, then wake up at coresponding page slice(TIM beacon) which includes that block.

 When station wake up at that block, it check whether AP has data for itself. If has, keep awake to receive packets and go to sleep in the next beacon.
 */

void configurePageSlice (void)
{
    config.pageS.SetPageindex (config.pageIndex);
    config.pageS.SetPagePeriod (config.pagePeriod); //2 TIM groups between DTIMs
    config.pageS.SetPageSliceLen (config.pageSliceLength); //each TIM group has 1 block (2 blocks in 2 TIM groups)
    config.pageS.SetPageSliceCount (config.pageSliceCount);
    config.pageS.SetBlockOffset (config.blockOffset);
    config.pageS.SetTIMOffset (config.timOffset);
    //std::cout << "pageIndex=" << (int)config.pageIndex << ", pagePeriod=" << (int)config.pagePeriod << ", pageSliceLength=" << (int)config.pageSliceLength << ", pageSliceCount=" << (int)config.pageSliceCount << ", blockOffset=" << (int)config.blockOffset << ", timOffset=" << (int)config.timOffset << std::endl;
    // page 0
    // 8 TIM(page slice) for one page
    // 4 block (each page)
    // 8 page slice
    // both offset are 0
}

void configureTIM (void)
{
    config.tim.SetPageIndex (config.pageIndex);
    if (config.pageSliceCount)
    	config.tim.SetDTIMPeriod (config.pageSliceCount); // not necessarily the same
    else
    	config.tim.SetDTIMPeriod (1);

    //std::cout << "DTIM period=" << (int)config.pagePeriod << std::endl;
}

void checkRawAndTimConfiguration (void)
{
	std::cout << "Checking RAW and TIM configuration..." << std::endl;
	bool configIsCorrect = true;
	NS_ASSERT (config.rps.rpsset.size());
	// Number of page slices in a single page has to equal number of different RPS elements because
	// If #PS > #RPS, the same RPS will be used in more than 1 PS and that is wrong because
	// each PS can accommodate different AIDs (same RPS means same stations in RAWs)
    if(config.pageSliceCount)
    {
	//NS_ASSERT (config.pagePeriod == config.rps.rpsset.size());
    }
	for (uint32_t j = 0; j < config.rps.rpsset.size(); j++)
	{
		uint32_t totalRawTime = 0;
		for (uint32_t i = 0; i < config.rps.rpsset[j]->GetNumberOfRawGroups(); i++)
		{
			totalRawTime += (120 * config.rps.rpsset[j]->GetRawAssigmentObj(i).GetSlotDurationCount() + 500) * config.rps.rpsset[j]->GetRawAssigmentObj(i).GetSlotNum();
			auto aidStart = config.rps.rpsset[j]->GetRawAssigmentObj(i).GetRawGroupAIDStart();
			auto aidEnd = config.rps.rpsset[j]->GetRawAssigmentObj(i).GetRawGroupAIDEnd();
			configIsCorrect = check (aidStart, j) && check (aidEnd, j);
			// AIDs in each RPS must comply with TIM in the following way:
			// TIM0: 1-63; TIM1: 64-127; TIM2: 128-191; ...; TIM32: 1983-2047
			// If RPS that belongs to TIM0 includes other AIDs (other than range [1-63]) configuration is incorrect
			NS_ASSERT (configIsCorrect);
		}
		NS_ASSERT (totalRawTime <= config.BeaconInterval);
	}
}
// assumes each TIM has its own beacon - doesn't need to be the case as there has to be only PageSliceCount beacons between DTIMs
bool check (uint16_t aid, uint32_t index)
{
	uint8_t block = (aid >> 6 ) & 0x001f;
	NS_ASSERT (config.pageS.GetPageSliceLen() > 0);
	//uint8_t toTim = (block - config.pageS.GetBlockOffset()) % config.pageS.GetPageSliceLen();
	if (index == config.pageS.GetPageSliceCount() - 1 && config.pageS.GetPageSliceCount() != 0)
	{
		// the last page slice has 32 - the rest blocks
		return (block <= 31) && (block >= index * config.pageS.GetPageSliceLen());
	}
	else if (config.pageS.GetPageSliceCount() == 0)
		return true;

	return (block >= index * config.pageS.GetPageSliceLen()) && (block < (index + 1) * config.pageS.GetPageSliceLen());
}


void sendStatistics(bool schedule) {
	eventManager.onUpdateStatistics(stats);
	eventManager.onUpdateSlotStatistics(
			transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval,
			transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval);
	// reset
	std::fill(transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval.begin(),
			transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval.end(), 0);
	std::fill(transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval.begin(),
			transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval.end(), 0);

	if (schedule)
		Simulator::Schedule(Seconds(config.visualizerSamplingInterval),	&sendStatistics, true);
}

void onSTADeassociated(int i) {
	eventManager.onNodeDeassociated(*nodes[i]);
}

void updateNodesQueueLength() {
	for (uint32_t i = 0; i < config.Nsta; i++) {
		nodes[i]->UpdateQueueLength();
		stats.get(i).EDCAQueueLength = nodes[i]->queueLength;
	}
	Simulator::Schedule(Seconds(0.5), &updateNodesQueueLength);
}

void onSTAAssociated(int i) {
	cout << "Node " << std::to_string(i) << " is associated and has aids:";
	for (auto aid : nodes[i]->aids)
		std::cout << "   " << aid;
	std::cout << std::endl;


	for (int k = 0; k < config.rps.rpsset.size(); k++) {
		for (int j = 0; j < config.rps.rpsset[k]->GetNumberOfRawGroups(); j++) {
			if (config.rps.rpsset[k]->GetRawAssigmentObj(j).GetRawGroupAIDStart()
					<= i + 1
					&& i + 1
							<= config.rps.rpsset[k]->GetRawAssigmentObj(j).GetRawGroupAIDEnd()) {
				nodes[i]->rpsIndex = k + 1;
				nodes[i]->rawGroupNumber = j + 1;
				nodes[i]->rawSlotIndex =
						nodes[i]->aids[nodes[i]->aids.size() - 1]
								% config.rps.rpsset[k]->GetRawAssigmentObj(j).GetSlotNum()
								+ 1;
				/*cout << "Node " << i << " with AID " << (int)nodes[i]->aId << " belongs to " << (int)nodes[i]->rawSlotIndex << " slot of RAW group "
				 << (int)nodes[i]->rawGroupNumber << " within the " << (int)nodes[i]->rpsIndex << " RPS." << endl;
				 */
			}
		}
	}

	eventManager.onNodeAssociated(*nodes[i]);

	// RPS, Raw group and RAW slot assignment

	if (GetAssocNum() == config.Nsta && nodes[i]->aids[nodes[i]->aids.size() - 1] <= config.Nsta) {
		cout << "All " << AssocNum << " stations associated at " << Simulator::Now ().GetMicroSeconds () <<", configuring clients & server" << endl;

		// association complete, start sending packets
		stats.TimeWhenEverySTAIsAssociated = Simulator::Now();

		if (config.trafficType == "udp") {
			configureUDPServer();
			configureUDPClients();
		} else if (config.trafficType == "udpecho") {
			configureUDPEchoServer();
			configureUDPEchoClients();
		} else if (config.trafficType == "tcpecho") {
			configureTCPEchoServer();
			configureTCPEchoClients();
		} else if (config.trafficType == "tcppingpong") {
			configureTCPPingPongServer();
			configureTCPPingPongClients();
		} else if (config.trafficType == "tcpipcamera") {
			configureTCPIPCameraServer();
			configureTCPIPCameraClients();
		} else if (config.trafficType == "tcpfirmware") {
			configureTCPFirmwareServer();
			configureTCPFirmwareClients();
		} else if (config.trafficType == "tcpsensor") {
			configureTCPSensorServer();
			configureTCPSensorClients();
		}
		else if (config.trafficType == "coap") {
			config.useV6 = false; //TODO enable
			if (config.nControlLoops == -1)
				config.nControlLoops = config.Nsta / 2;//  = 100;

			configureCoapServer();
			configureCoapClients();
		}
		updateNodesQueueLength();
	}
}

void RpsIndexTrace (uint16_t oldValue, uint16_t newValue)
{
	currentRps = newValue;
	//cout << "RPS: " << newValue << " at " << Simulator::Now().GetMicroSeconds() << endl;
}

void RawGroupTrace (uint8_t oldValue, uint8_t newValue)
{
	currentRawGroup = newValue;
	//cout << "	group " << std::to_string(newValue) << " at " << Simulator::Now().GetMicroSeconds() << endl;
}

void RawSlotTrace (uint8_t oldValue, uint8_t newValue)
{
	currentRawSlot = newValue;
	//cout << "		slot " << std::to_string(newValue) << " at " << Simulator::Now().GetMicroSeconds() << endl;
}

void configureNodes(NodeContainer& wifiStaNode, NetDeviceContainer& staDevice) {
	cout << "Configuring STA Node trace sources..." << endl;

	for (uint32_t i = 0; i < config.Nsta; i++) {

		cout << "Hooking up trace sources for STA " << i << endl;

		NodeEntry* n = new NodeEntry(i, &stats, wifiStaNode.Get(i),
				staDevice.Get(i));

		n->SetAssociatedCallback([ = ] {onSTAAssociated(i);});
		n->SetDeassociatedCallback([ = ] {onSTADeassociated(i);});

		nodes.push_back(n);

		if (i < config.nControlLoops)
		{
		Config::Set(
				"/NodeList/"+ std::to_string(i)
		+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/ServiceCharacteristic",
				UintegerValue(4));
		}
		else
		{
			Config::Set(
					"/NodeList/"+ std::to_string(i)
			+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/ServiceCharacteristic",
			UintegerValue(1));
		}
		// hook up Associated and Deassociated events
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/Assoc",
				MakeCallback(&NodeEntry::SetAssociation, n));
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/DeAssoc",
				MakeCallback(&NodeEntry::UnsetAssociation, n));
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/NrOfTransmissionsDuringRAWSlot",
				MakeCallback(
						&NodeEntry::OnNrOfTransmissionsDuringRAWSlotChanged,
						n));	//not implem

		//Config::Connect("/NodeList/" + std::to_string(i) + "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/S1gBeaconMissed", MakeCallback(&NodeEntry::OnS1gBeaconMissed, n));

		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/PacketDropped",
				MakeCallback(&NodeEntry::OnMacPacketDropped, n));
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/Collision",
				MakeCallback(&NodeEntry::OnCollision, n));
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/TransmissionWillCrossRAWBoundary",
				MakeCallback(&NodeEntry::OnTransmissionWillCrossRAWBoundary,
						n)); //?

		// hook up TX
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyTxBegin",
				MakeCallback(&NodeEntry::OnPhyTxBegin, n));
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyTxEnd",
				MakeCallback(&NodeEntry::OnPhyTxEnd, n));
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyTxDropWithReason",
				MakeCallback(&NodeEntry::OnPhyTxDrop, n)); //?

		// hook up RX
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxBegin",
				MakeCallback(&NodeEntry::OnPhyRxBegin, n));
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxEnd",
				MakeCallback(&NodeEntry::OnPhyRxEnd, n));
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxDropWithReason",
				MakeCallback(&NodeEntry::OnPhyRxDrop, n));

		// hook up MAC traces
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/MacTxRtsFailed",
				MakeCallback(&NodeEntry::OnMacTxRtsFailed, n)); //?
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/MacTxDataFailed",
				MakeCallback(&NodeEntry::OnMacTxDataFailed, n));
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/MacTxFinalRtsFailed",
				MakeCallback(&NodeEntry::OnMacTxFinalRtsFailed, n)); //?
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/MacTxFinalDataFailed",
				MakeCallback(&NodeEntry::OnMacTxFinalDataFailed, n)); //?

		// hook up PHY State change
		Config::Connect(
				"/NodeList/" + std::to_string(i)
						+ "/DeviceList/0/$ns3::WifiNetDevice/Phy/State/State",
				MakeCallback(&NodeEntry::OnPhyStateChange, n));

	}
}

int getBandwidth(string dataMode) {
	if (dataMode == "MCS1_0" || dataMode == "MCS1_1" || dataMode == "MCS1_2"
			|| dataMode == "MCS1_3" || dataMode == "MCS1_4"
			|| dataMode == "MCS1_5" || dataMode == "MCS1_6"
			|| dataMode == "MCS1_7" || dataMode == "MCS1_8"
			|| dataMode == "MCS1_9" || dataMode == "MCS1_10")
		return 1;

	else if (dataMode == "MCS2_0" || dataMode == "MCS2_1"
			|| dataMode == "MCS2_2" || dataMode == "MCS2_3"
			|| dataMode == "MCS2_4" || dataMode == "MCS2_5"
			|| dataMode == "MCS2_6" || dataMode == "MCS2_7"
			|| dataMode == "MCS2_8")
		return 2;

	return 0;
}

string getWifiMode(string dataMode) {
	if (dataMode == "MCS1_0")
		return "OfdmRate300KbpsBW1MHz";
	else if (dataMode == "MCS1_1")
		return "OfdmRate600KbpsBW1MHz";
	else if (dataMode == "MCS1_2")
		return "OfdmRate900KbpsBW1MHz";
	else if (dataMode == "MCS1_3")
		return "OfdmRate1_2MbpsBW1MHz";
	else if (dataMode == "MCS1_4")
		return "OfdmRate1_8MbpsBW1MHz";
	else if (dataMode == "MCS1_5")
		return "OfdmRate2_4MbpsBW1MHz";
	else if (dataMode == "MCS1_6")
		return "OfdmRate2_7MbpsBW1MHz";
	else if (dataMode == "MCS1_7")
		return "OfdmRate3MbpsBW1MHz";
	else if (dataMode == "MCS1_8")
		return "OfdmRate3_6MbpsBW1MHz";
	else if (dataMode == "MCS1_9")
		return "OfdmRate4MbpsBW1MHz";
	else if (dataMode == "MCS1_10")
		return "OfdmRate150KbpsBW1MHz";

	else if (dataMode == "MCS2_0")
		return "OfdmRate650KbpsBW2MHz";
	else if (dataMode == "MCS2_1")
		return "OfdmRate1_3MbpsBW2MHz";
	else if (dataMode == "MCS2_2")
		return "OfdmRate1_95MbpsBW2MHz";
	else if (dataMode == "MCS2_3")
		return "OfdmRate2_6MbpsBW2MHz";
	else if (dataMode == "MCS2_4")
		return "OfdmRate3_9MbpsBW2MHz";
	else if (dataMode == "MCS2_5")
		return "OfdmRate5_2MbpsBW2MHz";
	else if (dataMode == "MCS2_6")
		return "OfdmRate5_85MbpsBW2MHz";
	else if (dataMode == "MCS2_7")
		return "OfdmRate6_5MbpsBW2MHz";
	else if (dataMode == "MCS2_8")
		return "OfdmRate7_8MbpsBW2MHz";
	return "";
}

void OnAPPhyRxDrop(std::string context, Ptr<const Packet> packet,
		DropReason reason) {
	// THIS REQUIRES PACKET METADATA ENABLE!
	auto pCopy = packet->Copy();
	auto it = pCopy->BeginItem();
	while (it.HasNext()) {

		auto item = it.Next();
		Callback<ObjectBase *> constructor = item.tid.GetConstructor();

		ObjectBase *instance = constructor();
		Chunk *chunk = dynamic_cast<Chunk *>(instance);
		chunk->Deserialize(item.current);

		if (dynamic_cast<WifiMacHeader*>(chunk)) {
			WifiMacHeader* hdr = (WifiMacHeader*) chunk;

			int staId = -1;
			if (!config.useV6) {
				for (uint32_t i = 0; i < staNodeInterface.GetN(); i++) {
					if (wifiStaNode.Get(i)->GetDevice(0)->GetAddress()
							== hdr->GetAddr2()) {
						staId = i;
						break;
					}
				}
			}
			else
			{
				for (uint32_t i = 0; i < staNodeInterface6.GetN(); i++) {
					if (wifiStaNode.Get(i)->GetDevice(0)->GetAddress()
							== hdr->GetAddr2()) {
						staId = i;
						break;
					}
				}
			}
			if (staId != -1) {
				stats.get(staId).NumberOfDropsByReasonAtAP[reason]++;
			}
			delete chunk;
			break;
		}
		else
			delete chunk;
	}

}

void OnAPPacketToTransmitReceived(string context, Ptr<const Packet> packet,
		Mac48Address to, bool isScheduled, bool isDuringSlotOfSTA,
		Time timeLeftInSlot) {
	int staId = -1;
	if (!config.useV6) {
		for (uint32_t i = 0; i < staNodeInterface.GetN(); i++) {
			if (wifiStaNode.Get(i)->GetDevice(0)->GetAddress() == to) {
				staId = i;
				break;
			}
		}
	}
	else
	{
		for (uint32_t i = 0; i < staNodeInterface6.GetN(); i++) {
			if (wifiStaNode.Get(i)->GetDevice(0)->GetAddress() == to) {
				staId = i;
				break;
			}
		}
	}
	if (staId != -1) {
		if (isScheduled)
			stats.get(staId).NumberOfAPScheduledPacketForNodeInNextSlot++;
		else {
			stats.get(staId).NumberOfAPSentPacketForNodeImmediately++;
			stats.get(staId).APTotalTimeRemainingWhenSendingPacketInSameSlot +=
					timeLeftInSlot;
		}
	}
}

void onChannelTransmission(Ptr<NetDevice> senderDevice, Ptr<Packet> packet) {
	int rpsIndex = currentRps - 1;
	int rawGroup = currentRawGroup - 1;
	int slotIndex = currentRawSlot - 1;
	//cout << rpsIndex << "		" << rawGroup << "		" << slotIndex << "		" << endl;

	uint64_t iSlot = slotIndex;
	if (rpsIndex > 0)
		for (int r = rpsIndex - 1; r >= 0; r--)
			for (int g = 0; g < config.rps.rpsset[r]->GetNumberOfRawGroups(); g++)
				iSlot += config.rps.rpsset[r]->GetRawAssigmentObj(g).GetSlotNum();

	if (rawGroup > 0)
		for (int i = rawGroup - 1; i >= 0; i--)
			iSlot += config.rps.rpsset[rpsIndex]->GetRawAssigmentObj(i).GetSlotNum();

	// to calculate the throughput of beacons
	WifiMacHeader hdr;
	packet->PeekHeader(hdr);
	if (hdr.IsS1gBeacon())
		stats.TotalBeaconsSizeInBytes += packet->GetSize();

	if (rpsIndex >= 0 && rawGroup >= 0 && slotIndex >= 0)
	{
		if (senderDevice->GetAddress() == apDevice.Get(0)->GetAddress())
		{
			// from AP
			transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval[iSlot] += packet->GetSerializedSize();
		}
		else
		{
			// from STA
			transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval[iSlot] += packet->GetSerializedSize();

		}
	}
	//std::cout << "------------- packetSerializedSize = " << packet->GetSerializedSize() << std::endl;
	//std::cout << "------------- txAP[" << iSlot <<"] = " << transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval[iSlot] << std::endl;
	//std::cout << "------------- txSTA[" << iSlot <<"] = " << transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval[iSlot] << std::endl;

}

int getSTAIdFromAddress(Ipv4Address from) {
	int staId = -1;
	for (int i = 0; i < staNodeInterface.GetN(); i++) {
		if (staNodeInterface.GetAddress(i) == from) {
			staId = i;
			break;
		}
	}
	return staId;
}

int getSTAIdFromAddress(Ipv6Address from) {
	int staId = -1;
	for (uint32_t i = 0; i < staNodeInterface6.GetN(); i++) {
		if (staNodeInterface6.GetAddress(i,0) == from) {
			staId = i;
			break;
		}
	}
	return staId;
}

void udpPacketReceivedAtServer(Ptr<const Packet> packet, Address from) { //works
	//cout << "+++++++++++udpPacketReceivedAtServer" << endl;
	int staId = getSTAIdFromAddress(
			InetSocketAddress::ConvertFrom(from).GetIpv4());
	if (staId != -1)
		nodes[staId]->OnUdpPacketReceivedAtAP(packet);
	else
		cout << "*** Node could not be determined from received packet at AP "
				<< endl;
}

void tcpPacketReceivedAtServer(Ptr<const Packet> packet, Address from) {
	int staId = getSTAIdFromAddress(
			InetSocketAddress::ConvertFrom(from).GetIpv4());
	if (staId != -1)
		nodes[staId]->OnTcpPacketReceivedAtAP(packet);
	else
		cout << "*** Node could not be determined from received packet at AP "
				<< endl;
}

void tcpRetransmissionAtServer(Address to) {
	int staId = getSTAIdFromAddress(Ipv4Address::ConvertFrom(to));
	if (staId != -1)
		nodes[staId]->OnTcpRetransmissionAtAP();
	else
		cout << "*** Node could not be determined from received packet at AP "
				<< endl;
}

void tcpPacketDroppedAtServer(Address to, Ptr<Packet> packet,
		DropReason reason) {
	int staId = getSTAIdFromAddress(Ipv4Address::ConvertFrom(to));
	if (staId != -1) {
		stats.get(staId).NumberOfDropsByReasonAtAP[reason]++;
	}
}

void tcpStateChangeAtServer(TcpSocket::TcpStates_t oldState,
		TcpSocket::TcpStates_t newState, Address to) {

	int staId = getSTAIdFromAddress(
			InetSocketAddress::ConvertFrom(to).GetIpv4());
	if (staId != -1)
		nodes[staId]->OnTcpStateChangedAtAP(oldState, newState);
	else
		cout << "*** Node could not be determined from received packet at AP "
				<< endl;

	//cout << Simulator::Now().GetMicroSeconds() << " ********** TCP SERVER SOCKET STATE CHANGED FROM " << oldState << " TO " << newState << endl;
}

void tcpIPCameraDataReceivedAtServer(Address from, uint16_t nrOfBytes) {
	int staId = getSTAIdFromAddress(
			InetSocketAddress::ConvertFrom(from).GetIpv4());
	if (staId != -1)
		nodes[staId]->OnTcpIPCameraDataReceivedAtAP(nrOfBytes);
	else
		cout << "*** Node could not be determined from received packet at AP " << endl;
}

void coapPacketReceivedAtServer(Ptr<const Packet> packet, Address from) {
	int staId;
	if (!config.useV6)
		staId = getSTAIdFromAddress(InetSocketAddress::ConvertFrom(from).GetIpv4());
	else
		staId = getSTAIdFromAddress(Inet6SocketAddress::ConvertFrom(from).GetIpv6());
	if (staId != -1)
		nodes[staId]->OnCoapPacketReceivedAtServer(packet); ///ami OnCoapPacketReceivedAtAP
	else
		cout << "*** Node could not be determined from received packet at server " << endl;
}

void configureUDPServer() {
	UdpServerHelper myServer(9);
	serverApp = myServer.Install(wifiApNode);
	serverApp.Get(0)->TraceConnectWithoutContext("Rx",
			MakeCallback(&udpPacketReceivedAtServer));
	serverApp.Start(Seconds(0));

}

void configureUDPEchoServer() {
	UdpEchoServerHelper myServer(9);
	serverApp = myServer.Install(wifiApNode);
	serverApp.Get(0)->TraceConnectWithoutContext("Rx",
			MakeCallback(&udpPacketReceivedAtServer));
	serverApp.Start(Seconds(0));
}

void configureTCPEchoServer() {
	TcpEchoServerHelper myServer(80);
	serverApp = myServer.Install(wifiApNode);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
}

void configureTCPPingPongServer() {
	// TCP ping pong is a test for the new base tcp-client and tcp-server applications
	ObjectFactory factory;
	factory.SetTypeId(TCPPingPongServer::GetTypeId());
	factory.Set("Port", UintegerValue(81));

	Ptr<Application> tcpServer = factory.Create<TCPPingPongServer>();
	wifiApNode.Get(0)->AddApplication(tcpServer);

	auto serverApp = ApplicationContainer(tcpServer);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
}

void configureTCPPingPongClients() {

	ObjectFactory factory;
	factory.SetTypeId(TCPPingPongClient::GetTypeId());
	factory.Set("Interval", TimeValue(MilliSeconds(config.trafficInterval)));
	factory.Set("PacketSize", UintegerValue(config.payloadSize));

	factory.Set("RemoteAddress",
			Ipv4AddressValue(apNodeInterface.GetAddress(0)));
	factory.Set("RemotePort", UintegerValue(81));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable>();

	for (uint16_t i = 0; i < config.Nsta; i++) {

		Ptr<Application> tcpClient = factory.Create<TCPPingPongClient>();
		wifiStaNode.Get(i)->AddApplication(tcpClient);
		auto clientApp = ApplicationContainer(tcpClient);
		wireTCPClient(clientApp, i);

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0 + random));
		//clientApp.Stop(Seconds(simulationTime + 1));
	}
}

void configureTCPIPCameraServer() {
	ObjectFactory factory;
	factory.SetTypeId(TCPIPCameraServer::GetTypeId());
	factory.Set("Port", UintegerValue(82));

	Ptr<Application> tcpServer = factory.Create<TCPIPCameraServer>();
	wifiApNode.Get(0)->AddApplication(tcpServer);

	auto serverApp = ApplicationContainer(tcpServer);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
//	serverApp.Stop(Seconds(config.simulationTime));
}

void configureTCPIPCameraClients() {

	ObjectFactory factory;
	factory.SetTypeId(TCPIPCameraClient::GetTypeId());
	factory.Set("MotionPercentage",
			DoubleValue(config.ipcameraMotionPercentage));
	factory.Set("MotionDuration",
			TimeValue(Seconds(config.ipcameraMotionDuration)));
	factory.Set("DataRate", UintegerValue(config.ipcameraDataRate));

	factory.Set("PacketSize", UintegerValue(config.payloadSize));

	factory.Set("RemoteAddress",
			Ipv4AddressValue(apNodeInterface.GetAddress(0)));
	factory.Set("RemotePort", UintegerValue(82));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable>();

	for (uint16_t i = 0; i < config.Nsta; i++) {

		Ptr<Application> tcpClient = factory.Create<TCPIPCameraClient>();
		wifiStaNode.Get(i)->AddApplication(tcpClient);
		auto clientApp = ApplicationContainer(tcpClient);
		wireTCPClient(clientApp, i);

		clientApp.Start(MilliSeconds(0));
		clientApp.Stop(Seconds(config.simulationTime));
	}
}

void configureTCPFirmwareServer() {
	ObjectFactory factory;
	factory.SetTypeId(TCPFirmwareServer::GetTypeId());
	factory.Set("Port", UintegerValue(83));

	factory.Set("FirmwareSize", UintegerValue(config.firmwareSize));
	factory.Set("BlockSize", UintegerValue(config.firmwareBlockSize));
	factory.Set("NewUpdateProbability",
			DoubleValue(config.firmwareNewUpdateProbability));

	Ptr<Application> tcpServer = factory.Create<TCPFirmwareServer>();
	wifiApNode.Get(0)->AddApplication(tcpServer);

	auto serverApp = ApplicationContainer(tcpServer);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
//	serverApp.Stop(Seconds(config.simulationTime));
}

void configureTCPFirmwareClients() {

	ObjectFactory factory;
	factory.SetTypeId(TCPFirmwareClient::GetTypeId());
	factory.Set("CorruptionProbability",
			DoubleValue(config.firmwareCorruptionProbability));
	factory.Set("VersionCheckInterval",
			TimeValue(MilliSeconds(config.firmwareVersionCheckInterval)));
	factory.Set("PacketSize", UintegerValue(config.payloadSize));

	factory.Set("RemoteAddress",
			Ipv4AddressValue(apNodeInterface.GetAddress(0)));
	factory.Set("RemotePort", UintegerValue(83));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable>();

	for (uint16_t i = 0; i < config.Nsta; i++) {

		Ptr<Application> tcpClient = factory.Create<TCPFirmwareClient>();
		wifiStaNode.Get(i)->AddApplication(tcpClient);
		auto clientApp = ApplicationContainer(tcpClient);
		wireTCPClient(clientApp, i);

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0 + random));
		clientApp.Stop(Seconds(config.simulationTime));
	}
}

void configureTCPSensorServer() {
	ObjectFactory factory;
	factory.SetTypeId(TCPSensorServer::GetTypeId());
	factory.Set("Port", UintegerValue(84));

	Ptr<Application> tcpServer = factory.Create<TCPSensorServer>();
	wifiApNode.Get(0)->AddApplication(tcpServer);

	auto serverApp = ApplicationContainer(tcpServer);
	wireTCPServer(serverApp);
	serverApp.Start(Seconds(0));
//	serverApp.Stop(Seconds(config.simulationTime));
}

void configureTCPSensorClients() {

	ObjectFactory factory;
	factory.SetTypeId(TCPSensorClient::GetTypeId());

	factory.Set("Interval", TimeValue(MilliSeconds(config.trafficInterval)));
	factory.Set("PacketSize", UintegerValue(config.payloadSize));
	factory.Set("MeasurementSize", UintegerValue(config.sensorMeasurementSize));

	factory.Set("RemoteAddress",
			Ipv4AddressValue(apNodeInterface.GetAddress(0)));
	factory.Set("RemotePort", UintegerValue(84));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable>();

	for (uint16_t i = 0; i < config.Nsta; i++) {

		Ptr<Application> tcpClient = factory.Create<TCPSensorClient>();
		wifiStaNode.Get(i)->AddApplication(tcpClient);
		auto clientApp = ApplicationContainer(tcpClient);
		wireTCPClient(clientApp, i);

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0 + random));
		clientApp.Stop(Seconds(config.simulationTime));
	}
}

void wireTCPServer(ApplicationContainer serverApp) {
	serverApp.Get(0)->TraceConnectWithoutContext("Rx",
			MakeCallback(&tcpPacketReceivedAtServer));
	serverApp.Get(0)->TraceConnectWithoutContext("Retransmission",
			MakeCallback(&tcpRetransmissionAtServer));
	serverApp.Get(0)->TraceConnectWithoutContext("PacketDropped",
			MakeCallback(&tcpPacketDroppedAtServer));
	serverApp.Get(0)->TraceConnectWithoutContext("TCPStateChanged",
			MakeCallback(&tcpStateChangeAtServer));

	if (config.trafficType == "tcpipcamera") {
		serverApp.Get(0)->TraceConnectWithoutContext("DataReceived",
				MakeCallback(&tcpIPCameraDataReceivedAtServer));
	}
}

void wireTCPClient(ApplicationContainer clientApp, int i) {

	clientApp.Get(0)->TraceConnectWithoutContext("Tx",
			MakeCallback(&NodeEntry::OnTcpPacketSent, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("Rx",
			MakeCallback(&NodeEntry::OnTcpEchoPacketReceived, nodes[i]));

	clientApp.Get(0)->TraceConnectWithoutContext("CongestionWindow",
			MakeCallback(&NodeEntry::OnTcpCongestionWindowChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("RTO",
			MakeCallback(&NodeEntry::OnTcpRTOChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("RTT",
			MakeCallback(&NodeEntry::OnTcpRTTChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("SlowStartThreshold",
			MakeCallback(&NodeEntry::OnTcpSlowStartThresholdChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("EstimatedBW",
			MakeCallback(&NodeEntry::OnTcpEstimatedBWChanged, nodes[i]));

	clientApp.Get(0)->TraceConnectWithoutContext("TCPStateChanged",
			MakeCallback(&NodeEntry::OnTcpStateChanged, nodes[i]));
	clientApp.Get(0)->TraceConnectWithoutContext("Retransmission",
			MakeCallback(&NodeEntry::OnTcpRetransmission, nodes[i]));

	clientApp.Get(0)->TraceConnectWithoutContext("PacketDropped",
			MakeCallback(&NodeEntry::OnTcpPacketDropped, nodes[i]));

	if (config.trafficType == "tcpfirmware") {
		clientApp.Get(0)->TraceConnectWithoutContext("FirmwareUpdated",
				MakeCallback(&NodeEntry::OnTcpFirmwareUpdated, nodes[i]));
	} else if (config.trafficType == "tcpipcamera") {
		clientApp.Get(0)->TraceConnectWithoutContext("DataSent",
				MakeCallback(&NodeEntry::OnTcpIPCameraDataSent, nodes[i]));
		clientApp.Get(0)->TraceConnectWithoutContext("StreamStateChanged",
				MakeCallback(&NodeEntry::OnTcpIPCameraStreamStateChanged,
						nodes[i]));
	}
}

void configureTCPEchoClients() {
	TcpEchoClientHelper clientHelper(apNodeInterface.GetAddress(0), 80); //address of remote node
	clientHelper.SetAttribute("MaxPackets", UintegerValue(4294967295u));
	clientHelper.SetAttribute("Interval",
			TimeValue(MilliSeconds(config.trafficInterval)));
	//clientHelper.SetAttribute("IntervalDeviation", TimeValue(MilliSeconds(config.trafficIntervalDeviation)));
	clientHelper.SetAttribute("PacketSize", UintegerValue(config.payloadSize));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable>();

	for (uint16_t i = 0; i < config.Nsta; i++) {
		ApplicationContainer clientApp = clientHelper.Install(
				wifiStaNode.Get(i));
		wireTCPClient(clientApp, i);

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0 + random));
		clientApp.Stop(Seconds(config.simulationTime));
	}
}

void configureUDPClients() {
	//Application start time
	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable>();

	UdpClientHelper myClient(apNodeInterface.GetAddress(0), 9); //address of remote node
	myClient.SetAttribute("MaxPackets", UintegerValue(config.maxNumberOfPackets));
	myClient.SetAttribute("PacketSize", UintegerValue(config.payloadSize));
	traffic_sta.clear();
	ifstream trafficfile(config.TrafficPath);
	if (trafficfile.is_open())
	{
		uint16_t sta_id;
		float sta_traffic;
		for (uint16_t kk = 0; kk < config.Nsta; kk++) {
			trafficfile >> sta_id;
			trafficfile >> sta_traffic;
			traffic_sta.insert(std::make_pair(sta_id, sta_traffic)); //insert data
			//cout << "sta_id = " << sta_id << " sta_traffic = " << sta_traffic << "\n";
		}
		trafficfile.close();
	} else
	{
		cout << "Unable to open traffic file \n";
		NS_ASSERT (false);
	}

	double randomStart = 0.0;
	for (std::map<uint16_t, float>::iterator it = traffic_sta.begin();
			it != traffic_sta.end(); ++it) {
		std::ostringstream intervalstr;
		intervalstr << (config.payloadSize * 8) / (it->second * 1000000);
		std::string intervalsta = intervalstr.str();

		//config.trafficInterval = UintegerValue (Time (intervalsta));

		myClient.SetAttribute("Interval", TimeValue(Time(intervalsta))); // TODO add to nodeEntry and visualize
		randomStart = m_rv->GetValue(0,
				(config.payloadSize * 8) / (it->second * 1000000));
		ApplicationContainer clientApp = myClient.Install(
				wifiStaNode.Get(it->first));
		clientApp.Get(0)->TraceConnectWithoutContext("Tx",
				MakeCallback(&NodeEntry::OnUdpPacketSent, nodes[it->first]));
		clientApp.Start(Seconds(1 + randomStart));
		clientApp.Stop (Seconds (config.simulationTime)); //with this throughput is smaller
	}
	AppStartTime = Simulator::Now().GetSeconds() + 1;
	//Simulator::Stop (Seconds (config.simulationTime+1));
}

void configureUDPEchoClients() {
	UdpEchoClientHelper clientHelper(apNodeInterface.GetAddress(0), 9); //address of remote node
	clientHelper.SetAttribute("MaxPackets", UintegerValue(4294967295u));
	clientHelper.SetAttribute("Interval", TimeValue(MilliSeconds(config.trafficInterval)));
	//clientHelper.SetAttribute("IntervalDeviation", TimeValue(MilliSeconds(config.trafficIntervalDeviation)));
	clientHelper.SetAttribute("PacketSize", UintegerValue(config.payloadSize));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable>();

	for (uint16_t i = 0; i < config.Nsta; i++) {
		ApplicationContainer clientApp = clientHelper.Install(
				wifiStaNode.Get(i));
		clientApp.Get(0)->TraceConnectWithoutContext("Tx",
				MakeCallback(&NodeEntry::OnUdpPacketSent, nodes[i]));
		clientApp.Get(0)->TraceConnectWithoutContext("Rx",
				MakeCallback(&NodeEntry::OnUdpEchoPacketReceived, nodes[i]));

		double random = m_rv->GetValue(0, config.trafficInterval);
		clientApp.Start(MilliSeconds(0 + random));
		clientApp.Stop(Seconds(config.simulationTime));
	}
}

/* In control loops, odd nodes (1, 3, 5, ...) are clients
 * even nodes and zeroth node can be servers (0, 2, 4, ...)
 *
 * */
void configureCoapServer() {
	// server to be destination for sensor nodes is at the AP (UL traffic only)
	CoapServerHelper myServer(5683);
	/*for (uint32_t i = 0; i < config.nControlLoops*2; i += 2)
	{
		serverApp = myServer.Install(wifiStaNode.Get(i));
		nodes[i]->m_nodeType = NodeEntry::SERVER;
		serverApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&coapPacketReceivedAtServer));
		serverApp.Start(Seconds(0));
	}*/
	serverApp = myServer.Install(wifiApNode.Get(0));
	serverApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&coapPacketReceivedAtServer));
	serverApp.Start(Seconds(0));

	// server to be the destination for coap clients - wired server to the AP
	// Note: externalNodes.size () = 2; externalNodes[0] is coap server; externalNodes[1] is AP
	if (config.nControlLoops > 0)
	{
		CoapServerHelper clServer(5683);
		clServer.SetAttribute("ProcessingDelay", TimeValue (MilliSeconds (10)));
		for (int i = 0; i < config.nControlLoops; i++)
		{
			serverApp = clServer.Install(externalNodes.Get(i));
			serverApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&coapPacketReceivedAtServer));
			serverApp.Start(Seconds(0));
			//serverApp.Stop (Seconds (config.simulationTime));
		}
	}
}

void configureCoapClients()
{
	for (uint32_t i = 0; i < config.Nsta; i++)
	{
		if (!config.useV6)
		{
			// Configure client (cpntroller) that sends PUT control value to the server (process = sensor+actuator)
			if (/*i % 2 == 0 &&*/i < 1*config.nControlLoops) // 1 wifi link so 1* instead of 2*, and no i % 2 == 0 &&
			{
				// address of remote node n0 (server)
				/*Ptr<Ipv4> ip = wifiStaNode.Get(i)->GetObject<Ipv4>();
				Ipv4InterfaceAddress iAddr = ip->GetAddress(1,0);
				CoapClientHelper clientHelper (iAddr.GetLocal(), 5683);
				configureCoapClientHelper(clientHelper, i+1);
				nodes[i+1]->m_nodeType = NodeEntry::CLIENT;*/
				CoapClientHelper clientHelper (externalInterfaces.GetAddress(i), 5683);
				configureCoapClientHelper(clientHelper, i);
				nodes[i]->m_nodeType = NodeEntry::CLIENT;
			}
			else if (i >= 1*config.nControlLoops)// 1 wifi link so 1* instead of 2*
			{
				// SENSORS sending uplink traffic to AP
				//CoapClientHelper clientHelperDummy (externalInterfaces.GetAddress(0), 5683); //address of AP
				CoapClientHelper clientHelperDummy (apNodeInterface.GetAddress(0), 5683);
				clientHelperDummy.SetAttribute("MaxPackets", UintegerValue(config.maxNumberOfPackets)); //4294967295u
				clientHelperDummy.SetAttribute("Interval", TimeValue(MilliSeconds(config.trafficInterval)));
				clientHelperDummy.SetAttribute("IntervalDeviation", TimeValue(MilliSeconds(config.trafficInterval/10)));
				clientHelperDummy.SetAttribute("PayloadSize", UintegerValue(config.payloadSize));
				clientHelperDummy.SetAttribute("RequestMethod", UintegerValue(4)); // COAP_REQUEST_DELETE controlValue
				clientHelperDummy.SetAttribute("MessageType", UintegerValue(1)); // non-confirmable
				Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

				ApplicationContainer clientApp = clientHelperDummy.Install(wifiStaNode.Get(i));
				clientApp.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&NodeEntry::OnCoapPacketSent, nodes[i]));
				clientApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&NodeEntry::OnCoapPacketReceived, nodes[i]));
				nodes[i]->m_nodeType = NodeEntry::DUMMY;

				double random = m_rv->GetValue(0, config.trafficInterval);
				clientApp.Start(MilliSeconds(1+random));
				clientApp.Stop(Seconds(config.simulationTime));


			}

		}
		else
		{
			// Configure client (cpntroller) that sends PUT control value to the server (process = sensor+actuator)
			if (i % 2 == 0 && i < 2*config.nControlLoops)
			{
				// address of remote node n0 (server)
				Ptr<Ipv6> ip = wifiStaNode.Get(i)->GetObject<Ipv6>();
				Ipv6InterfaceAddress iAddr = ip->GetAddress(1,0);
				CoapClientHelper clientHelper (iAddr.GetAddress(), 5683);
				configureCoapClientHelper(clientHelper, i+1);
			}
			else if (i >= 2*config.nControlLoops)
			{
				// Dummy clients for network congestion send packets to some external service over AP
				CoapClientHelper clientHelperDummy (externalInterfaces6.GetAddress(0, 0), 5683); //address of AP
				//std::cout << " external node address is " << externalInterfaces.GetAddress(0) << std::endl;
				clientHelperDummy.SetAttribute("MaxPackets", UintegerValue(config.maxNumberOfPackets)); //4294967295u
				clientHelperDummy.SetAttribute("Interval", TimeValue(MilliSeconds(config.trafficInterval)));
				clientHelperDummy.SetAttribute("IntervalDeviation", TimeValue(MilliSeconds(config.trafficInterval/10)));
				clientHelperDummy.SetAttribute("PayloadSize", UintegerValue(config.payloadSize));
				clientHelperDummy.SetAttribute("RequestMethod", UintegerValue(1));
				clientHelperDummy.SetAttribute("MessageType", UintegerValue(1));
				Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

				ApplicationContainer clientApp = clientHelperDummy.Install(wifiStaNode.Get(i));
				clientApp.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&NodeEntry::OnCoapPacketSent, nodes[i]));
				clientApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&NodeEntry::OnCoapPacketReceived, nodes[i]));

				double random = m_rv->GetValue(0, 3000);
				clientApp.Start(MilliSeconds(0+random));
				clientApp.Stop(Seconds(config.simulationTime));
			}

		}
	}
}

void configureCoapClientHelper(CoapClientHelper& clientHelper, uint32_t n)
{
	clientHelper.SetAttribute("MaxPackets", UintegerValue(config.maxNumberOfPackets));
	clientHelper.SetAttribute("Interval", TimeValue(MicroSeconds(config.cycleTime)));
	clientHelper.SetAttribute("IntervalDeviation", TimeValue(MicroSeconds(0)));//MilliSeconds(config.cycleTime/10)
	clientHelper.SetAttribute("PayloadSize", UintegerValue(config.payloadSize));
	clientHelper.SetAttribute("RequestMethod", UintegerValue(3));
	clientHelper.SetAttribute("MessageType", UintegerValue(0));
	clientHelper.SetAttribute("CooldownTime", TimeValue(Seconds(config.CoolDownPeriod)));

	Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();

	ApplicationContainer clientApp = clientHelper.Install(wifiStaNode.Get(n));
	clientApp.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&NodeEntry::OnCoapPacketSent, nodes[n]));
	clientApp.Get(0)->TraceConnectWithoutContext("Rx", MakeCallback(&NodeEntry::OnCoapPacketReceived, nodes[n]));
	double random = m_rv->GetValue(0, config.cycleTime);
	clientApp.Start(MicroSeconds(0+random));//0+random //37820 //33890
	clientApp.Stop(Seconds(config.simulationTime));

}

void printStatsToFile (bool print)
{
	if (!print)
		return;
	ofstream os;
	//string addressresults = Outputpath + "/moreinfo.txt";
	string path = "./results-coap/" + config.NSSFile;
	os.open(path.c_str(), ios::out | ios::trunc);

	os << "Total simulation time ms=" << std::to_string(stats.EndApplicationTime.GetSeconds()) << endl;
	os << "Time every station associated ms=" << std::to_string(stats.TimeWhenEverySTAIsAssociated.GetMilliSeconds()) << endl;
	os << "\n";
	double sTotalDeliveredPackets (0), sTotalSentPackets(0), clTotalSentPackets(0), clTotalDeliveredPackets (0), clTotalRtPackets (0);
	double sTotalDuplicates (0), clTotalDuplicates (0);
	double sTotalCollisions (0), clTotalCollisions (0);
	//Time clTotalIpdClient (0), clTotalIpdServer (0);
	for (uint32_t i = 0; i < config.Nsta; i++)
	{
		if (nodes[i]->m_nodeType == NodeEntry::CLIENT)
		{
			clTotalSentPackets += stats.get(i).NumberOfSentPackets;
			clTotalDeliveredPackets += stats.get(i).NumberOfSuccessfulPackets;
			clTotalRtPackets += stats.get(i).NumberOfSuccessfulRoundtripPackets;
			clTotalDuplicates += stats.get(i).NumberOfDuplicatesAtClient + stats.get(i).NumberOfDuplicatesAtServer;
			clTotalCollisions += stats.get(i).NumberOfCollisions;
			//clTotalIpdClient += std::accumulate(stats.get(i).m_interPacketDelayClient.begin(), stats.get(i).m_interPacketDelayClient.end(), 0);
			//clTotalIpdServer += std::accumulate(stats.get(i).m_interPacketDelayServer.begin(), stats.get(i).m_interPacketDelayServer.end(), 0);
		}
		else if (nodes[i]->m_nodeType == NodeEntry::DUMMY)
		{
			sTotalSentPackets += stats.get(i).NumberOfSentPackets;
			sTotalDeliveredPackets += stats.get(i).NumberOfSuccessfulPackets;
			sTotalDuplicates += stats.get(i).NumberOfDuplicatesAtServer;
			sTotalCollisions += stats.get(i).NumberOfCollisions;
		}
	}
	os << "beaconsThorughput kbps=" << stats.TotalBeaconsSizeInBytes * 8 / stats.EndApplicationTime.GetSeconds() / 1000.0 << std::endl; //Kbps
	std::cout << "beaconsThorughput kbps=" << stats.TotalBeaconsSizeInBytes * 8 / stats.EndApplicationTime.GetSeconds() / 1000.0 << std::endl;
	//clTotalIpdClient = clTotalIpdClient / clTotalRtPackets;
	//clTotalIpdServer = clTotalIpdServer / clTotalRtPackets;
	double sensorThroughput = (sTotalDeliveredPackets + sTotalDuplicates) * config.payloadSize * 8 / (stats.EndApplicationTime.GetSeconds() - stats.TimeWhenEverySTAIsAssociated.GetSeconds()) / 1000.0;
	double sensorPacketLoss = -1;
	if (sTotalSentPackets > 0) sensorPacketLoss = (1 - sTotalDeliveredPackets / sTotalSentPackets) * 100;
	os << "sensorThroughput kbps=" << sensorThroughput << "\n";
	os << "sensorPacketLoss %=" << sensorPacketLoss << "\n";
	os << "sensorNumberOfCollisions=" << sTotalCollisions << "\n";
	double clThroughput = (clTotalDeliveredPackets + clTotalRtPackets + clTotalDuplicates) * config.payloadSize * 8 / (stats.EndApplicationTime.GetSeconds() - stats.TimeWhenEverySTAIsAssociated.GetSeconds()) / 1000.0;
	double clPacketLoss = -1;
		if (clTotalDeliveredPackets > 0) clPacketLoss = (clTotalSentPackets - clTotalRtPackets) / (float)(clTotalSentPackets + clTotalDeliveredPackets);
	os << "clThroughput kbps=" << clThroughput << "\n";
	os << "clPacketLoss %=" << clPacketLoss << "\n";
	os << "clNumberOfCollisions=" << clTotalCollisions << "\n";
	//os << "clAverageIpdClients ms=" << clTotalIpdClient.GetMilliSeconds() / clTotalRtPackets;
	//os << "clAverageIpdServers ms=" << clTotalIpdServer.GetMilliSeconds() / clTotalRtPackets;
	os << "cl global max Latency (C->S) µs=" << std::to_string(NodeEntry::maxLatency.GetMicroSeconds())<< "\n"; // CORRECT
	os << "cl global min Latency (C->S) µs=" << std::to_string(NodeEntry::minLatency.GetMicroSeconds())<< "\n"; // CORRECT

	for (uint32_t i = 0; i < config.Nsta; i++)
	{
		if (nodes[i]->m_nodeType == NodeEntry::CLIENT || nodes[i]->m_nodeType == NodeEntry::DUMMY)
		{
			os << "-----------------\n";
			os << "NodeType=" << std::to_string(nodes[i]->m_nodeType) << "\n";
			os << "aid=" << std::to_string(i+1) << endl;
			os << "X: " << nodes[i]->x << ", Y: " << nodes[i]->y << endl;
			os << "Tx Remaining Queue size=" << nodes[i]->queueLength << endl;
			os << "Total transmit time=" << std::to_string(stats.get(i).TotalTransmitTime.GetMilliSeconds()) << "ms" << endl;
			os << "Total receive time=" << std::to_string(stats.get(i).TotalReceiveTime.GetMilliSeconds()) << "ms" << endl;
			os << "ConsumedEnergy=" << stats.get(i).GetTotalEnergyConsumption() << " mJ\n";
			os << "Number of packets sent=" << std::to_string(stats.get(i).NumberOfSentPackets) << endl; // CORRECT
			os << "Number of packets successfuly arrived to the dst=" << std::to_string(stats.get(i).NumberOfSuccessfulPackets) << endl; //CORRECT
			os << "Number of packets dropped=" << std::to_string(stats.get(i).getNumberOfDroppedPackets()) << endl; // NOT CORRECT
			os << "Number of roundtrip packets successful=" << std::to_string(stats.get(i).NumberOfSuccessfulRoundtripPackets) << endl; //CORRECT
			os << "Number of duplicates on server (UL)=" << std::to_string(stats.get(i).NumberOfDuplicatesAtServer) << endl;
			os << "Number of duplicates on client (DL)=" << std::to_string(stats.get(i).NumberOfDuplicatesAtClient) << endl;
			os << "Average packet sent/receive time=" << std::to_string(stats.get(i).getAveragePacketSentReceiveTime()) << "ms" << std::endl; // CORRECT
			os << "Average packet roundtrip time=" << std::to_string(stats.get(i).getAveragePacketRoundTripTime(config.trafficType)) << "ms" << std::endl; //not

			os << "Avg jitter (RTT)=" << std::to_string(stats.get(i).GetAverageJitter()) << "ms" << std::endl; //CORRECT
			os << "Average inter packet delay at server µs=" << std::to_string(stats.get(i).GetAverageInterPacketDelay(stats.get(i).m_interPacketDelayServer).GetMicroSeconds()) << "\n";
			os << "Inter-packet-delay at the server standard deviation µs=" << stats.get(i).GetInterPacketDelayDeviation(stats.get(i).m_interPacketDelayServer) << "\n";
			os << "Inter-packet-delay at the server standard deviation %=" << stats.get(i).GetInterPacketDelayDeviationPercentage(stats.get(i).m_interPacketDelayServer) << "\n";
			os << "Average inter packet delay at client µs=" << std::to_string(stats.get(i).GetAverageInterPacketDelay(stats.get(i).m_interPacketDelayClient).GetMicroSeconds()) << "\n";
			os << "Inter-packet-delay at the client standard deviation µs=" << stats.get(i).GetInterPacketDelayDeviation(stats.get(i).m_interPacketDelayClient) << "\n";
			os << "Inter-packet-delay at the client standard deviation %=" << stats.get(i).GetInterPacketDelayDeviationPercentage(stats.get(i).m_interPacketDelayClient) << "\n";
			//calculate the deviation between inter packet arrival times at the server
			os << "Packet loss=" << std::to_string(stats.get(i).GetPacketLoss(config.trafficType)) << "%" << endl; //CORRECT
			if (nodes[i]->m_nodeType == NodeEntry::CLIENT)
			{
				os << "\n";
				os << "Inter-packet delay at client us:\n";
				for (std::vector<Time>::const_iterator t = stats.get(i).m_interPacketDelayClient.begin(); t != stats.get(i).m_interPacketDelayClient.end(); ++t)
					os << (*t).GetMicroSeconds()<< " ";
				os << endl;
				os << "Inter-packet delay at server:\n";
				for (std::vector<Time>::const_iterator t = stats.get(i).m_interPacketDelayServer.begin(); t != stats.get(i).m_interPacketDelayServer.end(); ++t)
					os << (*t).GetMicroSeconds()<< " ";
				os << endl;
				os << "Inter-packet delay deviation at client=" << stats.get(i).GetInterPacketDelayDeviation(stats.get(i).m_interPacketDelayClient) << "==" << stats.get(i).GetInterPacketDelayDeviationPercentage(stats.get(i).m_interPacketDelayClient) << endl;
				os << "Inter-packet delay deviation at server=" << stats.get(i).GetInterPacketDelayDeviation(stats.get(i).m_interPacketDelayServer) << "==" << stats.get(i).GetInterPacketDelayDeviationPercentage(stats.get(i).m_interPacketDelayServer) << endl;
				os << "\n";
				os << "Packet seqs:\n";
				for (map<uint32_t, Time>::const_iterator it = stats.get(i).m_sentTimeBySeqClient.begin(); it != stats.get(i).m_sentTimeBySeqClient.end(); ++it)
					os << it->first << " ";
				os << endl;
				os << "Times sent app [us]:\n";
				for (map<uint32_t, Time>::const_iterator it = stats.get(i).m_sentTimeBySeqClient.begin(); it != stats.get(i).m_sentTimeBySeqClient.end(); ++it)
					os << it->second.GetMicroSeconds() << " ";
				os << endl;
				os << "Times received app [us]:\n";
				for (map<uint32_t, Time>::const_iterator it = stats.get(i).m_receivedTimeBySeqClient.begin(); it != stats.get(i).m_receivedTimeBySeqClient.end(); ++it)
					os << it->second.GetMicroSeconds() << " ";
				os << endl;
				os << "Time diff sent-received app [ms]:\n";
				for (map<uint32_t, Time>::const_iterator it = stats.get(i).m_sentTimeBySeqClient.begin(); it != stats.get(i).m_sentTimeBySeqClient.end(); ++it)
					//if (stats.get(i).m_receivedTimeBySeqClient[it->first])
						os << stats.get(i).m_receivedTimeBySeqClient[it->first].GetMilliSeconds() - it->second.GetMilliSeconds() << " ";
					/*else
						os << "X";*/

				os << endl;
				os << "Time phy TX begin [us]=\n";
				for (map<uint64_t, Time>::const_iterator it = stats.get(i).txBeginTimeMap.begin(); it != stats.get(i).txBeginTimeMap.end(); ++it)
					os << it->second.GetMicroSeconds() << " ";
				os << endl;
				os << "Time phy TX end [us]=\n";
				for (map<uint64_t, Time>::const_iterator it = stats.get(i).txEndTimeMap.begin(); it != stats.get(i).txEndTimeMap.end(); ++it)
					os << it->second.GetMicroSeconds() << " ";
				os << endl;
				os << "Time phy RX begin [us]=\n";
				for (map<uint64_t, Time>::const_iterator it = stats.get(i).rxBeginTimeMap.begin(); it != stats.get(i).rxBeginTimeMap.end(); ++it)
					os << it->second.GetMicroSeconds() << " ";
				os << endl;
				/*os << "Time phy RX end [us]=\n";
				for (map<uint64_t, Time>::const_iterator it = stats.get(i).rxEndTimeMap.begin(); it != stats.get(i).rxEndTimeMap.end(); ++it)
					os << it->second.GetMicroSeconds() << " ";
				os << endl;*/ // These values are equal to the m_receivedTimeBySeqClient
				os << "dT =\n";
				std::cout << "dT =\n";
				for (map<uint32_t, Time>::const_iterator it = stats.get(i).m_receivedTimeBySeqClient.begin(), jt = --stats.get(i).m_receivedTimeBySeqClient.end(); it != jt; ++it)
				{
					auto dt = stats.get(i).m_sentTimeBySeqClient[it->first + 1].GetMicroSeconds() - it->second.GetMicroSeconds();
					os << dt << " ";
					std::cout << dt << " ";
				}
				os << endl;
				cout << endl;
				os << "Goodput=" << (stats.get(i).getGoodputKbit(stats.TimeWhenEverySTAIsAssociated)) << "Kbit" << endl; //CORRECT
			}
			else
			{
				os << "\n";
				os << "Goodput=" << (stats.get(i).getGoodputKbit(stats.TimeWhenEverySTAIsAssociated)) << "Kbit" << endl; //CORRECT
			}

		}
	}
	os.close();
}


void printStatistics() {


	cout << "Statistics" << endl;
	cout << "" << endl;
	cout << "    global max Latency (C->S)= " << std::to_string(NodeEntry::maxLatency.GetMicroSeconds()) << "µs" << endl; // CORRECT
	cout << "    global min Latency (C->S) = " << std::to_string(NodeEntry::minLatency.GetMicroSeconds()) << "µs" << endl; // CORRECT
	cout << "    max diference in RTT between 2 subsequent packets = " << std::to_string(NodeEntry::maxJitter.GetMicroSeconds()) << "µs" << endl;
	cout << "    min diference in RTT between 2 subsequent packets = " << std::to_string(NodeEntry::minJitter.GetMicroSeconds()) << "µs" << endl;

	for (uint32_t i = 0; i < config.Nsta; i++) {
		if (nodes[i]->m_nodeType == NodeEntry::CLIENT){
			cout << "Node " << std::to_string(i) << endl;
			cout << "X: " << nodes[i]->x << ", Y: " << nodes[i]->y << endl;
			cout << "Tx Remaining Queue size: " << nodes[i]->queueLength << endl;
			//cout << "Tcp congestion window value: " <<  std::to_string(stats.get(i).TCPCongestionWindow) << endl;
			cout << "--------------" << endl;
			cout << "Total transmit time: " << std::to_string(stats.get(i).TotalTransmitTime.GetMilliSeconds()) << "ms" << endl;
			cout << "Total receive time: " << std::to_string(stats.get(i).TotalReceiveTime.GetMilliSeconds()) << "ms" << endl;
			cout << "ConsumedEnergy=" << stats.get(i).GetTotalEnergyConsumption() << " mJ\n";
			//cout << endl;
			//cout << "Total active time: " << std::to_string(stats.get(i).TotalActiveTime.GetMilliSeconds()) << "ms" << endl;
			//cout << "Total doze time: " << std::to_string(stats.get(i).TotalDozeTime.GetMilliSeconds()) << "ms" << endl;
			//cout << "" << endl;
			//cout << "Number of transmissions: " << std::to_string(stats.get(i).NumberOfTransmissions) << endl;
			//cout << "Number of transmissions dropped: " << std::to_string(stats.get(i).NumberOfTransmissionsDropped) << endl;
			//cout << "Number of receives: " << std::to_string(stats.get(i).NumberOfReceives) << endl;
			//cout << "Number of receives dropped: " << std::to_string(stats.get(i).NumberOfReceivesDropped) << endl;
			//cout << "" << endl;
			cout << "Number of packets sent: " << std::to_string(stats.get(i).NumberOfSentPackets) << endl; // CORRECT
			cout << "Number of packets successfuly arrived to the dst: " << std::to_string(stats.get(i).NumberOfSuccessfulPackets) << endl; //CORRECT
			cout << "Number of packets dropped: " << std::to_string(stats.get(i).getNumberOfDroppedPackets()) << endl; // NOT CORRECT
			cout << "Number of roundtrip packets successful: " << std::to_string(stats.get(i).NumberOfSuccessfulRoundtripPackets) << endl; //CORRECT
			cout << "Average packet sent/receive time: " << std::to_string(stats.get(i).getAveragePacketSentReceiveTime()) << "ms" << std::endl; // CORRECT
			cout << "Average packet roundtrip time: " << std::to_string(stats.get(i).getAveragePacketRoundTripTime(config.trafficType)) << "ms" << std::endl; //not
			// Average packet roundtrip time NOT CORRECT, uracunato je send-rcv vrijeme dropped paketa a dijeljeno je samo sa brojem successfull RTT paketa
			//cout << "IP Camera Data sending rate: " << stats.get(i).getIPCameraSendingRate() << "kbps" << std::endl;
			//cout << "IP Camera Data receiving rate: " << std::to_string(stats.get(i).getIPCameraAPReceivingRate()) << "kbps" << std::endl;
			//cout << endl;
			cout << "    Jitter (RTT)= " << std::to_string(stats.get(i).GetAverageJitter()) << "ms" << std::endl; //CORRECT
			cout << "    Average inter packet delay at server is " << std::to_string(stats.get(i).GetAverageInterPacketDelay(stats.get(i).m_interPacketDelayServer).GetMicroSeconds()) << "µs" << endl;
			cout << "    Inter-packet-delay at the server standard deviation is " << stats.get(i).GetInterPacketDelayDeviation(stats.get(i).m_interPacketDelayServer) << " which is " << stats.get(i).GetInterPacketDelayDeviationPercentage(stats.get(i).m_interPacketDelayServer) << "%" <<endl;
			cout << "    Average inter packet delay at client is " << std::to_string(stats.get(i).GetAverageInterPacketDelay(stats.get(i).m_interPacketDelayClient).GetMicroSeconds()) << "µs" << endl;
			cout << "    Inter-packet-delay at the client standard deviation is " << stats.get(i).GetInterPacketDelayDeviation(stats.get(i).m_interPacketDelayClient) << " which is " << stats.get(i).GetInterPacketDelayDeviationPercentage(stats.get(i).m_interPacketDelayClient) << "%" <<endl;
			//calculate the deviation between inter packet arrival times at the server
			cout << "    Packet loss " << std::to_string(stats.get(i).GetPacketLoss(config.trafficType)) << "%" << endl; //CORRECT

			cout << endl;
			cout << "Goodput: " << (stats.get(i).getGoodputKbit(stats.TimeWhenEverySTAIsAssociated)) << "Kbit" << endl; //CORRECT
			cout << "*********************" << endl;
			/*std::vector<Time> x = stats.get(i).m_interPacketDelay;
         for (uint32_t i=0; i< x.size(); i++)
         {
                 cout << "S: " << (x[i]).GetMicroSeconds() << ", ";
         }*/
			cout << endl;
		}
		/*else if (nodes[i]->m_nodeType == NodeEntry::SERVER)
		{
			cout << "Node " << std::to_string(i) << endl;
			cout << "X: " << nodes[i]->x << ", Y: " << nodes[i]->y << endl;
			cout << "Tx Remaining Queue size: " << nodes[i]->queueLength << endl;
		}*/

		cout << "----------" << endl;
		cout << "Total simulation time: " << std::to_string(config.simulationTime) << "ms" << endl;
		cout << "Time every station associated: " << std::to_string(stats.TimeWhenEverySTAIsAssociated.GetMilliSeconds()) << "ms" << endl;
		double totalDeliveredPackets (0), totalRttPackets (0), totalSentPackets (0);
		for (uint32_t i = 0; i < config.Nsta; i++)
		{
			totalSentPackets += stats.get(i).NumberOfSentPackets;
			totalDeliveredPackets += stats.get(i).NumberOfSuccessfulPackets;
			totalRttPackets += stats.get(i).NumberOfSuccessfulRoundtripPackets;
		}
		cout << "Total packets sent = " << totalSentPackets << endl;
		cout << "Total packets delivered to dst = " << totalDeliveredPackets << endl;
		cout << "Total packets round trip completed = " << totalRttPackets << endl;

		double throughput1 = totalDeliveredPackets * config.payloadSize * 8 / (config.simulationTime + config.CoolDownPeriod - stats.TimeWhenEverySTAIsAssociated.GetSeconds()) / 1000.0;
		double throughput2 = totalRttPackets * config.payloadSize * 8 / (config.simulationTime + config.CoolDownPeriod - stats.TimeWhenEverySTAIsAssociated.GetSeconds()) / 1000.0;

		cout << "Total throughput Kbit/s=" << throughput1 + throughput2 << endl;

	}
}


Time timeTx;
Time timeRx;
Time timeIdle;
Time timeSleep;

//ns3::int64x64_t throughouputArray [MaxSta];
Time timeIdleArray[MaxSta];
Time timeRxArray[MaxSta];
Time timeTxArray[MaxSta];
Time timeSleepArray[MaxSta];
double dist[MaxSta];

//it prints the information regarding the state of the device
void PhyStateTrace(std::string context, Time start, Time duration,
		enum WifiPhy::State state) {

	/*Get the number of the node from the context*/
	/*context = "/NodeList/"+strSTA+"/DeviceList/'*'/Phy/$ns3::YansWifiPhy/State/State"*/
	unsigned first = context.find("t/");
	unsigned last = context.find("/D");
	string strNew = context.substr((first + 2), (last - first - 2));

	int node = std::stoi(strNew);

	//start calculating energy after complete association
	if (GetAssocNum() == StaNum) {
		switch (state) {
		case WifiPhy::State::IDLE: //Idle
			timeIdle = timeIdle + duration;
			timeIdleArray[node] = timeIdleArray[node] + duration;
			break;
		case WifiPhy::State::RX: //Rx
			timeRx = timeRx + duration;
			timeRxArray[node] = timeRxArray[node] + duration;
			break;
		case WifiPhy::State::TX: //Tx
			timeTx = timeTx + duration;
			timeTxArray[node] = timeTxArray[node] + duration;
			break;
		case WifiPhy::State::SLEEP: //Sleep
			timeSleep = timeSleep + duration;
			timeSleepArray[node] = timeSleepArray[node] + duration;
			break;
		}
	}
}

int main(int argc, char *argv[]) {
	PacketMetadata::Enable();

	//LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
	//LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
	//LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	/*LogComponentEnable ("CoapClient", LOG_LEVEL_INFO);
	LogComponentEnable ("CoapServer", LOG_LEVEL_INFO);*/

	LogComponentEnable ("ApWifiMac", LOG_LEVEL_INFO);
	LogComponentEnable ("StaWifiMac", LOG_LEVEL_INFO);
	LogComponentEnable ("EdcaTxopN", LOG_LEVEL_DEBUG);
	//LogComponentEnable ("MacLow", LOG_LEVEL_DEBUG);

	bool OutputPosition = true;
	config = Configuration(argc, argv);

	config.rps = configureRAW(config.rps, config.RAWConfigFile);
	config.Nsta = config.NRawSta;

	configurePageSlice ();
	configureTIM ();
	//checkRawAndTimConfiguration ();

	if (config.trafficType == "tcpipcamera")
	{
		config.NSSFile = config.trafficType + "_" + std::to_string(config.Nsta)	+ "sta_"
					+ std::to_string(config.NGroup) + "Group_"
					+ std::to_string(config.NRawSlotNum) + "slots_"
					+ std::to_string(config.ipcameraDataRate) + "Kbps_"
					+ std::to_string(config.BeaconInterval) + "BI" + ".nss";
	}
	else if (config.trafficType == "udp" || config.trafficType == "udpecho")
	{
	config.NSSFile = config.trafficType + "_"
			+ std::to_string(config.Nsta) + "sta_"
			+ std::to_string(config.NGroup) + "Group_"
			+ std::to_string(config.NRawSlotNum) + "slots_"
			+ std::to_string(config.trafficInterval) + "s_"
			//+ std::to_string(config.payloadSize) + "B_"
			+ std::to_string(config.BeaconInterval) + "BI_"
			+ std::to_string(config.simulationTime) + "Tsim_"
			+ std::to_string(config.seed) + "seed_"
			+ config.DataMode + "_"
			+ config.rho + "rho"
			+ ".nss";
	}
	else if (config.trafficType == "coap")
		{
		config.NSSFile = config.trafficType + "_"
				+ std::to_string(config.Nsta) + "sta_"
				+ std::to_string(config.NGroup) + "Group_"
				+ std::to_string(config.NRawSlotNum) + "slots_"
				+ std::to_string(config.trafficInterval) + "s_"
				+ std::to_string(config.BeaconInterval) + "BI_"
				+ std::to_string(config.simulationTime) + "Tsim_"
				+ std::to_string(config.seed) + "seed_"
				+ config.DataMode + "_"
				+ config.rho + "rho_"
				+ std::to_string(config.cycleTime) + "Tl_"
				+ std::to_string(config.nControlLoops) + "loops";
		if (config.crossSlotBoundary == 0) config.NSSFile += "_NC.nss";
		else config.NSSFile += ".nss";
		}
	stats = Statistics(config.Nsta);
	eventManager = SimulationEventManager(config.visualizerIP, config.visualizerPort, "./nssfiles/" + config.NSSFile);
	uint32_t totalRawGroups(0);
	for (int i = 0; i < config.rps.rpsset.size(); i++)
	{
		int nRaw = config.rps.rpsset[i]->GetNumberOfRawGroups();
		totalRawGroups += nRaw;
		//cout << "Total raw groups after rps " << i << " is " << totalRawGroups << endl;
		for (int j = 0; j < nRaw; j++) {
			config.totalRawSlots += config.rps.rpsset[i]->GetRawAssigmentObj(j).GetSlotNum();
			//cout << "Total slots after group " << j << " is " << totalRawSlots << endl;
		}

	}
	transmissionsPerTIMGroupAndSlotFromAPSinceLastInterval = vector<long>(
			config.totalRawSlots, 0);
	transmissionsPerTIMGroupAndSlotFromSTASinceLastInterval = vector<long>(
			config.totalRawSlots, 0);

	RngSeedManager::SetSeed(config.seed);

	wifiStaNode.Create(config.Nsta);
	wifiApNode.Create(1);

	YansWifiChannelHelper channelBuilder = YansWifiChannelHelper();
	channelBuilder.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
			"Exponent", DoubleValue(3.76), "ReferenceLoss", DoubleValue(8.0),
			"ReferenceDistance", DoubleValue(1.0));
	channelBuilder.SetPropagationDelay(
			"ns3::ConstantSpeedPropagationDelayModel");
	Ptr<YansWifiChannel> channel = channelBuilder.Create();
	channel->TraceConnectWithoutContext("Transmission",
			MakeCallback(&onChannelTransmission)); //TODO

	YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
	phy.SetErrorRateModel("ns3::YansErrorRateModel");
	phy.SetChannel(channel);
	phy.Set("ShortGuardEnabled", BooleanValue(false));
	phy.Set("ChannelWidth", UintegerValue(getBandwidth(config.DataMode))); // changed
	phy.Set("EnergyDetectionThreshold", DoubleValue(-110.0));
	phy.Set("CcaMode1Threshold", DoubleValue(-113.0));
	phy.Set("TxGain", DoubleValue(0.0));
	phy.Set("RxGain", DoubleValue(0.0));
	phy.Set("TxPowerLevels", UintegerValue(1));
	phy.Set("TxPowerEnd", DoubleValue(0.0));
	phy.Set("TxPowerStart", DoubleValue(0.0));
	phy.Set("RxNoiseFigure", DoubleValue(6.8));
	phy.Set("LdpcEnabled", BooleanValue(true));
	phy.Set("S1g1MfieldEnabled", BooleanValue(config.S1g1MfieldEnabled));

	WifiHelper wifi = WifiHelper::Default();
	wifi.SetStandard(WIFI_PHY_STANDARD_80211ah);
	S1gWifiMacHelper mac = S1gWifiMacHelper::Default();

	Ssid ssid = Ssid("ns380211ah");
	StringValue DataRate;
	DataRate = StringValue(getWifiMode(config.DataMode)); // changed

	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate, "ControlMode", DataRate);

	mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing",
			BooleanValue(false), "VirtualAidsEnabled", BooleanValue(config.enableVirtualAids));

	NetDeviceContainer staDevice;
	staDevice = wifi.Install(phy, mac, wifiStaNode);

	mac.SetType ("ns3::ApWifiMac",
	                 "Ssid", SsidValue (ssid),
	                 "BeaconInterval", TimeValue (MicroSeconds(config.BeaconInterval)),
	                 "NRawStations", UintegerValue (config.Nsta),
	                 "RPSsetup", RPSVectorValue (config.rps),
	                 "PageSliceSet", pageSliceValue (config.pageS),
	                 "TIMSet", TIMValue (config.tim)
	               );

	phy.Set("TxGain", DoubleValue(3.0));
	phy.Set("RxGain", DoubleValue(3.0));
	phy.Set("TxPowerLevels", UintegerValue(1));
	phy.Set("TxPowerEnd", DoubleValue(30.0));
	phy.Set("TxPowerStart", DoubleValue(30.0));
	phy.Set("RxNoiseFigure", DoubleValue(6.8));

	apDevice = wifi.Install(phy, mac, wifiApNode);

	Config::Set(
			"/NodeList/*/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_EdcaTxopN/Queue/MaxPacketNumber",
			UintegerValue(10));
	Config::Set(
			"/NodeList/*/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/BE_EdcaTxopN/Queue/MaxDelay",
			TimeValue(NanoSeconds(6000000000000)));
/*
	string DataModeCamera = "OfdmRate650KbpsBW2MHz";
	StringValue aa = StringValue(DataModeCamera);
	for (uint16_t k = 0; k < config.Nsta; k++) {
		std::ostringstream APSTA;
		APSTA << k;
		std::string strAP = APSTA.str();
		Config::Set(
				"/NodeList/" + strAP
						+ "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/$ns3::ConstantRateWifiManager/DataMode",
				aa);
		Config::Set(
				"/NodeList/" + strAP
						+ "/DeviceList/0/$ns3::WifiNetDevice/RemoteStationManager/$ns3::ConstantRateWifiManager/ControlMode",
				aa);
		Config::Set(
				"/NodeList/" + strAP
						+ "/DeviceList/0/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/ChannelWidth",
				UintegerValue(2));
	}
*/
	std::ostringstream oss;
	oss << "/NodeList/" << wifiApNode.Get(0)->GetId()
			<< "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::ApWifiMac/";
	Config::ConnectWithoutContext(oss.str() + "RpsIndex", MakeCallback(&RpsIndexTrace));
	Config::ConnectWithoutContext(oss.str() + "RawGroup", MakeCallback(&RawGroupTrace));
	Config::ConnectWithoutContext(oss.str() + "RawSlot", MakeCallback(&RawSlotTrace));

	// mobility.
	MobilityHelper mobility;
	double xpos = std::stoi(config.rho, nullptr, 0);
	double ypos = xpos;
	mobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator", "X",
			StringValue(std::to_string(xpos)), "Y",
			StringValue(std::to_string(ypos)), "rho", StringValue(config.rho));
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(wifiStaNode);

	MobilityHelper mobilityAp;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<
			ListPositionAllocator>();
	positionAlloc->Add(Vector(xpos, ypos, 0.0));
	mobilityAp.SetPositionAllocator(positionAlloc);
	mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobilityAp.Install(wifiApNode);

	/*

	 MobilityHelper mobilityApCamera;
	 Ptr<ListPositionAllocator> positionAllocAp = CreateObject<ListPositionAllocator> ();
	 positionAllocAp->Add (Vector (xpos, ypos, 0.0));
	 mobilityApCamera.SetPositionAllocator (positionAllocAp);
	 mobilityApCamera.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	 mobilityApCamera.Install (wifiApNode);

	 float deltaAngle = 2* M_PI / (config.tcpipcameraEnd - config.tcpipcameraStart +1);
	 float angle = 0.0;
	 double x = 0.0;
	 double y = 0.0;

	 double Distance = 50.0;


	 Ptr<UniformRandomVariable> m_rv = CreateObject<UniformRandomVariable> ();


	 for (int i = config.tcpipcameraStart; i <= config.tcpipcameraEnd; i++)
	 {
	 x = cos(angle) * Distance + xpos;
	 y = sin(angle) * Distance + ypos;

	 MobilityHelper mobilityCamera;
	 Ptr<ListPositionAllocator> positionAllocSta = CreateObject<ListPositionAllocator> ();
	 positionAllocSta->Add(Vector(x, y, 0.0));
	 mobilityCamera.SetPositionAllocator(positionAllocSta);
	 mobilityCamera.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	 mobilityCamera.Install(wifiStaNode.Get(i));
	 angle += deltaAngle;
	 }

	 */

	/* Internet stack*/
	InternetStackHelper stack;
	stack.Install(wifiApNode);
	stack.Install(wifiStaNode);

	// wired CoAP server
	if (config.trafficType == "coap" && config.nControlLoops > 0)
	{
		externalNodes.Create(config.nControlLoops);
		externalNodes.Add(wifiApNode.Get(0));
		CsmaHelper ethernet;
		ethernet.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
		ethernet.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
		externalDevices = ethernet.Install (externalNodes);
		InternetStackHelper stack;
		for (auto it = externalNodes.Begin(); it != std::prev(externalNodes.End()); ++it)
		{
			stack.Install(*it);
		}

	}

	Ipv4AddressHelper address;
	address.SetBase("192.168.0.0", "255.255.0.0");

	staNodeInterface = address.Assign(staDevice);
	apNodeInterface = address.Assign(apDevice);

	if (config.trafficType == "coap"  && config.nControlLoops > 0)
	{
		address.SetBase("169.200.0.0", "255.255.0.0");
		externalInterfaces = address.Assign (externalDevices);
	}
	//trace association
	std::cout << "Configuring trace sources..." << std::endl;
	for (uint16_t kk = 0; kk < config.Nsta; kk++) {
		std::ostringstream STA;
		STA << kk;
		std::string strSTA = STA.str();

		assoc_record *m_assocrecord = new assoc_record;
		m_assocrecord->setstaid(kk);
		Config::Connect(
				"/NodeList/" + strSTA
						+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/Assoc",
				MakeCallback(&assoc_record::SetAssoc, m_assocrecord));
		Config::Connect(
				"/NodeList/" + strSTA
						+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::RegularWifiMac/$ns3::StaWifiMac/DeAssoc",
				MakeCallback(&assoc_record::UnsetAssoc, m_assocrecord));
		assoc_vector.push_back(m_assocrecord);
	}

	std::cout << "Populating routing tables..." << std::endl;
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	std::cout << "Populating ARP cache..." << std::endl;
	PopulateArpCache();

	// configure tracing for associations & other metrics
	std::cout << "Configuring trace sinks for nodes..." << std::endl;
	configureNodes(wifiStaNode, staDevice);

	Config::Connect(
			"/NodeList/" + std::to_string(config.Nsta)
					+ "/DeviceList/0/$ns3::WifiNetDevice/Phy/PhyRxDropWithReason",
			MakeCallback(&OnAPPhyRxDrop));
	Config::Connect(
			"/NodeList/" + std::to_string(config.Nsta)
					+ "/DeviceList/0/$ns3::WifiNetDevice/Mac/$ns3::ApWifiMac/PacketToTransmitReceivedFromUpperLayer",
			MakeCallback(&OnAPPacketToTransmitReceived));

	Ptr<MobilityModel> mobility1 = wifiApNode.Get (0)->GetObject<MobilityModel>();
	Vector apposition = mobility1->GetPosition();
	if  (OutputPosition)
	{
		uint32_t i = 0;
		while (i < config.Nsta)
		{
			Ptr<MobilityModel> mobility = wifiStaNode.Get (i)->GetObject<MobilityModel>();
			Vector position = mobility->GetPosition();
			nodes[i]->x = position.x;
			nodes[i]->y = position.y;
			std::cout << "Sta node#" << i << ", " << "position = " << position
					<< std::endl;
			dist[i] = mobility->GetDistanceFrom(
					wifiApNode.Get(0)->GetObject<MobilityModel>());
			i++;
		}
		std::cout << "AP node, position = " << apposition << std::endl;
	}

	/*Print of the state of the stations*/
	for (uint32_t i = 0; i < config.Nsta; i++) {
		std::ostringstream STA;
		STA << i;
		std::string strSTA = STA.str();

		Config::Connect(
				"/NodeList/" + strSTA
						+ "/DeviceList/*/Phy/$ns3::YansWifiPhy/State/State",
				MakeCallback(&PhyStateTrace));
	}

	eventManager.onStartHeader();
	eventManager.onStart(config);
	if (config.rps.rpsset.size() > 0)
		for (uint32_t i = 0; i < config.rps.rpsset.size(); i++)
			for (uint32_t j = 0; j < config.rps.rpsset[i]->GetNumberOfRawGroups(); j++)
				eventManager.onRawConfig(i, j, config.rps.rpsset[i]->GetRawAssigmentObj(j));

	for (uint32_t i = 0; i < config.Nsta; i++)
		eventManager.onSTANodeCreated(*nodes[i]);

	eventManager.onAPNodeCreated(apposition.x, apposition.y);
	eventManager.onStatisticsHeader();

	sendStatistics(true);

	Simulator::Stop(Seconds(config.simulationTime + config.CoolDownPeriod)); // allow up to a minute after the client & server apps are finished to process the queue
	Simulator::Run();

	//printStatistics();
	printStatsToFile (config.PrintStats);

	// Visualizer throughput
	int pay = 0, totalSuccessfulPackets = 0, totalSentPackets = 0, totalPacketsEchoed = 0, rtPacketsDelivered = 0;
	for (int i = 0; i < config.Nsta; i++)
	{
		totalSuccessfulPackets += stats.get(i).NumberOfSuccessfulPackets;
		totalSentPackets += stats.get(i).NumberOfSentPackets;
		totalPacketsEchoed += stats.get(i).NumberOfSuccessfulRoundtripPackets;
		if (config.nControlLoops > 0 && i < config.nControlLoops)
		{
			rtPacketsDelivered += stats.get(i).NumberOfSuccessfulPackets;
		}
		pay += stats.get(i).TotalPacketPayloadSize;
		cout << i << " sent: " << stats.get(i).NumberOfSentPackets
				<< "\t" << "; delivered: " << stats.get(i).NumberOfSuccessfulPackets
				<< "\t" << "; echoed: " << stats.get(i).NumberOfSuccessfulRoundtripPackets
				<< "\t" << "; packetloss: " << stats.get(i).GetPacketLoss(config.trafficType)
				//<< "\t" << "; remaining TX queue len: " << nodes[i]->queueLength
				<< endl;
	}

	if (config.trafficType == "udp")
	{
		double throughput = 0;
		uint32_t totalPacketsThrough =
				DynamicCast<UdpServer>(serverApp.Get(0))->GetReceived();
		throughput = totalPacketsThrough * config.payloadSize * 8
				/ (config.simulationTime + config.CoolDownPeriod - stats.TimeWhenEverySTAIsAssociated.GetSeconds() * 1000000.0);
		cout << "totalPacketsThrough " << totalPacketsThrough << " ++my "
				<< totalSuccessfulPackets << endl;
		cout << "throughput " << throughput << " ++my "
				<< pay * 8. / (config.simulationTime + config.CoolDownPeriod - stats.TimeWhenEverySTAIsAssociated.GetSeconds() * 1000000.0) << endl;
		std::cout << "datarate" << "\t" << "throughput" << std::endl;
		std::cout << config.datarate << "\t" << throughput << " Mbit/s"
				<< std::endl;

	}
	else if (config.trafficType == "udpecho")
	{
		double ulThroughput = 0, dlThroughput = 0;
		ulThroughput = totalSuccessfulPackets * config.payloadSize * 8 / (config.simulationTime * 1000000.0);
		dlThroughput = totalPacketsEchoed * config.payloadSize * 8 / (config.simulationTime * 1000000.0);
		cout << "totalPacketsSent " << totalSentPackets << endl;
		cout << "totalPacketsDelivered " << totalSuccessfulPackets << endl;
		cout << "totalPacketsEchoed " << totalPacketsEchoed << endl;
		cout << "UL packets lost " << totalSentPackets - totalSuccessfulPackets << endl;
		cout << "DL packets lost " << totalSuccessfulPackets - totalPacketsEchoed << endl;
		cout << "Total packets lost " << totalSentPackets - totalPacketsEchoed << endl;

		/*cout << "uplink throughput Mbit/s " << ulThroughput << endl;
		cout << "downlink throughput Mbit/s " << dlThroughput << endl;*/

		double throughput = (totalSuccessfulPackets + totalPacketsEchoed) * config.payloadSize * 8 / ((config.simulationTime + config.CoolDownPeriod - stats.TimeWhenEverySTAIsAssociated.GetSeconds()) * 1000000.0);

		std::cout << "datarate" << "\t" << "throughput" << std::endl;
		std::cout << config.datarate << "\t" << throughput * 1000 << " Kbit/s" << std::endl;
	}
	else if (config.trafficType == "coap")
	{
		double ulThroughput = 0, dlThroughput = 0;
		ulThroughput = totalSuccessfulPackets * config.payloadSize * 8 / (stats.EndApplicationTime.GetSeconds() - stats.TimeWhenEverySTAIsAssociated.GetSeconds() * 1000000.0);
		dlThroughput = totalPacketsEchoed * config.payloadSize * 8 / (stats.EndApplicationTime.GetSeconds() - stats.TimeWhenEverySTAIsAssociated.GetSeconds() * 1000000.0);
		cout << "totalPacketsSent " << totalSentPackets << endl;
		cout << "totalPacketsDelivered " << totalSuccessfulPackets << endl;
		cout << "totalPacketsEchoed " << totalPacketsEchoed << endl;
		cout << "UL packets lost " << totalSentPackets - totalSuccessfulPackets << endl;
		cout << "DL packets lost " << rtPacketsDelivered - totalPacketsEchoed << endl;
		cout << "Total packets lost " << totalSentPackets - totalSuccessfulPackets + rtPacketsDelivered - totalPacketsEchoed << endl;

		/*cout << "uplink throughput Mbit/s " << ulThroughput << endl;
				cout << "downlink throughput Mbit/s " << dlThroughput << endl;*/

		double throughput = (totalSuccessfulPackets + totalPacketsEchoed) * config.payloadSize * 8 / ((stats.EndApplicationTime.GetSeconds() - stats.TimeWhenEverySTAIsAssociated.GetSeconds()) * 1000000.0);
		cout << "Time apps stoped=" << stats.EndApplicationTime.GetSeconds() << " s" << endl;
		std::cout << "datarate" << "\t" << "throughput Kbit/s" << std::endl;
		std::cout << config.datarate << "\t" << throughput * 1000 << " Kbit/s" << std::endl;
	}
	cout << "total packet loss % "
			<< 100 - 100. * (totalSuccessfulPackets + totalPacketsEchoed) / (totalSentPackets + rtPacketsDelivered) << endl;

	Simulator::Destroy();

	//Energy consumption per station
	timeRx = timeRx / config.Nsta;
	timeIdle = timeIdle / config.Nsta;
	timeTx = timeTx / config.Nsta;
	timeSleep = timeSleep / config.Nsta;


	ofstream risultati;
	//string addressresults = Outputpath + "/moreinfo.txt";
	string addressresults = config.OutputPath + "moreinfo.txt";
	risultati.open(addressresults.c_str(), ios::out | ios::trunc);

	risultati
			<< "Sta node#      distance        timerx      timeidle        timetx      timesleep      totenergy"
			<< std::endl;
	int i = 0;
	ns3::int64x64_t totenergycons = 0;
	string spazio = "         ";

	while (i < config.Nsta) {
		totenergycons = (timeRxArray[i].GetSeconds() * 4.4)
				+ (timeIdleArray[i].GetSeconds() * 4.4)
				+ (timeTxArray[i].GetSeconds() * 7.2);
		risultati << i << spazio << dist[i] << spazio
				<< timeRxArray[i].GetSeconds() << spazio
				<< timeIdleArray[i].GetSeconds() << spazio
				<< timeTxArray[i].GetSeconds() << spazio
				<< timeSleepArray[i].GetSeconds() << spazio << totenergycons
				<< std::endl;

		totenergycons = 0;

		/*
		 cout << "================== Sleep " << stats.get(i).TotalSleepTime.GetSeconds() << endl;
		 cout << "================== Tx " << stats.get(i).TotalTxTime.GetSeconds() << endl;
		 cout << "================== Rx " << stats.get(i).TotalRxTime.GetSeconds() << endl;
		 cout << "+++++++++++++++++++IDLE " << stats.get(i).TotalIdleTime.GetSeconds() << endl;
		 cout << "ooooooooooooooooooo TOTENERGY " <<  stats.get(i).GetTotalEnergyConsumption() << " mW" << endl;
		 cout << "Rx+Idle ENERGY " <<  stats.get(i).EnergyRxIdle << " mW" << endl;
		 cout << "Tx ENERGY " <<  stats.get(i).EnergyTx << " mW" << endl;*/

		i++;
	}

	risultati.close();
	return 0;
}
