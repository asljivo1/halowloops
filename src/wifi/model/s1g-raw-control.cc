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
#include "extension-headers.h"
#include "mac-low.h"
#include "amsdu-subframe-header.h"
#include "msdu-aggregator.h"
#include "ns3/uinteger.h"
#include "wifi-mac-queue.h"

#include "s1g-raw-control.h"

#include <iostream>     // std::cout
#include <algorithm>    // std::find
#include <vector>       // std::vector
#include <fstream>
#include <string>

#include <stdio.h>
#include <valarray>
#include <stdlib.h>
#include <math.h>
#include <climits>
#include <iterator>
#include <map>
#include <iomanip>
//#include <sstream>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("S1gRawCtr");



Slot::Slot () : m_assignedAid(0), m_slotCount(0), m_slotStartTime(Time ()), m_slotDuration(Time ()), m_slotFormat (1), m_startAid (0), m_endAid (0)
{}

Slot::~Slot ()
{}

uint16_t
Slot::GetAid (void) const
{
	//NS_ASSERT (m_startAid == m_endAid);
	return m_assignedAid;
}

void
Slot::SetAid (uint16_t aid)
{
	NS_ASSERT (aid > 0 && aid < 8192);
	m_assignedAid = aid;
	m_startAid = aid;
	m_endAid = aid;
}

uint16_t
Slot::GetStartAid (void) const
{
	return m_startAid;
}

void
Slot::SetStartAid (uint16_t aid)
{
	NS_ASSERT (aid > 0 && aid < 8192);
	m_startAid = aid;
}

uint16_t
Slot::GetEndAid (void) const
{
	return m_endAid;
}

void
Slot::SetEndAid (uint16_t aid)
{
	NS_ASSERT (aid > 0 && aid < 8192);
	m_endAid = aid;
}

uint16_t
Slot::GetSlotCount (void) const
{
	return m_slotCount;
}

void
Slot::SetSlotCount (uint16_t count)
{
	NS_ASSERT((!m_slotFormat && (count < 256)) || (m_slotFormat && (count < 2048)));
	m_slotCount = count;
}

uint8_t
Slot::GetSlotFormat (void) const
{
	return m_slotFormat;
}

void
Slot::SetSlotFormat (uint8_t format)
{
	NS_ASSERT (format < 2);
	m_slotFormat = format;
}

Time
Slot::GetSlotDuration (void) const
{
	return MicroSeconds(500 + 120 * GetSlotCount());
}

Time
Slot::GetSlotStartTime (void) const
{
	return m_slotStartTime;
}

void
Slot::SetSlotStartTime (Time start)
{
	m_slotStartTime = start;
}

SensorActuator::SensorActuator (void) : m_pendingDownlinkPackets(0), m_paged (false), m_nTx (0), m_oldRawStart (INT_MAX), m_newRawStart (INT_MAX), m_tSent (Time ()), m_tSentPrev (Time ()), m_tInterval (Time ()), m_tIntervalMin (Time ()), m_tIntervalMax (Time ()), m_tEnqMin (Time ()), m_outstandingUplinkPackets (0), m_scheduledUplinkPackets (0), m_numOutstandingDl(0), m_deltaT(Time())
{

}

SensorActuator::~SensorActuator (void)
{
}

//** AP update info after RAW ends(right before next beacon is sent)
//list of sensor allowed to transmit in last beacon ************
Sensor::Sensor ()
{
	last_transmissionInterval = 1;
	last2_transmissionInterval = 1;
	m_transmissionIntervalMax =1;
	m_transmissionIntervalMin = 1;
	m_transInOneBeacon = 1;
	m_transIntervalListSize = 5;
	m_index = 0;
}

Sensor::~Sensor ()
{
}

uint16_t
Sensor::GetTransInOneBeacon (void) const
{
   return m_transInOneBeacon;
}

void
Sensor::SetTransInOneBeacon (uint16_t num)
{
     m_transInOneBeacon = num;
}


void
Sensor::EstimateNextTransmissionId (uint64_t m_nextId)
{
    m_nextTransmissionId = m_nextId;
}


void
Sensor::SetAid (uint16_t aid)
{
    m_aid = aid;
}

void
Sensor::SetTransmissionSuccess (bool success)
{
    m_transmissionSuccess = success;
}

uint16_t
Sensor::GetAid (void) const
{
    return m_aid;
}

bool
Sensor::GetTransmissionSuccess (void) const
{
    return m_transmissionSuccess;
}

uint64_t
Sensor::GetEstimateNextTransmissionId (void) const
{
    return m_nextTransmissionId;
}

void
Sensor::SetEverSuccess(bool success)
{
    m_everSuccess = success;
}
bool
Sensor::GetEverSuccess (void) const
{
    return m_everSuccess;
}

//OffloadStation
OffloadStation::OffloadStation ():
                m_offloadFailedCount (0)
{
}

OffloadStation::~OffloadStation ()
{
}

void
OffloadStation::SetAid (uint16_t aid)
{
    m_aid = aid;
}

void
OffloadStation::SetTransmissionSuccess (bool success)
{
    m_transmissionSuccess = success;
}

void
OffloadStation::SetOffloadStaActive (bool active)
{
    m_offloadStaActive = active;
}

void
OffloadStation::IncreaseFailedTransmissionCount (bool add)
{
    if (add)
     {
        m_offloadFailedCount++;
     }
    else
     {
        m_offloadFailedCount = 0;
     }
}


uint16_t
OffloadStation::GetAid (void) const
{
    return m_aid;
}

bool
OffloadStation::GetTransmissionSuccess (void) const
{
    return m_transmissionSuccess;
}

bool
OffloadStation::GetOffloadStaActive (void) const
{
    return m_offloadStaActive;
}

uint16_t
OffloadStation::GetFailedTransmissionCount (void) const
{
    return m_offloadFailedCount;
}

//
S1gRawCtr::S1gRawCtr ()
{
   /*RpsIndex = 0;
   m_offloadFailedMax = 5;
   sensorpacketsize = 1;
   offloadpacketsize = 1;
   m_slotDurationCount = 15;


    m_rawslotDuration = (m_slotDurationCount*120) +500; //for test.
    m_offloadRawslotDuration = (m_slotDurationCount*120) +500;

    m_beaconOverhead = 0; // us

    MaxSlotForSensor = 40; //In order to guarantee channel for offload stations.
    m_rps = new RPS;*/
	currentId = 0;
	m_nTxs = 0;
	m_prevRps = nullptr;
	//m_prevPrevRps = nullptr;
	m_rps = new RPS;
	m_startOptimalOpp = Time();
	m_success = false;
}

S1gRawCtr::~S1gRawCtr ()
{
	deleteRps();
}

void
S1gRawCtr::UpdateSensorStaInfo (std::vector<uint16_t> sensorList, std::vector<uint16_t> receivedFromAids, std::vector<Time> receivedTimes, std::vector<Time> sentTimes)
{
	for (std::vector<uint16_t>::iterator ci = sensorList.begin(); ci != sensorList.end(); ci++)
	{
		if (LookupSensorSta (*ci) == nullptr)
		{
			Sensor *sta = new Sensor;
			sta->SetAid (*ci);
			sta->SetNumPacketsReceived(0);
			m_sensorStations.push_back (sta);
		}
	}
	bool disassoc;
	StationsCI itcheck = m_sensorStations.begin();
	for (uint16_t i = 0; i < m_sensorStations.size(); i++)
	{
		disassoc = true;
		for (std::vector<uint16_t>::iterator ci = sensorList.begin(); ci != sensorList.end(); ci++)
		{
			if ((*itcheck)->GetAid ()  == *ci)
			{
				disassoc = false;
				if (i < m_sensorStations.size() - 1)
					itcheck++;
				break;
			}
		}

		if (disassoc == true)
		{
			NS_LOG_UNCOND ( "Aid " << (*itcheck)->GetAid () << " erased from m_sensorStations since disassociated");
			m_sensorStations.erase(itcheck);
		}
	}
	for (std::vector<uint16_t>::iterator ci = receivedFromAids.begin(); ci != receivedFromAids.end(); ci++)
	{
		bool match = false;
		for (std::vector<uint16_t>::iterator it = m_aidList.begin(); it != m_aidList.end(); it++)
		{
			if (*ci == *it)
			{
				match = true;
				break;
			}
		}
		uint16_t nTX = 0;
		Sensor * stationTransmit = LookupSensorSta (*ci);
		if (stationTransmit != nullptr && !match)
		{
			m_aidList.push_back (*ci);
			if (m_prevRps && m_prevRps->GetNumAssignedRaws(*ci))
				stationTransmit->m_oldRawStart = this->m_prevRps->GetRawSlotStartFromAid(*ci);
			//NS_LOG_UNCOND ("+++++++++aid=" << *ci <<", m_oldRawStart = " << stationTransmit->m_oldRawStart);
			for (int i = 0; i < receivedFromAids.size(); i++)
			{
				if (*ci == receivedFromAids[i])
				{
					stationTransmit->m_nTx=++nTX;
					stationTransmit->m_tSuccessPreLast = stationTransmit->m_tSuccessLast;
					stationTransmit->m_tSuccessLast = receivedTimes[i];
					stationTransmit->m_tSentPrev = stationTransmit->m_tSent;
					stationTransmit->m_tSent = sentTimes[i];
					if (stationTransmit->m_tSentPrev != Time ())
					{
						if (stationTransmit->m_tInterval == Time ())
							stationTransmit->m_tInterval = stationTransmit->m_tSent - stationTransmit->m_tSentPrev;

					}
					//NS_LOG_UNCOND ("*****aid=" << *ci << ", m_tInterval=" << stationTransmit->m_tInterval << ", m_tSent=" << stationTransmit->m_tSent << ", m_tSentPrev=" << stationTransmit->m_tSentPrev);
					//NS_LOG_UNCOND ("m_tSuccessLast=" << stationTransmit->m_tSuccessLast << ", m_tSuccessPreLast=" << stationTransmit->m_tSuccessPreLast << ", m_nTx=" << stationTransmit->m_nTx);
				}
			}
		}
	}
	NS_LOG_DEBUG ("receivedFromAids.size () = " << receivedFromAids.size () << ", m_sensorStations.size() = " << m_sensorStations.size() << ", currentBeacon = " << currentId);

}

void
S1gRawCtr::UpdateCriticalStaInfo (std::vector<uint16_t> criticalAids, std::vector<uint16_t> receivedFromAids, std::vector<uint16_t> enqueuedToAids, std::vector<Time> receivedTimes, std::vector<Time> sentTimes, std::map<uint16_t, uint16_t> numExpectedDlPacketsForAids, std::string outputpath)
{
	for (std::vector<uint16_t>::iterator ci = criticalAids.begin(); ci != criticalAids.end(); ci++)
	{
		if (LookupCriticalSta (*ci) == nullptr)
		{
			SensorActuator * sta = new SensorActuator;
			sta->SetAid (*ci);

			sta->SetNumPacketsReceived(0);
			m_criticalStations.push_back (sta);

			std::ostringstream ss;
			ss.clear ();
			ss.str ("");
			ss << (*ci);
			std::string aidstring = ss.str();
			sensorfile = outputpath + aidstring + ".txt";
			std::ofstream outputfile;
			outputfile.open (sensorfile, std::ios::out | std::ios::app);
			outputfile.close();

			//NS_LOG_UNCOND ("initial, aid = " << *ci);

		}
		else
		{
			SensorActuator * sta = LookupCriticalSta (*ci);
			sta->m_deltaT = Time();
			sta->m_outstandingUplinkPackets = 0;
			sta->m_numOutstandingDl = 0;
			sta->m_paged = false;
			sta->m_pendingDownlinkPackets = 0;
			sta->m_scheduledUplinkPackets = 0;
		}
	}
	bool disassoc;
	CriticalStationsCI itcheck = m_criticalStations.begin();
	for (uint16_t i = 0; i < m_criticalStations.size(); i++)
	{
		disassoc = true;
		for (std::vector<uint16_t>::iterator ci = criticalAids.begin(); ci != criticalAids.end(); ci++)
		{
			if ((*itcheck)->GetAid ()  == *ci)
			{
				disassoc = false;
				if (i < m_criticalStations.size() - 1)
					itcheck++;  //avoid itcheck increase to m_sensorlist.end()
				break;
			}
		}

		if (disassoc == true)
		{
			NS_LOG_UNCOND ( "Aid " << (*itcheck)->GetAid () << " erased from m_criticalStations since disassociated");
			m_criticalStations.erase(itcheck);
		}
	}


	for (std::vector<uint16_t>::iterator ci = receivedFromAids.begin(); ci != receivedFromAids.end(); ci++)
	{
		bool match = false;
		for (std::vector<uint16_t>::iterator it = m_aidList.begin(); it != m_aidList.end(); it++)
		{
			if (*ci == *it)
			{
				match = true;
				break;
			}
		}
		uint16_t nTX = 0;
		SensorActuator * stationTransmit = LookupCriticalSta (*ci);
		if (stationTransmit != nullptr && !match)
		{
			NS_LOG_UNCOND ("+++++++++aid=" << *ci <<", m_oldRawStart = " << stationTransmit->m_oldRawStart);
			m_aidList.push_back (*ci);
			/*if (m_prevRps && m_prevRps->GetNumAssignedRaws(*ci))
			{
				//m_prevRps->Print(std::cout);
				//std::cout << "current ID = " << this->currentId << std::endl;
				//stationTransmit->m_oldRawStart = this->m_prevRps->GetRawSlotStartFromAid(*ci);
			}*/
			for (int i = 0; i < receivedFromAids.size(); i++)
			{
				if (*ci == receivedFromAids[i])
				{
					stationTransmit->m_nTx=++nTX;
					stationTransmit->m_tSuccessPreLast = stationTransmit->m_tSuccessLast;
					stationTransmit->m_tSuccessLast = receivedTimes[i];
					stationTransmit->m_tSentPrev = stationTransmit->m_tSent;
					stationTransmit->m_tSent = sentTimes[i];
					if (stationTransmit->m_tSentPrev != Time ())
					{
						if (stationTransmit->m_tInterval == Time ())
							stationTransmit->m_tInterval = stationTransmit->m_tSent - stationTransmit->m_tSentPrev;
						//else
							//NS_ASSERT (stationTransmit->m_tInterval == stationTransmit->m_tSent - stationTransmit->m_tSentPrev);
					}
					//NS_LOG_UNCOND ("*****aid=" << *ci << ", m_tInterval=" << stationTransmit->m_tInterval << ", m_tSent=" << stationTransmit->m_tSent << ", m_tSentPrev=" << stationTransmit->m_tSentPrev);
					//NS_LOG_UNCOND ("m_tSuccessLast=" << stationTransmit->m_tSuccessLast << ", m_tSuccessPreLast=" << stationTransmit->m_tSuccessPreLast << ", m_nTx=" << stationTransmit->m_nTx);
				}
			}
		}
	}


	for (std::vector<uint16_t>::iterator ci = enqueuedToAids.begin(); ci != enqueuedToAids.end(); ci++)
	{
		bool match = false;
		for (std::vector<uint16_t>::iterator it = m_aidListPaged.begin(); it != m_aidListPaged.end(); it++)
		{
			if (*ci == *it)
			{
				match = true;
				break;
			}
		}
		SensorActuator * stationTransmit = LookupCriticalSta (*ci);
		if (stationTransmit != nullptr && !match)
		{
			m_aidListPaged.push_back (*ci);
			int numpack = 0;
			for (int i = 0; i < enqueuedToAids.size(); i++)
			{
				if (*ci == enqueuedToAids[i])
				{
					stationTransmit->m_paged=true;
					stationTransmit->m_pendingDownlinkPackets = ++numpack;
				}
			}
		}
	}

	m_aidForcePage.clear();
	for (auto sta : m_criticalStations)
	{
		int numOutstandingUl (0);
		while (sta->m_tInterval != Time() && sta->m_tSent + (numOutstandingUl + 1) * sta->m_tInterval <= Simulator::Now())
		{
			numOutstandingUl++;
			if (std::find(m_aidForcePage.begin(), m_aidForcePage.end(), sta->GetAid()) == m_aidForcePage.end())
				m_aidForcePage.push_back(sta->GetAid());
		}
		sta->m_outstandingUplinkPackets = numOutstandingUl;
		int numScheduledUl (0);
		while (sta->m_tInterval != Time() && sta->m_tSent + (numOutstandingUl + numScheduledUl + 1) * sta->m_tInterval > Simulator::Now() && sta->m_tSent + (numOutstandingUl + numScheduledUl + 1) * sta->m_tInterval < Simulator::Now() + MicroSeconds(m_beaconInterval))
		{
			numScheduledUl++;
			if (std::find(m_aidForcePage.begin(), m_aidForcePage.end(), sta->GetAid()) == m_aidForcePage.end())
				m_aidForcePage.push_back(sta->GetAid());
		}
		//numScheduledUl -= numOutstandingUl;
		sta->m_scheduledUplinkPackets = numScheduledUl;
		NS_LOG_DEBUG("Sta " << sta->GetAid() << " delivered the last packet to AP at " << sta->m_tSent << ", interval=" << sta->m_tInterval << ", est. num. of outstanding UL pacets is " << sta->m_outstandingUplinkPackets);
		NS_LOG_DEBUG("Sta " << sta->GetAid() << " should transmit " << numScheduledUl << " packets in the next BI, interval=" << sta->m_tInterval);
		sta->m_numOutstandingDl = numExpectedDlPacketsForAids.find(sta->GetAid())->second;

		sta->m_deltaT = sta->m_tSent + (numOutstandingUl + 1) * sta->m_tInterval - Simulator::Now();

	}


	NS_LOG_DEBUG ("receivedFromAids.size () = " << receivedFromAids.size () << ", m_criticalStations.size() = " << m_criticalStations.size() << ", m_aidListPaged.size = " << this->m_aidListPaged.size() << ", currentBeacon = " << currentId);
	NS_LOG_DEBUG ("enqueuedToAids = " << enqueuedToAids.size());
	/*NS_LOG_UNCOND ("AID LIST:");
	for (std::vector<uint16_t>::iterator it = m_aidList.begin(); it != m_aidList.end(); it++)
	{
		SensorActuator * s = LookupCriticalSta (*it);
		NS_LOG_UNCOND ("aid=" << *it << ", m_nTx=" << s->m_nTx << ", m_tSuccessPreLast=" << s->m_tSuccessPreLast << ", m_tSuccessLast=" << s->m_tSuccessLast);
	}
	NS_LOG_UNCOND ("AID PAGED LIST:");
	for (std::vector<uint16_t>::iterator it = m_aidListPaged.begin(); it != m_aidListPaged.end(); it++)
	{
		SensorActuator * s = LookupCriticalSta (*it);
		NS_LOG_UNCOND ("aid=" << *it << ", m_pendingDownlinkPackets=" << s->m_pendingDownlinkPackets);
	}*/
}

void
S1gRawCtr::UdpateSensorStaInfo (std::vector<uint16_t> m_sensorlist, std::vector<uint16_t> m_receivedAid, std::string outputpath)
{
  //initialization
  //if sorted queue is empty, put all sensor into queue
  std::string ApNode;
  std::ofstream outputfile;
  std::ostringstream APId;

  uint16_t numsensor = m_sensorlist.size (); //need to be improved
  /*if (m_stations.size() < numsensor)
     {
      for (uint16_t i = m_stations.size(); i < numsensor; i++)
        {
            Sensor * m_sta = new Sensor;
            m_sta->SetAid (i+1);
            m_sta->EstimateNextTransmissionId (currentId+1);
            //NS_LOG_UNCOND ("initial, set next id = " << currentId);
            //initialize UpdateInfo struct
            m_sta->SetEverSuccess (false);
            m_sta->m_snesorUpdatInfo = (UpdateInfo){currentId,currentId,currentId,false,currentId,currentId,currentId,false};
            //
            m_stations.push_back (m_sta); // should create a Dispose function?
            //
            APId.clear ();
            APId.str ("");
            APId << (i+1);
            ApNode = APId.str();
            sensorfile = outputpath + ApNode + ".txt";
            outputfile.open (sensorfile, std::ios::out | std::ios::trunc);
            outputfile.close();
            //
        }
     } */

    for (std::vector<uint16_t>::iterator ci = m_sensorlist.begin(); ci != m_sensorlist.end(); ci++)
    {
        if (LookupSensorSta (*ci) == nullptr)
        {
            Sensor * m_sta = new Sensor;
            m_sta->SetAid (*ci);
            m_sta->EstimateNextTransmissionId (currentId+1);
            //NS_LOG_UNCOND ("initial, set next id = " << currentId);
            //initialize UpdateInfo struct
            m_sta->SetEverSuccess (false);
            m_sta->m_snesorUpdatInfo = (UpdateInfo){currentId,currentId,currentId,false,currentId,currentId,currentId,false};
            for (uint16_t i = 0; i < m_sta->m_transIntervalListSize; i++)
            {
                m_sta->m_transIntervalList.push_back(1);
            }
            //
            m_stations.push_back (m_sta); // should create a Dispose function?
            //
            APId.clear ();
            APId.str ("");
            APId << (*ci);
            ApNode = APId.str();
            sensorfile = outputpath + ApNode + ".txt";
            outputfile.open (sensorfile, std::ios::out | std::ios::app);
            outputfile.close();
            //
            m_lastTransmissionList.push_back (*ci);
            //NS_LOG_UNCOND ("initial, aid = " << *ci);

        }
    }

    bool disassoc;
    //for (StationsCI it = m_stations.begin(); it != m_stations.end(); it++)
    StationsCI itcheck = m_stations.begin();
    for (uint16_t i = 0; i < m_stations.size(); i++)

    {
        disassoc = true;
        for (std::vector<uint16_t>::iterator ci = m_sensorlist.begin(); ci != m_sensorlist.end(); ci++)
        {
            if ((*itcheck)->GetAid ()  == *ci)
             {
                 disassoc = false;
                 if (i < m_stations.size() - 1)
                     itcheck++;  //avoid itcheck increase to m_sensorlist.end()
                 break;
             }
        }

      if (disassoc == true)
        {
            NS_LOG_UNCOND ( "Aid " << (*itcheck)->GetAid () << " erased from m_stations since disassociated");
            m_stations.erase(itcheck);
        }
    }



    NS_LOG_UNCOND ("m_aidList.size() = " << m_aidList.size() << ", m_receivedAid = " << m_receivedAid.size () << ", m_stations.size() = " << m_stations.size() << ", currentId = " << currentId);

    //update transmission interval info, stations allowed to transmit in last beacon
    for (std::vector<uint16_t>::iterator it = m_aidList.begin(); it != m_aidList.end(); it++)
     {
        Sensor * stationTransmit = LookupSensorSta (*it);
         if (stationTransmit == nullptr)
         {
             return;
         }

         m_receivedsuccess = false;
         for (std::vector<uint16_t>::iterator ci = m_receivedAid.begin(); ci != m_receivedAid.end(); ci++)
         {

             if (*ci == *it)
              {
                //NS_LOG_UNCOND ("stations of aid " << *it << " received");
                stationTransmit->SetTransmissionSuccess (true);
                  //output to files.
                  //write into currentID  coulumn
                  //write  into allowed to send coulumn
                  //write  into reaeived to receive coulumn
                  /* APId.clear ();
                  APId.str ("");
                  APId << (*it);
                  ApNode = APId.str();

                  sensorfile = outputpath + ApNode + ".txt";
                  outputfile.open (sensorfile, std::ios::out | std::ios::app);
                  outputfile << currentId << "\t" << "1" << "\t" << "1" << "\n";
                  outputfile.close(); */
                  //

                if (stationTransmit->GetEverSuccess () == false)
                  {
                      stationTransmit->m_snesorUpdatInfo = (UpdateInfo){currentId-1,currentId-1,currentId-1,false,currentId-1,currentId-1,currentId-1,false};
                      stationTransmit->SetEverSuccess (true);
                      /*for (uint16_t i = 0; i < stationTransmit->m_transIntervalListSize; i++)
                      {
                          stationTransmit->m_transIntervalList.push_back(1);
                      }*/
                      for (std::vector<uint16_t>::iterator ci = stationTransmit->m_transIntervalList.begin(); ci != stationTransmit->m_transIntervalList.end(); ci++)
                      {
                          (*ci) = 1;
                      }
                  }

                stationTransmit->m_snesorUpdatInfo.lastTryBFpreSuccessId = stationTransmit->m_snesorUpdatInfo.lastTryBFCurrentSuccessId; //update, swith current to pre

                stationTransmit->m_snesorUpdatInfo.preSuccessId = stationTransmit->m_snesorUpdatInfo.CurrentSuccessId; //update, swith current to pre
                stationTransmit->m_snesorUpdatInfo.CurrentSuccessId = currentId;

                  stationTransmit->m_snesorUpdatInfo.lastTryBFCurrentSuccessId = std::max(stationTransmit->m_snesorUpdatInfo.preSuccessId, stationTransmit->m_snesorUpdatInfo.CurrentUnSuccessId);

                stationTransmit->m_snesorUpdatInfo.preTrySuccess = stationTransmit->m_snesorUpdatInfo.CurrentTrySuccess;
                stationTransmit->m_snesorUpdatInfo.CurrentTrySuccess = true;

                goto EstimateInterval;
              }
         }


            //NS_LOG_UNCOND ("stations of aid " << *it << " not received");
            stationTransmit->SetTransmissionSuccess (false);
            //output to files.
            //write into currentID  coulumn
            //write  into allowed to send coulumn
            //write  into received to receive coulumn
           /* APId.clear ();
            APId.str ("");
            APId << (*it);
            ApNode = APId.str();

            sensorfile = outputpath + ApNode + ".txt";
            outputfile.open (sensorfile, std::ios::out | std::ios::app);
            outputfile << currentId << "\t" << "1" << "\t" << "0" << "\n";
            outputfile.close(); */
            //

            stationTransmit->m_snesorUpdatInfo.preUnsuccessId = stationTransmit->m_snesorUpdatInfo.CurrentUnSuccessId; //update, swith current to pre
            stationTransmit->m_snesorUpdatInfo.CurrentUnSuccessId = currentId;
            stationTransmit->m_snesorUpdatInfo.preSuccessId = stationTransmit->m_snesorUpdatInfo.CurrentSuccessId;

            stationTransmit->m_snesorUpdatInfo.preTrySuccess = stationTransmit->m_snesorUpdatInfo.CurrentTrySuccess;
            stationTransmit->m_snesorUpdatInfo.CurrentTrySuccess = false;

    EstimateInterval :
           //stationTransmit->EstimateTransmissionInterval (currentId);
           //NS_LOG_UNCOND ("... "); //to do, remove it
         uint16_t add=0;
     }

    for (std::vector<uint16_t>::iterator it = m_aidList.begin(); it != m_aidList.end(); it++)
     {
        Sensor * stationTransmit = LookupSensorSta (*it);
        if (stationTransmit == nullptr)
          {
            return;
          }

        uint16_t m_numReceived = 0;
        for (std::vector<uint16_t>::iterator ci = m_receivedAid.begin(); ci != m_receivedAid.end(); ci++)
          {
            if (*ci == *it)
             {
                m_numReceived++;
             }
          }

        //NS_LOG_UNCOND ("stations of aid " << *it << " received " << m_numReceived << " packets");

         APId.clear ();
         APId.str ("");
         APId << (*it);
         ApNode = APId.str();

         sensorfile = outputpath + ApNode + ".txt";
         outputfile.open (sensorfile, std::ios::out | std::ios::app);
         outputfile << currentId << "\t" << "1" << "\t" << m_numReceived << "\t" << stationTransmit->GetTransInOneBeacon () << "\n";
         outputfile.close();

         stationTransmit->SetNumPacketsReceived (m_numReceived);
         stationTransmit->EstimateTransmissionInterval (currentId, m_beaconInterval);
     }

 for (std::vector<uint16_t>::iterator ci = m_receivedAid.begin(); ci != m_receivedAid.end(); ci++)
    {
      uint16_t m_numReceived = 0;
      bool match = false;
      for (std::vector<uint16_t>::iterator it = m_aidList.begin(); it != m_aidList.end(); it++)
         {
             if (*ci == *it)
              {
                match = true;
                break;
              }
         }

        Sensor * stationTransmit = LookupSensorSta (*ci);
    if (stationTransmit != nullptr && !match)
        {
            m_aidList.push_back (*ci); //trick, avoid same receiveAid repeate several times
             if (stationTransmit->GetEverSuccess () == false)
             {
                 stationTransmit->m_snesorUpdatInfo = (UpdateInfo){currentId-1,currentId-1,currentId-1,false,currentId-1,currentId-1,currentId-1,false};
                 stationTransmit->SetEverSuccess (true);
                 for (std::vector<uint16_t>::iterator ci = stationTransmit->m_transIntervalList.begin(); ci != stationTransmit->m_transIntervalList.end(); ci++)
                 {
                     (*ci) = 1;
                 }
             }
             stationTransmit->SetTransmissionSuccess (true);

             stationTransmit->m_snesorUpdatInfo.lastTryBFpreSuccessId = stationTransmit->m_snesorUpdatInfo.lastTryBFCurrentSuccessId; //update, swith current to pre

             stationTransmit->m_snesorUpdatInfo.preSuccessId = stationTransmit->m_snesorUpdatInfo.CurrentSuccessId; //update, swith current to pre
             stationTransmit->m_snesorUpdatInfo.CurrentSuccessId = currentId;

             stationTransmit->m_snesorUpdatInfo.lastTryBFCurrentSuccessId = std::max(stationTransmit->m_snesorUpdatInfo.preSuccessId, stationTransmit->m_snesorUpdatInfo.CurrentUnSuccessId);

             stationTransmit->m_snesorUpdatInfo.preTrySuccess = stationTransmit->m_snesorUpdatInfo.CurrentTrySuccess;
             stationTransmit->m_snesorUpdatInfo.CurrentTrySuccess = true;

             //NS_LOG_UNCOND ("stations of aid " << *it << " received " << m_numReceived << " packets");

            for (std::vector<uint16_t>::iterator ct = m_receivedAid.begin(); ct != m_receivedAid.end(); ct++)
            {
                if (*ci == *ct)
                {
                    m_numReceived++;
                }
            }

             APId.clear ();
             APId.str ("");
             APId << (*ci);
             ApNode = APId.str();

             sensorfile = outputpath + ApNode + ".txt";
             outputfile.open (sensorfile, std::ios::out | std::ios::app);
             outputfile << currentId << "\t" << "0" << "\t" << m_numReceived << "\t" << stationTransmit->GetTransInOneBeacon () << "\n";
             outputfile.close();

             stationTransmit->SetNumPacketsReceived (m_numReceived);
             stationTransmit->EstimateTransmissionInterval (currentId, m_beaconInterval);
        }
    }

}

void
Sensor::SetNumPacketsReceived (uint16_t numReceived)
{
    m_receivedNum = numReceived;
}

uint16_t
Sensor::GetNumPacketsReceived (void) const
{
    return m_receivedNum;
}


void
Sensor::EstimateTransmissionInterval (uint64_t currentId, uint64_t m_beaconInterval)
{
    if (m_snesorUpdatInfo.preTrySuccess && m_transmissionSuccess)
     {
         m_index = 0;
        last2_transmissionInterval = last_transmissionInterval;
        last_transmissionInterval = m_transmissionInterval;
        /*
        if (GetNumPacketsReceived () > 1 && m_transmissionInterval != 1)
         {
             m_transmissionIntervalMin = last_transmissionInterval - 1;
             m_transmissionIntervalMax = last_transmissionInterval - 1;
         } */
         //NS_LOG_UNCOND ("aid = " << m_aid << " ,m_transmissionIntervalMin = " << m_transmissionIntervalMin << ", m_transmissionIntervalMin = " << m_transmissionIntervalMax << " , GetNumPacketsReceived = " << GetNumPacketsReceived () << ", m_transmissionInterval = " << m_transmissionInterval);

         if (GetNumPacketsReceived () > 1 && m_transmissionInterval > 1)
           {
             //m_transmissionIntervalMin = last_transmissionInterval - 1;
             //m_transmissionIntervalMax = last_transmissionInterval - 1;
             m_transmissionInterval = m_transmissionInterval - 1;
             m_transInOneBeacon = 1;
               uint16_t intervalsum = m_transmissionInterval;
               /*
               for (std::vector<uint16_t>::iterator ci = m_transIntervalList.begin(); ci != m_transIntervalList.end(); ci++)
               {
                   intervalsum = intervalsum + *ci;
               }

               m_transmissionInterval = intervalsum/(m_transIntervalListSize + 1); */
               m_transIntervalList.pop_back ();
               
               std::vector<uint16_t>::iterator ci;
               ci = m_transIntervalList.begin();
               m_transIntervalList.insert (ci,m_transmissionInterval);
           }
         else if (GetNumPacketsReceived () > 1 && m_transmissionInterval == 1)
          {
              m_transIntervalList.pop_back ();
              
              std::vector<uint16_t>::iterator ci;
              ci = m_transIntervalList.begin();
              m_transIntervalList.insert (ci,m_transmissionInterval);
             if ( GetNumPacketsReceived () > m_transInOneBeacon)
              {
                  m_transInOneBeacon++;
              }
            else if ( GetNumPacketsReceived () < m_transInOneBeacon)
              {
                  m_transInOneBeacon--;
              }
          }
         else if (GetNumPacketsReceived () == 1)
          {
             m_transInOneBeacon = 1;
             //m_transmissionIntervalMax = currentId - m_snesorUpdatInfo.preSuccessId;
              m_transmissionInterval = currentId - m_snesorUpdatInfo.preSuccessId;
              m_transIntervalList.pop_back ();
              
              std::vector<uint16_t>::iterator ci;
              ci = m_transIntervalList.begin();
              m_transIntervalList.insert (ci,m_transmissionInterval);
          }

         //NS_LOG_UNCOND ("aid = " << m_aid << " ,m_transmissionIntervalMin = " << m_transmissionIntervalMin << ", m_transmissionIntervalMin = " << m_transmissionIntervalMax << " , GetNumPacketsReceived = " << GetNumPacketsReceived ()<< ", m_transmissionInterval = " << m_transmissionInterval);
     }
    else if (m_transmissionSuccess == true && m_snesorUpdatInfo.preTrySuccess == false)
     {
        //m_transmissionIntervalMin = m_snesorUpdatInfo.CurrentUnSuccessId - m_snesorUpdatInfo.preSuccessId + 1;
        //necessary to update Min here, in order to update min properly when first packets received.
        //m_transmissionIntervalMax = currentId - m_snesorUpdatInfo.lastTryBFpreSuccessId - 1;
         //m_transmissionIntervalMax = currentId - m_snesorUpdatInfo.preSuccessId;
         m_transmissionInterval = currentId - m_snesorUpdatInfo.preSuccessId;
         m_index = 0;
         uint16_t intervalsum = m_transmissionInterval;
         /*
         for (std::vector<uint16_t>::iterator ci = m_transIntervalList.begin(); ci != m_transIntervalList.end(); ci++)
         {
             intervalsum = intervalsum + *ci;
             //NS_LOG_UNCOND ("aid = " << GetAid () <<  ",  list = " << *ci);
         }

         m_transmissionInterval = intervalsum/(m_transIntervalListSize + 1); */
         m_transIntervalList.pop_back ();
         
         std::vector<uint16_t>::iterator ci;
         ci = m_transIntervalList.begin();
         m_transIntervalList.insert (ci,m_transmissionInterval);
     }
    else
     {
        //m_transmissionIntervalMin = currentId - m_snesorUpdatInfo.CurrentSuccessId + 1;
        //m_transmissionIntervalMax remains
         m_transInOneBeacon = 1;

        m_index++;
        m_transmissionInterval = currentId - m_snesorUpdatInfo.preSuccessId;
        m_transmissionInterval = m_transmissionInterval + 2*m_index - 1;
        m_transIntervalList.pop_back ();
         
         std::vector<uint16_t>::iterator ci;
         ci = m_transIntervalList.begin();
         m_transIntervalList.insert (ci,m_transmissionInterval);
     }
    //NS_LOG_UNCOND ("aid = " << GetAid () <<  ",  m_transmissionInterval = " << m_transmissionInterval << ", m_index = " << m_index << ", m_transIntervalListSize = " << m_transIntervalList.size() << ", m_snesorUpdatInfo.preSuccessId = " << m_snesorUpdatInfo.preSuccessId);

    /*
    if (m_transmissionIntervalMax < m_transmissionIntervalMin)
     {
         //m_transmissionIntervalMax = 2 * m_transmissionIntervalMin;
         m_transmissionIntervalMax = m_transmissionIntervalMin;
     }*/
    //NS_LOG_UNCOND ("aid = " << m_aid << " ,m_transmissionIntervalMin = " << m_transmissionIntervalMin << ", m_transmissionIntervalMax = " << m_transmissionIntervalMax );
    //m_transmissionInterval = (m_transmissionIntervalMin + m_transmissionIntervalMax)/2;
    uint64_t m_nextId = m_snesorUpdatInfo.CurrentSuccessId + m_transmissionInterval; ////** AP update info after RAW ends(right before next beacon is sent)
    EstimateNextTransmissionId (m_nextId);
    //NS_LOG_UNCOND (m_snesorUpdatInfo.lastTryBFpreSuccessId << "," << m_snesorUpdatInfo.lastTryBFCurrentSuccessId << ", " << m_snesorUpdatInfo.preSuccessId << ", " << currentId << ", " << m_snesorUpdatInfo.preTrySuccess << ", " << m_snesorUpdatInfo.preUnsuccessId);

    //NS_LOG_UNCOND ("aid = " << GetAid () <<  ",  NextTransmissionId = " << m_nextId << ", currentId = " << currentId);

}


void
S1gRawCtr::calculateSensorNumWantToSend ()
{
    m_numSensorWantToSend = 0;
    m_numSendSensorWant = 0;

    NS_LOG_UNCOND ("currentid update ");

    for (StationsCI it = m_stations.begin(); it != m_stations.end(); it++)
      {
          //NS_LOG_UNCOND ("aid " << (*it)->GetAid () <<  "," << (*it)->GetEstimateNextTransmissionId () << ", " <<currentId);
          //NS_ASSERT ((*it)->GetEstimateNextTransmissionId () >= currentId);
        if ((*it)->GetEstimateNextTransmissionId () <= currentId)
          {
              //NS_LOG_UNCOND ("Sensor = " << (*it)->GetAid () << ", want to send  = " << (*it)->GetTransInOneBeacon ());

            m_numSensorWantToSend++;
            if ((*it)->GetTransInOneBeacon () > (m_beaconInterval-m_beaconOverhead)/m_rawslotDuration - 1)
              {
                  (*it)->SetTransInOneBeacon ((m_beaconInterval-m_beaconOverhead)/m_rawslotDuration - 1);
                  //limit TransInOneBeacon, prevent channel used only by one sensor
              }
            m_numSendSensorWant = m_numSendSensorWant + (*it)->GetTransInOneBeacon ();
          }
      }

     NS_LOG_UNCOND ("m_numSensorWantToSend = " << m_numSensorWantToSend << ", m_numSendSensorWant = " << m_numSendSensorWant);
}

void
S1gRawCtr::calculateMaybeAirtime ()
{
    uint16_t sensortime = m_numSendSensorWant * sensorpacketsize; //to do, define sensorpacketsize

    uint16_t offloadtime = m_numOffloadStaActive * offloadpacketsize; //to do, define offloadpacketsize
    if (sensortime + offloadtime == 0)
     {
        m_maybeAirtimeSensor = 0; //to do, define BeaconInterval;
         return;
     }
    m_maybeAirtimeSensor = m_beaconInterval * sensortime / (sensortime + offloadtime); //to do, define BeaconInterval;
}
/*
void
S1gRawCtr::SetSensorAllowedToSend ()
{
    m_aidList.clear (); //Re assign slot to stations
    uint16_t numAllowed = m_maybeAirtimeSensor/m_rawslotDuration;

    m_numSensorAllowedToSend =  std::min(m_numSensorWantToSend, numAllowed);
    //work here
    //m_numSensorAllowedToSend =  m_numSensorWantToSend; //to change
    NS_LOG_UNCOND ("-------------------------------------------start");
    NS_LOG_UNCOND ("SetSensorAllowedToSend () = " << m_numSensorAllowedToSend );

    for (StationsCI it = m_stations.begin(); it != m_stations.end(); it++)
     {
        if ((*it)->GetEstimateNextTransmissionId () == currentId)
         {
            if (m_aidList.size () == m_numSensorAllowedToSend)
              {
                  (*it)->EstimateNextTransmissionId (currentId+1);
              }
             else
              {
                 m_aidList.push_back((*it)->GetAid ());
                 std::vector<uint16_t>::iterator position = LookupLastTransmission ((*it)->GetAid ());
                  m_lastTransmissionList.erase (position);
                  m_lastTransmissionList.push_back ((*it)->GetAid ());

              }
             ////put first m_numSensorAllowedToSend stations to m_aidList. To do, choose stations based on last transmission time.
         }
     }
}*/


 void
 S1gRawCtr::SetSensorAllowedToSend ()
 {
   m_aidList.clear (); //Re assign slot to stations
   uint16_t numAllowed = m_maybeAirtimeSensor/m_rawslotDuration;

   m_numSendSensorAllowed  =  std::min(m_numSendSensorWant, numAllowed); //replace m_numSensorAllowedToSend with m_numSendSensorAllowed
   m_numSendSensorAllowed =  std::min(m_numSendSensorAllowed, MaxSlotForSensor);
   //m_numSensorAllowedToSend =  std::min(m_numSensorWantToSend, numAllowed);
   NS_LOG_UNCOND ("-------------------------------------------start");
   NS_LOG_UNCOND ("m_numSendSensorWant = " << m_numSendSensorWant << ", numAllowed based on faireness = " << numAllowed);

   uint32_t SendNum = 0;
     
   std::vector<uint16_t>::iterator it = m_lastTransmissionList.begin();
   //for (std::vector<uint16_t>::iterator it = m_lastTransmissionList.begin(); it != m_lastTransmissionList.end(); it++)
    for (uint16_t i = 0; i < m_lastTransmissionList.size (); i++)
     {
         Sensor * stationTransmit;
         if (LookupSensorSta (*it) == nullptr) //disassociated station
           {
             m_lastTransmissionList.erase (it);
             it++;
             continue;
           }
         stationTransmit = LookupSensorSta (*it);
         it++;
       if (stationTransmit->GetEstimateNextTransmissionId () <= currentId)
        {
           if (SendNum == m_numSendSensorAllowed)
             {
                stationTransmit->EstimateNextTransmissionId (currentId+1);
                //Postpone transmission to next interval
                //NS_LOG_UNCOND ("aid = " << stationTransmit->GetAid () << " not allwed to send, since its full ");
                //NS_LOG_UNCOND ("update NextTransmissionId to = " << currentId+1);
             }
           else if ( SendNum + stationTransmit->GetTransInOneBeacon () > m_numSendSensorAllowed)
            {
                stationTransmit->EstimateNextTransmissionId (currentId+1);
                //Postpone transmission to next interval
                //NS_LOG_UNCOND ("aid = " << stationTransmit->GetAid () << " not allwed to send, since transmit number is " << stationTransmit->GetTransInOneBeacon () );
                //NS_LOG_UNCOND ("update NextTransmissionId to = " << currentId+1);

                uint8_t numleft = m_numSendSensorAllowed - SendNum;
                if (numleft > 0)
                 {
                     it--;
                     stationTransmit->SetTransInOneBeacon (numleft);
                     m_aidList.push_back(stationTransmit->GetAid ());
                     std::vector<uint16_t>::iterator position = LookupLastTransmission (stationTransmit->GetAid ());
                     m_lastTransmissionList.erase (position);
                     m_lastTransmissionList.push_back (stationTransmit->GetAid ());
                     SendNum = SendNum + numleft;
                     //NS_LOG_UNCOND ("reset Trans number to = " << numleft << " to send");
                 }
            }
           else
            {
               it--;
               m_aidList.push_back(stationTransmit->GetAid ());
               std::vector<uint16_t>::iterator position = LookupLastTransmission (stationTransmit->GetAid ());
               m_lastTransmissionList.erase (position);
               m_lastTransmissionList.push_back (stationTransmit->GetAid ());
               SendNum = SendNum + stationTransmit->GetTransInOneBeacon ();
               //NS_LOG_UNCOND ("aid = " << stationTransmit->GetAid () << " allowed to send");

            }
         ////put first send stations to m_aidList, choose stations based on last transmission time.
        }
    }

    m_numSendSensorAllowed = SendNum;
    NS_LOG_UNCOND ("m_numSendSensorAllowed = " << m_numSendSensorAllowed );

 }

std::vector<uint16_t>::iterator
S1gRawCtr::LookupLastTransmission (uint16_t aid)
{

    for (std::vector<uint16_t>::iterator it = m_lastTransmissionList.begin(); it != m_lastTransmissionList.end(); it++)
    {
       // NS_LOG_UNCOND ("m_lastTransmissionList = " << *it);
    }

  for (std::vector<uint16_t>::iterator it = m_lastTransmissionList.begin(); it != m_lastTransmissionList.end(); it++)
    {
        if (aid == *it)
        {
            return it;
        }
    }

  NS_ASSERT ("ERROR OCUURS");
}





//offload
void
S1gRawCtr::UdpateOffloadStaInfo (std::vector<uint16_t> m_OffloadList, std::vector<uint16_t> m_receivedAid, std::string outputpath)
//to do
//need to change Ap-wifi-mac.cc to get numsensor info
//need to get successful transmission info.
//this info can be got from static configuration or via association, should from association
{
    //initialization
    //if sorted queue is empty, put all offload stations into queue

    /*uint16_t offloadstaNum = m_OffloadList.size (); //need to be improved
    if (m_offloadStations.size() < offloadstaNum)
    {
        for (uint16_t i = m_offloadStations.size(); i < offloadstaNum; i++)
        {
            OffloadStation * m_offloadSta = new OffloadStation;
            m_offloadSta->SetAid (6); //for test
            m_offloadSta->SetOffloadStaActive (true);
            m_offloadSta->IncreaseFailedTransmissionCount (0);
            m_offloadStations.push_back (m_offloadSta); // should create a Dispose function?
            NS_LOG_UNCOND ("m_offloadStations.size () = " << m_offloadStations.size ());

        }
        //return;
    } should be removed*/


    std::string ApNode;
    std::ofstream outputfile;
    std::ostringstream APId;


    for (std::vector<uint16_t>::iterator ci = m_OffloadList.begin(); ci != m_OffloadList.end(); ci++)
     {
        if (LookupOffloadSta (*ci) == nullptr)
          {
              OffloadStation * m_offloadSta = new OffloadStation;
              m_offloadSta->SetAid (*ci);
              m_offloadSta->SetOffloadStaActive (true);
              m_offloadSta->IncreaseFailedTransmissionCount (0);
              m_offloadStations.push_back (m_offloadSta); // should create a Dispose function?
              NS_LOG_UNCOND ("m_offloadStations.size () = " << m_offloadStations.size ());

              APId.clear ();
              APId.str ("");
              APId << (*ci);
              ApNode = APId.str();
              sensorfile = outputpath + ApNode + ".txt";
              outputfile.open (sensorfile, std::ios::out | std::ios::trunc);
              outputfile.close();
          }
     }

    //update active offload stations' info.
    for (std::vector<uint16_t>::iterator it = m_aidOffloadList.begin(); it != m_aidOffloadList.end(); it++)
    {
        OffloadStation * OffloadStaTransmit = LookupOffloadSta (*it);

        for (std::vector<uint16_t>::iterator ci = m_receivedAid.begin(); ci != m_receivedAid.end(); ci++)
        {
            if (*ci == *it)
            {
                //NS_LOG_UNCOND ("stations of aid " << *it << " received, " << *ci);
                //output to files.
                APId.clear ();
                APId.str ("");
                APId << (*it);
                ApNode = APId.str();
                sensorfile = outputpath + ApNode + ".txt";
                outputfile.open (sensorfile, std::ios::out | std::ios::app);
                outputfile << currentId << "\t" << "1" << "\t" << "1" << "\n";
                outputfile.close();
                //
                OffloadStaTransmit->SetTransmissionSuccess (true);
                OffloadStaTransmit->IncreaseFailedTransmissionCount (1);
                goto FailedMax;
            }
        }

        //output to files.
        //NS_LOG_UNCOND ("stations of aid " << *it << " not received");
        APId.clear ();
        APId.str ("");
        APId << (*it);
        ApNode = APId.str();
        sensorfile = outputpath + ApNode + ".txt";
        outputfile.open (sensorfile, std::ios::out | std::ios::app);
        outputfile << currentId << "\t" << "1" << "\t" << "0" << "\n";
        outputfile.close();
        //

        OffloadStaTransmit->SetTransmissionSuccess (false);
        OffloadStaTransmit->IncreaseFailedTransmissionCount (0);
    FailedMax:
        if (OffloadStaTransmit->GetFailedTransmissionCount () == m_offloadFailedMax)
            OffloadStaTransmit->SetOffloadStaActive (false);

        OffloadStaTransmit->SetOffloadStaActive (true); //for test
    }
}

void
S1gRawCtr::calculateActiveOffloadSta ()
{
    m_numOffloadStaActive = 0;
    for (OffloadStationsCI it = m_offloadStations.begin(); it != m_offloadStations.end(); it++)
    {
        if ((*it)->GetOffloadStaActive () == true)
            m_numOffloadStaActive++;
    }
}

void
S1gRawCtr::SetOffloadAllowedToSend ()
{
    m_aidOffloadList.clear (); //Re assign slot to stations
    m_offloadRawslotDuration = (m_slotDurationCount*120)+500;
    uint16_t numAllowed = ((m_beaconInterval-m_beaconOverhead) - m_numSendSensorAllowed * m_rawslotDuration)/m_offloadRawslotDuration;
    //NS_LOG_UNCOND ("SetOffloadAllowedToSend,  numAllowed= " << numAllowed << ", m_numOffloadStaActive = " << m_numOffloadStaActive);
    if (numAllowed == 0)
      {
         numAllowed = 1;
      }
    m_numOffloadAllowedToSend =  std::min(m_numOffloadStaActive, numAllowed);
    if (m_numOffloadAllowedToSend == 0)
      {
          m_offloadRawslotDuration = ((m_beaconInterval-m_beaconOverhead) - m_numSendSensorAllowed * m_rawslotDuration);
          return;
      }

    if (m_numOffloadAllowedToSend <= numAllowed)
      {
        m_offloadRawslotDuration = ((m_beaconInterval-m_beaconOverhead) - m_numSendSensorAllowed * m_rawslotDuration)/m_numOffloadAllowedToSend;
      }
    //m_numOffloadAllowedToSend = 1; //for test
    NS_LOG_UNCOND ("SetOffloadAllowedToSend ()= " << m_numOffloadAllowedToSend);

    for (OffloadStationsCI it = m_offloadStations.begin(); it != m_offloadStations.end(); it++)
    {
        if ((*it)->GetOffloadStaActive ())
         {
            m_aidOffloadList.push_back((*it)->GetAid ()); //put first send stations to m_aidList. To do, choose stations based on last transmission time.
            if (m_aidOffloadList.size () == m_numOffloadAllowedToSend)
                return;
         }
    }
}

RPS
S1gRawCtr::GetRPS ()
{
 if (RpsIndex < rpslist.rpsset.size())
    {
        m_rps = rpslist.rpsset.at(RpsIndex);
        NS_LOG_DEBUG ("< RpsIndex =" << RpsIndex);
        RpsIndex++;
    }
  else
    {
        m_rps = rpslist.rpsset.at(0);
        NS_LOG_DEBUG ("RpsIndex =" << RpsIndex);
        RpsIndex = 1;
    }
  return *m_rps;
}

bool
S1gRawCtr::IsInfoAvailableForAllSta()
{
	for (CriticalStationsCI ci = this->m_criticalStations.begin(); ci != m_criticalStations.end(); ci++)
	{
		SensorActuator * sta = LookupCriticalSta ((*ci)->GetAid());
		if (sta->m_tInterval == Time ())
			return false;
	}
	return true;
}

//Second attempt
bool
S1gRawCtr::OptimizeRaw (std::vector<uint16_t> criticalList, std::vector<uint16_t> sensorList, uint32_t m, uint64_t BeaconInterval, RPS *prevRps, pageSlice pageslice, uint8_t dtimCount, Time tProcessing, std::string outputpath, Time simulationTime)
{
	if (!IsInfoAvailableForAllSta())
	    	 return false;
	else if (m_startOptimalOpp == Time ())
	{
		m_startOptimalOpp = Simulator::Now();
	}
	uint32_t n = criticalList.size();
	try {
		// Create an environment
		GRBEnv env = GRBEnv();

		std::string pat = "./././optimization/LogOpt_n=" + std::to_string(n) + "_nSensors=" + std::to_string(sensorList.size()) + "_m=" + std::to_string(m) + "_BI=" + std::to_string(BeaconInterval) + "_tProcessingms=" + std::to_string(tProcessing.GetMilliSeconds()) + "___" + outputpath + ".log";

		env.set("LogFile", pat); // "./././results-coap/rawOpt.log"
		env.start();

		GRBVar w[m][n];
		GRBVar c[m];
		GRBVar u[m][n];
		GRBVar d[m][n];
		GRBVar tstart[m];
		GRBVar tend[m];
		GRBVar meff;
		GRBVar Tch;
		GRBVar a;
		GRBVar f[m][n];
		GRBVar s[m][n];
		GRBVar r[m];
		GRBVar v[m][m][n];

		// Create an empty model
		GRBModel model = GRBModel(env);

		//Constants that will be used in the model
		uint32_t MaxChannelTime, BeaconTxTimeMin (2040), tPacketTx (5000); //us
	    MaxChannelTime = BeaconInterval - BeaconTxTimeMin; //us
	    std::sort(criticalList.begin(), criticalList.end());

	    std::fstream ostr;
	    std::string path = "./optimization/OptInfo_n=" + std::to_string(n) + "_nSensors=" + std::to_string(sensorList.size()) + "_m=" + std::to_string(m) + "_BI=" + std::to_string(BeaconInterval) + "_tProcessingms=" + std::to_string(tProcessing.GetMilliSeconds()) + "___" + outputpath + ".log";
	    ostr.open(path.c_str(), std::fstream::out | std::fstream::app);
	    ostr << ">>Optimization started at "  << m_startOptimalOpp.GetNanoSeconds() << ", Now =" << Simulator::Now() << " ns." << std::endl;
	    ostr << "INPUT INFORMATION" << std::endl;
	    ostr << "BI = " << BeaconInterval << ", n = " << n << ", m = " << m << ", t^P_TX = " << tPacketTx << ", tProcessing=" << tProcessing.GetMicroSeconds() << std::endl;
	    ostr << "prev RAW = " << std::endl;
	    prevRps->Print(ostr);


	    uint32_t scheduledUlTotal (0);
	    std::map<uint16_t, bool> pagedAids, outsdandingAids;
	    for (int h=0; h < criticalList.size(); h++)
	    {
	    	uint16_t aid = criticalList[h];
	    	SensorActuator * sta = LookupCriticalSta (aid);
	    	ostr << "\n STA #" << h << ", AID=" << aid << ", UL to be sent by next BI = " << sta->m_scheduledUplinkPackets + sta->m_outstandingUplinkPackets
	    			<< ", outstanding DL = " << sta->m_numOutstandingDl << ", m_pendingDownlinkPackets = " << sta->m_pendingDownlinkPackets << ", m_paged = "
	    			<< sta->m_paged << ", DT BI-tSENT = " << Simulator::Now().GetMicroSeconds() - sta->m_tSent.GetMicroSeconds() << ", sta->m_deltaT us = " << sta->m_deltaT.GetMicroSeconds()<< ", t_sent us = " << sta->m_tSent.GetMicroSeconds() << ", t_interval = " << sta->m_tInterval.GetMicroSeconds();;
	    }
	    std::ostringstream vname;
	    vname.str("");
	    vname << "meff";
	    meff = model.addVar(0.0, m, 0.0,  GRB_INTEGER, vname.str());
	    vname.str("");
	    vname << "a";
	    a = model.addVar(87, ceil((14 + 8 * (65 + 1 + 63 + 8 + 6*m)) / 12.), 0.0,  GRB_INTEGER, vname.str());
	    vname.str("");
	    vname << "Tch";
	    Tch = model.addVar(0.0, MaxChannelTime, 0.0,  GRB_INTEGER, vname.str());
	    for (int i = 0; i < m; i++)
	    {
	    	vname.str("");
	    	vname << "c" << i;
	    	c[i] = model.addVar(0.0, MaxChannelTime, 0.0,  GRB_INTEGER, vname.str()); //upper bound= 246140.0 us in text
	    	vname.str("");
	    	vname << "tstart" << i;
	    	tstart[i] = model.addVar(Simulator::Now().GetMicroSeconds(), Simulator::Now().GetMicroSeconds() + BeaconInterval, 0.0,  GRB_INTEGER, vname.str());
	    	vname.str("");
	    	vname << "tend" << i;
	    	tend[i] = model.addVar(Simulator::Now().GetMicroSeconds(), Simulator::Now().GetMicroSeconds() + BeaconInterval, 0.0,  GRB_INTEGER, vname.str());
	    	vname.str("");
	    	vname << "r" << i;
	    	r[i] = model.addVar(0.0, 1.0, 0.0,  GRB_BINARY, vname.str());

	    	for (int h = 0; h < n; h++)
	    	{
	    		vname.str("");
	    		vname << "w" << i << "." << h;
	    		w[i][h] = model.addVar (0.0, 1.0, 0.0, GRB_BINARY, vname.str());
	    		vname.str("");
	    		vname << "u" << i << "." << h;
	    		u[i][h] = model.addVar (0.0, 1.0, 0.0, GRB_BINARY, vname.str());
	    		vname.str("");
	    		vname << "d" << i << "." << h;
	    		d[i][h] = model.addVar (0.0, 1.0, 0.0, GRB_BINARY, vname.str());
	    		vname.str("");
	    		vname << "s" << i << "." << h;
	    		s[i][h] = model.addVar(0.0, m/2, 0.0,  GRB_INTEGER, vname.str());
	    		vname.str("");
	    		vname << "f" << i << "." << h;
	    		f[i][h] = model.addVar(0.0, m/2, 0.0,  GRB_INTEGER, vname.str());
	    		if (i <= m - 2)
	    		{
	    			for (int j = i + 1; j < m; j++)
	    			{
	    				vname.str("");
	    				vname << "v" << i << "." << j << "." << h;
	    				v[i][j][h] = model.addVar (0.0, 1.0, 0.0, GRB_BINARY, vname.str());
	    			}
	    		}
	    	}
	    }

	    // Objective + Constraint 40 + Constraint 44 +  + Constraint 49
	    GRBQuadExpr obj (0);
	    for (int i = 0; i < m; i++)
	    {
	    	vname.str("");
	    	vname << "CON_40_" << i;
	    	GRBLinExpr sumwh (0);
	    	for (int h = 0; h < n; h++)
	    	{
	    		obj += w[i][h]*c[i];
	    		sumwh += w[i][h];
	    	}
	    	model.addConstr(sumwh <= 1, vname.str());
	    	vname.str("");
	    	vname << "CON_44.left_" << i;
	    	model.addQConstr(r[i] * c[i] + r[i] >= c[i] , vname.str());
	    	vname.str("");
	    	vname << "CON_44.right_" << i;
	    	model.addQConstr(r[i] * c[i] + r[i] <= 1.9999 * c[i] + 0.9999, vname.str());
	    	vname.str("");
	    	vname << "CON_49.left_" << i;
	    	model.addConstr(tProcessing.GetMicroSeconds() * sumwh <= c[i], vname.str());
	    	vname.str("");
	    	vname << "CON_49.right_" << i;
	    	model.addConstr(246140 >= c[i], vname.str());
	    }
	    model.setObjective(obj, GRB_MINIMIZE);

	    // Constraint 41 + Constraint 42 + Constraint 43 + Constraint 47 + Constraint 48
	    for (int h=0; h < n; h++)
	    {
	    	GRBLinExpr sumwi (0);
	    	for (int i=0; i < m; i++)
	    	{
	    		sumwi += w[i][h];
	    		GRBLinExpr sumwUptoI (0);
	    		for (int k=0; k < i; k++)
	    			sumwUptoI += w[k][h];
	    		vname.str("");
	    		vname << "CON_42.left_" << i << "." << h;
	    		model.addConstr(f[i][h] >= 0.5 * sumwUptoI - 0.5, vname.str());
	    		vname.str("");
	    		vname << "CON_42.right_" << i << "." << h;
	    		model.addConstr(f[i][h] <= 0.5 * sumwUptoI, vname.str());
	    		vname.str("");
	    		vname << "CON_43.left_" << i << "." << h;
	    		model.addConstr(s[i][h] >= 0.5 * sumwUptoI, vname.str());
	    		vname.str("");
	    		vname << "CON_43.right_" << i << "." << h;
	    		model.addConstr(s[i][h] <= 0.5 + 0.5 * sumwUptoI, vname.str());
	    	}
	    	uint32_t leqNumPackets (0), geqNumPackets (0);
	    	uint16_t aid = criticalList[h];
	    	SensorActuator * sta = LookupCriticalSta (aid);
	    	int ulPacketsh = sta->m_scheduledUplinkPackets + sta->m_outstandingUplinkPackets;
	    	//scheduledUlTotal += ulPacketsh;
	    	uint32_t prevBeaconTxTime (5000);
	    	int epsh = sta->m_tSent.GetMicroSeconds() + sta->m_tInterval.GetMicroSeconds() * (ulPacketsh + 1) - 2 * tPacketTx - tProcessing.GetMicroSeconds() < Simulator::Now().GetMicroSeconds() + BeaconInterval + prevBeaconTxTime ? 1 : 0;
	    	geqNumPackets += 2 * (ulPacketsh - std::ceil((ulPacketsh + epsh) * 1.0 / (ulPacketsh + epsh + 1.0)) + epsh);

	    	if (sta->m_numOutstandingDl || sta->m_pendingDownlinkPackets)
	    		pagedAids.insert(std::pair<uint16_t, bool>(h, 1));
	    	else
	    		pagedAids.insert(std::pair<uint16_t, bool>(h, 0));

	    	bool outstandingUl = sta->m_tSent + sta->m_tInterval <= Simulator::Now();
	    	if (outstandingUl)
	    		outsdandingAids.insert(std::pair<uint16_t, bool>(h, 1));
	    	else
	    		outsdandingAids.insert(std::pair<uint16_t, bool>(h, 0));
	    	leqNumPackets += (uint32_t)pagedAids.find(h)->second + 2 * (sta->m_outstandingUplinkPackets + sta->m_scheduledUplinkPackets);
	    	geqNumPackets += (uint32_t)pagedAids.find(h)->second;
	    	vname.str("");
	    	vname << "CON_41_" << h;
	    	model.addConstr(sumwi >= 0, vname.str());
	    	// Constraint 47
	    	vname.str("");
	    	vname << "CON_47_" << h;
	    	model.addConstr(sumwi <= leqNumPackets, vname.str());
	    	// Constraint 48
	    	vname.str("");
	    	vname << "CON_48_" << h;
	    	model.addConstr(sumwi >= geqNumPackets
	    			//2 * scheduledUlTotal + 2 * std::count_if(outsdandingAids.begin(), outsdandingAids.end(), [](const std::pair<uint16_t, bool>& a){return a.second;}) - 2 - std::count_if(pagedAids.begin(), pagedAids.end(), [](const std::pair<uint16_t, bool>& a){return a.second;})
	    			, vname.str()); //2 * scheduledUlTotal
	    }
	    // Constraint 45 + Constraint 46
	    GRBLinExpr sumri (0), sumci (0);
	    for (int i=0; i < m; i++)
	    {
	    	sumri += r[i]; // for Constraint 50
	    	sumci += c[i]; // for Constraint 52
	    	for (int h=0; h < n; h++)
	    	{
	    		uint16_t ph (pagedAids.find(h)->second);
	    		vname.str("");
	    		vname << "CON_45_" << i << "." << h;
	    		GRBLinExpr uBracket (1 - s[i][h] + f[i][h] - ph + 2 * ph * s[i][h] - 2 * ph * f[i][h]);
	    		model.addQConstr(u[i][h] == w[i][h] * uBracket, vname.str());
	    		GRBLinExpr dBracket (s[i][h] - f[i][h] + ph - 2 * ph * s[i][h] + 2 * ph * f[i][h]);
	    		vname.str("");
	    		vname << "CON_46_" << i << "." << h;
	    		model.addQConstr(d[i][h] == w[i][h] * dBracket, vname.str());
	    	}
	    }
	    // Constraint 50
	    vname.str("");
	    vname << "CON_50_";
	    model.addConstr(meff == sumri, vname.str());

	    // Constraint 51
	    uint32_t pagedSubblocks (0), pageBitmapLen(0);
	    // Assumption: only 1 TIM - this will not work for multiple TIMs. Paged subblocks then must be counted separately for each TIM
	    auto allToPage = m_aidListPaged;
	    allToPage.insert(allToPage.end(), m_aidForcePage.begin(), m_aidForcePage.end());
	    if (allToPage.size())
	    {
	    	std::sort(allToPage.begin(), allToPage.end());
	    	pagedSubblocks = std::unique(allToPage.begin(), allToPage.end(), [](const uint16_t a, const uint16_t b){return ((a >> 3) & 0x07) == ((b >> 3) & 0x07); }) - allToPage.begin();
	    	pageBitmapLen =1; //TODO this can be 0,1,2,3,4 and should be read from the new page slice that will be constructed in this beacon
	    }
	    uint32_t numTimPaged (allToPage.size() > 0 ? 1 : 0);
	    uint32_t timSize (4 + numTimPaged * (1 + 2*31 + pagedSubblocks));
	    GRBLinExpr beaconSize = 27 + 26 + timSize + pageBitmapLen + 6 + 2 + 6 * meff;
	    GRBLinExpr ceilArg = (14 + 8 * beaconSize) / 12.;
	    //GRBLinExpr beaconTxDuration = 40 + 240 + 40 * ((14 + 8 * (65 + pageBitmapLen + numTimPaged * (63 + pagedSubblocks) + meff)) / 12) + 1000; //us
	    vname.str("");
	    vname << "CON_51a.left";
	    model.addConstr(ceilArg <= a, vname.str());
	    vname.str("");
	    vname << "CON_51a.right";
	    model.addConstr(a <= ceilArg + 0.9999, vname.str());

	    vname.str("");
	    vname << "CON_51";
	    model.addConstr(Tch == BeaconInterval - 240 - 40 * a, vname.str());


	    // Constraint 52
	    //GRBLinExpr Tch = BeaconInterval - beaconTxDuration;
	    vname.str("");
	    vname << "CON_52_";
	    model.addConstr(Tch >= sumci, vname.str());

	    GRBLinExpr sumcUptoi (Simulator::Now().GetMicroSeconds());
	    for (int i=0; i < m; i++)
	    {
	    	if (i > 0)
	    		sumcUptoi += c[i - 1];

	    	// Constraint 53
	    	vname.str("");
	    	vname << "CON_53_" << i;
	    	model.addConstr(tstart[i] == sumcUptoi + BeaconInterval - Tch, vname.str());
	    	// Constraint 54
	    	vname.str("");
	    	vname << "CON_54_" << i;
	    	model.addConstr(tend[i] == tstart[i] + c[i], vname.str());
	    	// Constraint 55
	    	vname.str("");
	    	vname << "CON_55_" << i;
	    	model.addConstr(tstart[i] >= Simulator::Now().GetMicroSeconds() + BeaconInterval - Tch, vname.str());
	    	// Constraint 56
	    	vname.str("");
	    	vname << "CON_56_" << i;
	    	model.addConstr(tstart[i] <= Simulator::Now().GetMicroSeconds() + BeaconInterval, vname.str());
	    	// Constraint 59
	    	vname.str("");
	    	vname << "CON_59_" << i;
	    	model.addConstr(tend[i] >= Simulator::Now().GetMicroSeconds() + BeaconInterval - Tch, vname.str());
	    	// Constraint 60
	    	vname.str("");
	    	vname << "CON_60_" << i;
	    	model.addConstr(tend[i] <= Simulator::Now().GetMicroSeconds() + BeaconInterval, vname.str());

	    	// Constraint 57 + Constraint 61 + Constraint 63 + Constraint 65
	    	GRBQuadExpr expr57 (0), expr61 (0), expr63 (0), expr65 (0);
	    	for (int h = 0; h < n; h++)
	    	{
	    		uint16_t aid = criticalList[h];
	    		SensorActuator * sta = LookupCriticalSta (aid);
	    		uint64_t num = sta->m_tSent.GetMicroSeconds() +  2 * tPacketTx + tProcessing.GetMicroSeconds();
	    		expr57 += u[i][h] * f[i][h] * sta->m_tInterval.GetMicroSeconds() + u[i][h] * num;
	    		expr65 += d[i][h] * s[i][h] * sta->m_tInterval.GetMicroSeconds() + d[i][h] * num;

	    		num = sta->m_tSent.GetMicroSeconds() + sta->m_tInterval.GetMicroSeconds() + tPacketTx;
	    		expr61 += u[i][h] * f[i][h] * sta->m_tInterval.GetMicroSeconds() + u[i][h] * num;

	    		num = sta->m_tSent.GetMicroSeconds() + tPacketTx;
	    		expr63 += d[i][h] * s[i][h] * sta->m_tInterval.GetMicroSeconds() + d[i][h] * num;

	    		// Constraint 58
	    		num = sta->m_tSent.GetMicroSeconds() +  2 * sta->m_tInterval.GetMicroSeconds() - 2 * tPacketTx - tProcessing.GetMicroSeconds();
	    		vname.str(""); vname << "CON_58_" << i << "." << h;
	    		model.addQConstr(tstart[i] * u[i][h] <= num + sta->m_tInterval.GetMicroSeconds() * f[i][h], vname.str());

	    		// Constraint 62
	    		num = sta->m_tSent.GetMicroSeconds() +  2 * sta->m_tInterval.GetMicroSeconds() - tPacketTx;
	    		vname.str(""); vname << "CON_62_" << i << "." << h;
	    		model.addQConstr(tend[i] * u[i][h] <= num + sta->m_tInterval.GetMicroSeconds() * f[i][h], vname.str());

	    		// Constraint 64
	    		num = sta->m_tSent.GetMicroSeconds() +  sta->m_tInterval.GetMicroSeconds() - tPacketTx;
	    		vname.str(""); vname << "CON_64_" << i << "." << h;
	    		model.addQConstr(tstart[i] * d[i][h] <= num + sta->m_tInterval.GetMicroSeconds() * s[i][h], vname.str());

	    		// Constraint 66
	    		num = sta->m_tSent.GetMicroSeconds() + sta->m_tInterval.GetMicroSeconds();
	    		vname.str(""); vname << "CON_66_" << i << "." << h;
	    		model.addQConstr(tend[i] * d[i][h] <= num + sta->m_tInterval.GetMicroSeconds() * s[i][h], vname.str());


	    	}
	    	vname.str(""); vname << "CON_57_" << i;
	    	model.addQConstr(tstart[i] >= expr57, vname.str());
	    	vname.str(""); vname << "CON_61_" << i;
	    	model.addQConstr(tend[i] >= expr61, vname.str());
	    	vname.str(""); vname << "CON_63_" << i;
	    	model.addQConstr(tstart[i] >= expr63, vname.str());
	    	vname.str(""); vname << "CON_65_" << i;
	    	model.addQConstr(tend[i] >= expr65, vname.str());
	    }

	    for (int h = 0; h < n; h++)
	    {
	    	uint16_t aid = criticalList[h];
	    	SensorActuator * sta = LookupCriticalSta (aid);
	    	// Constraint 68
	    	uint8_t nRaw = prevRps->GetNumberOfRawGroups();
	    	uint64_t startTime (0);
	    	int g;
	    	for (g = nRaw - 1; g >= 0; g--)
	    	{
	    		auto startaid = prevRps->GetRawAssigmentObj(g).GetRawGroupAIDStart();
	    		auto endaid = prevRps->GetRawAssigmentObj(g).GetRawGroupAIDEnd();
	    		if (startaid <= aid && endaid >= aid)
	    			break;
	    	}
	    	for (int l = 0; l < g; l++)
	    	{
	    		startTime += prevRps->GetRawAssigmentObj(l).GetSlotDuration().GetMicroSeconds() * prevRps->GetRawAssigmentObj(l).GetSlotNum();
	    	}
	    	startTime += Simulator::Now().GetMicroSeconds() - BeaconInterval + m_prevBeaconTxDuration.GetMicroSeconds(); //TODO add previous beacon TX time
	    	auto num = startTime + 2 * tPacketTx + tProcessing.GetMicroSeconds();
	    	ostr << ", prevRawBiStartTime = " << startTime << ", m_prevBeaconTxDuration = " << m_prevBeaconTxDuration.GetMicroSeconds() << std::endl;
	    	for (int i = 0; i < m - 1; i++)
	    	{
	    		vname.str(""); vname << "CON_68_" << i << "." << h;
	    		model.addQConstr(num * d[i][h] <= d[i][h] * tend[i], vname.str());
	    		for (int j = i + 1; j < m; j++)
	    		{
	    			// Constraint 67
	    			GRBLinExpr expr67 (tstart[i] + 2 * tPacketTx + tProcessing.GetMicroSeconds());
	    			vname.str(""); vname << "CON_67_" << i << "." << j << "." << h;
	    			model.addQConstr(v[i][j][h] * expr67 <= d[j][h] * tend[j], vname.str());
	    		}
	    	}
	    }

	    // Save problem
	    model.write("OptRaw_c++.mps");
	    model.write("OptRaw_c++.lp");

	    // Optimize model
	    std::cout << "--------------Time = " << Simulator::Now().GetSeconds() << " s, out of " << simulationTime.GetSeconds() << std::endl;
	    model.optimize();

	    // Status checking
	    int status = model.get(GRB_IntAttr_Status);
	    ostr << "Optimization was stopped with status " << status << std::endl;
	    if (status == GRB_INF_OR_UNBD ||
	    		status == GRB_INFEASIBLE  ||
	    		status == GRB_UNBOUNDED     )
	    {
	    	std::cout << "The model cannot be solved " <<
	    			"because it is infeasible or unbounded" << std::endl;

	    	// do IIS
	    	/*if (Simulator::Now() < simulationTime + Seconds (2))
	    	{
	    		model.computeIIS();
	    		ostr << "\nThe following constraint(s) " << "cannot be satisfied:" << std::endl;
	    		GRBConstr* constr = 0;
	    		constr = model.getConstrs();
	    		for (int i = 0; i < model.get(GRB_IntAttr_NumConstrs); ++i)
	    		{
	    			if (constr[i].get(GRB_IntAttr_IISConstr) == 1)
	    			{
	    				ostr << constr[i].get(GRB_StringAttr_ConstrName) << std::endl;
	    			}
	    		}

	    	}*/
	    	ostr.close();
	    	return false;
	    }
	    if (status != GRB_OPTIMAL) {
	    	ostr.close();
	    	return false;
	    }

/*
	    std::ofstream os;
	    std::string outputpath = "./optimization/";
	    int num = currentId + 1;
	    std::string fname = outputpath + "res_" + std::to_string(num) + ".txt";
	    os.open(fname.c_str(), std::ios::out | std::ios::trunc);
	    os << "Optimization started at " << m_startOptimalOpp.GetNanoSeconds() << ", Now =" << Simulator::Now() << " ns." << std::endl;*/
	    ostr << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;

	    for (int i = 0; i < m; i++)
	    {
	    	ostr << std::left << std::setw(13) << c[i].get(GRB_StringAttr_VarName) << std::setw(3) << " = " << std::setw(10) << c[i].get(GRB_DoubleAttr_X);
	    	for (int h = 0; h < n; h++)
	    	{
	    		ostr << std::left << std::setw(6) << w[i][h].get(GRB_StringAttr_VarName) << std::setw(3) << " = " << std::setw(10) << w[i][h].get(GRB_DoubleAttr_X);
	    		ostr << std::left << std::setw(6) << u[i][h].get(GRB_StringAttr_VarName) << std::setw(3) << " = " << std::setw(10) << u[i][h].get(GRB_DoubleAttr_X);
	    		ostr << std::left << std::setw(6) << d[i][h].get(GRB_StringAttr_VarName) << std::setw(3) << " = " << std::setw(10) << d[i][h].get(GRB_DoubleAttr_X);
	    	}
	    	ostr << std::endl;
	    }
	    ostr << std::endl;
	    //os << Tch.get(GRB_StringAttr_VarName) << " = " << Tch.get(GRB_DoubleAttr_X) << std::endl;
	    ostr << meff.get(GRB_StringAttr_VarName) << " = " << meff.get(GRB_DoubleAttr_X) << std::endl;
	    ostr << Tch.get(GRB_StringAttr_VarName) << " = " << Tch.get(GRB_DoubleAttr_X) << std::endl;
	    ostr.close();


	    std::vector<Slot> slots;
	    Time acc = Time ();
	    std::sort(sensorList.begin(), sensorList.end());
	    for (int i = 0; i < m; i++)
	    {
	    	Slot s;
	    	uint64_t duration = c[i].get(GRB_DoubleAttr_X);
	    	if (duration > 0)
	    	{

	    		for (int h = 0; h < n; h++)
	    		{
	    			if (w[i][h].get(GRB_DoubleAttr_X) > 0.01)
	    			{
	    				s.SetAid(h+1);
	    				s.SetSlotStartTime(acc);
	    				uint32_t count = ((uint64_t)duration - 500) / 120;
	    				if (count < 256)
	    					s.SetSlotFormat(0);
	    				else if (count < 2048)
	    					s.SetSlotFormat(1);
	    				else
	    				{
	    					NS_LOG_UNCOND ("Too large slot count attempted for critical, c = " << count << ", duration us=" << duration << ", slot i=" << i);
	    					NS_ASSERT (false);
	    				}
	    				s.SetSlotCount(count);
	    			}
	    		}
	    		if (s.GetAid() == 0 && s.GetStartAid() == 0)
	    		{
	    			// for non-critical traffic
	    			s.SetStartAid(*sensorList.begin());
	    			s.SetEndAid(sensorList.back());
	    			s.SetSlotStartTime(acc);
	    			if (duration >= 500)
	    			{
	    				uint16_t count = ((uint64_t)duration - 500) / 120;
	    				if (count < 256)
	    					s.SetSlotFormat(0);
	    				else if (count < 2048)
	    					s.SetSlotFormat(1);
	    				else
	    				{
	    					NS_LOG_UNCOND ("Too large slot count attempted for non-critical, c = " << count << ", duration us=" << duration << ", slot i=" << i);
	    					//NS_ASSERT (false);
	    				}
	    				s.SetSlotCount(count);
	    			}
	    			else
	    			{
	    				//duration is larger than 0 but smaller than 500 (minimal slot duration is 500)
	    				//merge this slot with the previous or next one
	    				bool fixed (false);
	    				auto next = i + 1;
	    				while (next < m && !fixed)
	    				{
	    					//next slot exists, merge with the next slot
	    					for (int h = 0; h < n; h++)
	    					{
	    						if (round(w[next][h].get(GRB_DoubleAttr_X)) == 1.0)
	    						{
	    							duration += c[next].get(GRB_DoubleAttr_X);
	    							s.SetAid(h+1);
	    							s.SetSlotStartTime(acc);
	    							uint32_t count = ((uint64_t)duration - 500) / 120;
	    							if (count < 256)
	    								s.SetSlotFormat(0);
	    							else if (count < 2048)
	    								s.SetSlotFormat(1);
	    							else
	    							{
	    								NS_LOG_UNCOND ("Too large slot count attempted for critical, c = " << count << ", duration us=" << duration << ", slot i=" << i);
	    								NS_ASSERT (false);
	    							}
	    							s.SetSlotCount(count);
	    							fixed = true;
	    							break;
	    						}
	    					}
	    					if (fixed) break;
	    					//next isn't critical
	    					if (duration + c[next].get(GRB_DoubleAttr_X) >= 500 && !fixed)
	    					{
	    						duration += c[next].get(GRB_DoubleAttr_X);
	    						fixed = true;
	    						uint16_t count = ((uint64_t)duration - 500) / 120;
	    						if (count < 256)
	    							s.SetSlotFormat(0);
	    						else if (count < 2048)
	    							s.SetSlotFormat(1);
	    						else
	    						{
	    							NS_LOG_UNCOND ("Too large slot count attempted for non-critical, c = " << count << ", duration us=" << duration << ", slot i=" << i);
	    							NS_ASSERT (false);
	    						}
	    						s.SetSlotCount(count);
	    						break;
	    					}
	    					next++;
	    				}
	    				if (fixed)
	    				{
	    					slots.push_back(s);
	    					acc += MicroSeconds (duration);
	    					i = next;
	    					continue;
	    				}
	    				else
	    				{
	    					//couldn't merge with next slots: 2 possibilities: 1. either next slot doesn't exist or 2. all next slots are non critical andsum of their duration is <500
	    					//either case, we can ignore those slots
	    				}

	    			}
	    		}
	    		slots.push_back(s);

	    		acc += MicroSeconds (duration);
	    	}

	    }
	    int remaining = Tch.get(GRB_DoubleAttr_X) - acc.GetMicroSeconds() - 5000;
	    std::cout << "acc = " << acc.GetMicroSeconds() << ", remaining = " << remaining << std::endl;
	    Slot last;
	    last.SetStartAid(*sensorList.begin());
	    last.SetEndAid(sensorList.back());
	    last.SetSlotStartTime(acc);
	    if (remaining >= 500)
	    {
	    	uint16_t count = (remaining - 500) / 120;
	    	if (count < 256)
	    		last.SetSlotFormat(0);
	    	else if (count < 2048)
	    		last.SetSlotFormat(1);
	    	else
	    	{
	    		NS_LOG_UNCOND ("Too large slot count attempted for non-critical last slot, count = " << count << ", duration us=" << remaining);
	    		//NS_ASSERT (false);
	    	}
	    	last.SetSlotCount(count);
	    	slots.push_back(last);
	    }
	    EncodeRaws (slots);
	    ostr << "Transmission of beacon will take = " << this->GetBeaconTxDuration(criticalList).GetMicroSeconds() << " us" << std::endl;
	    ostr.close();
	    return true;
	  } catch(GRBException e) {
		  std::cout << "Error code = " << e.getErrorCode() << std::endl;
		  std::cout << e.getMessage() << std::endl;
		  return false;
	  } catch(...) {
		  std::cout << "Exception during optimization" << std::endl;
		  return false;
	  }
}
// Beacon duration), before that use NGroup=1 and initialize by ap-wifi-mac
RPS
S1gRawCtr::UpdateRAWGroupping (std::vector<uint16_t> criticalList, std::vector<uint16_t> sensorList, std::vector<uint16_t> offloadList, std::vector<uint16_t> receivedFromAids, std::vector<Time> receivedTimes, std::vector<Time> sentTimes, std::vector<uint16_t> sentToAids, std::vector<uint16_t> enqueuedToAids, std::map<uint16_t,uint16_t> numExpectedDlPacketsForAids, uint64_t BeaconInterval, RPS *prevRps, pageSlice pageslice, uint8_t dtimCount, Time bufferTimeToAllowBeaconToBeReceived, std::string outputpath, Time simulationTime)
 {
     NS_ASSERT ("S1gRawCtr should not be called");
     //gandalf ();
     m_beaconInterval = BeaconInterval;
     /*if (m_prevRps == nullptr && criticalList.size())
     {
    	 //initial
    	 uint32_t count = (m_beaconInterval / 10 - 500) / 120;
    	 uint32_t duration = 500 + count * 120;

    	 //assign RAW slots to critical STAs
    	 //Assumption: All critical stations have aids 1, 2, ..., N whereas sensors have aids from N+1 onwards
    	 if (criticalList.size())
    	 {
    		 auto minaidcrit = *std::min_element(criticalList.begin(), criticalList.end());
    		 auto maxaidcrit = *std::max_element(criticalList.begin(), criticalList.end());
    		 NS_ASSERT (((minaidcrit >> 11) & 0x03) == ((maxaidcrit >> 11) & 0x03));

    		 uint8_t page = maxaidcrit >> 11 & 0x03;
    		 if (!(pageslice.GetPageSliceCount() == 0 && pageslice.GetPageSliceLen() == 1))
    		 {
    			 //more than 1 page slice
    			 auto num = criticalList.back() / (pageslice.GetPageSliceLen() * 64);
    			 NS_LOG_DEBUG ("CRITICAL_SERVICE stations span over " << num + 1 << " page slices (TIMs). Current tim=" << dtimCount + pageslice.GetTIMOffset());
    			 auto currentIndex = (dtimCount + pageslice.GetTIMOffset() > pageslice.GetPagePeriod() - 1) ? 0 : dtimCount + pageslice.GetTIMOffset();
    			 if (currentIndex <= num)
    			 {
    				 //loops are in this tim
    				 uint16_t aid_start = currentIndex == 0 ? 1 : currentIndex * 64;
    				 uint16_t aid_end = maxaidcrit < (currentIndex + 1) * 64 ? maxaidcrit : (currentIndex + 1) * 64 - 1;
    				 for (int r = aid_start; r <= aid_end; r++)
    				 {
    					 RPS::RawAssignment *m_raw = new RPS::RawAssignment; //loop's RAW, unless there is no loops in this TIM
    					 m_raw->SetRawControl(0);  //support paged STA or not
    					 m_raw->SetSlotCrossBoundary(1);
    					 m_raw->SetSlotFormat (1);
    					 uint32_t rawinfo = (r << 13) | (r << 2) | page;
    					 m_raw->SetRawGroup(rawinfo);
    					 m_raw->SetSlotNum(1);
    					 uint32_t count = (m_beaconInterval / 10 - 500) / 120;
    					 if (count < 256)
    						 m_raw->SetSlotFormat(0);
    					 else if (count < 2048)
    						 m_raw->SetSlotFormat(1);
    					 m_raw->SetSlotDurationCount(count);
    					 m_rps->SetRawAssignment(*m_raw);
    					 delete m_raw;
    				 }
    			 }
    			 else
    			 {
    				 //loops are not in this tim; assign to sensors (there is no offload stations in my experiments so I am not considering them)
    				 RPS::RawAssignment *m_raw = new RPS::RawAssignment; //loop's RAW, unless there is no loops in this TIM
    				 m_raw->SetRawControl(0);  //support paged STA or not
    				 m_raw->SetSlotCrossBoundary(1);
    				 m_raw->SetSlotFormat (1);
    				 uint16_t aid_start = currentIndex == 0 ? 1 : currentIndex * 64;
    				 auto maxaidsensor = *std::max_element(sensorList.begin(), sensorList.end());
    				 uint16_t aid_end = maxaidsensor < (currentIndex + 1) * 64 ? maxaidsensor : (currentIndex + 1) * 64 - 1;
    				 uint32_t rawinfo = (aid_end << 13) | (aid_start << 2) | page;
    				 m_raw->SetRawGroup(rawinfo);
    				 m_raw->SetSlotNum(1);
    				 uint32_t count = (m_beaconInterval - bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds() - 500) / 120;
    				 if (count < 256)
    					 m_raw->SetSlotFormat(0);
    				 else if (count < 2048)
    					 m_raw->SetSlotFormat(1);
    				 m_raw->SetSlotDurationCount(count);
    				 m_rps->SetRawAssignment(*m_raw);
    				 delete m_raw;
    			 }


    		 }
    		 else
    		 {
    			 // there is only one pageslice and all stations belong to it
    			 //loops' RAW
    			 uint16_t aid_start = *std::min_element(criticalList.begin(), criticalList.end());
    			 uint16_t aid_end = *std::max_element(criticalList.begin(), criticalList.end());
    			 for (int r = aid_start; r <= aid_end; r++)
    			 {
    				 RPS::RawAssignment *m_raw = new RPS::RawAssignment; //loop's RAW, unless there is no loops in this TIM
    				 m_raw->SetRawControl(0);  //support paged STA or not
    				 m_raw->SetSlotCrossBoundary(1);
    				 m_raw->SetSlotFormat (1);
    				 uint32_t rawinfo = (r << 13) | (r << 2) | page;
    				 m_raw->SetRawGroup(rawinfo);
    				 uint32_t count = (m_beaconInterval / 10 - 500) / 120;
    				 if (count < 256)
    					 m_raw->SetSlotFormat(0);
    				 else if (count < 2048)
    					 m_raw->SetSlotFormat(1);
    				 m_raw->SetSlotDurationCount(count);
    				 //uint32_t numslots;
    				 m_raw->SetSlotNum(1);
    				 uint32_t duration = 500 + 120 * count;
    				 NS_LOG_DEBUG ("bufferTimeToAllowBeaconToBeReceived us = " << bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds());
    				 if (duration * criticalList.size() >= m_beaconInterval - bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds())
    				 {
    					 count = 1 + (2 * 1240 - 500) / 120;
    					 duration = 500 + 120 * count;
    					 m_raw->SetSlotDurationCount(count);
    					 if (duration * criticalList.size() >= m_beaconInterval - bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds())
    					 {
    						 NS_LOG_DEBUG ("CANNOT ASSIGN SEPARATE SLOT TO " << criticalList.size() << " CRITICAL STATIONS!");
    						 //numslots = (m_beaconInterval - bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds()) / (500 + 120 * count);
    						 m_raw->SetSlotNum(1);
    					 }
    				 }
    				 m_rps->SetRawAssignment(*m_raw);
    				 delete m_raw;
    			 }
    			 // rest of the stations' RAW
    			 if (!sensorList.empty())
    			 {
    				 RPS::RawAssignment *m_raw = new RPS::RawAssignment; //loop's RAW, unless there is no loops in this TIM
    				 m_raw->SetRawControl(0);  //support paged STA or not
    				 m_raw->SetSlotCrossBoundary(1);
    				 m_raw->SetSlotFormat (1);
    				 aid_start = *std::min_element(sensorList.begin(), sensorList.end());
    				 aid_end = *std::max_element(sensorList.begin(), sensorList.end());
    				 uint32_t rawinfo = (aid_end << 13) | (aid_start << 2) | page;
    				 m_raw->SetRawGroup(rawinfo);
    				 count = (m_beaconInterval - bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds() - m_rps->GetRawAssigmentObj(0).GetSlotDuration().GetMicroSeconds() * m_rps->GetRawAssigmentObj(0).GetSlotNum() - 500) / 120;
    				 //NS_LOG_UNCOND ("--count=" << count);
    				 if (count < 256)
    					 m_raw->SetSlotFormat(0);
    				 else if (count < 2048)
    					 m_raw->SetSlotFormat(1);
    				 m_raw->SetSlotDurationCount(count);
    				 m_raw->SetSlotNum(1);
    				 m_rps->SetRawAssignment(*m_raw);
    				 delete m_raw;
    			 }

    		 }
    	 }

    	 this->m_prevRps = new RPS;
    	 *m_prevRps = *m_rps;
     }
     else if (m_prevRps != nullptr && criticalList.size())
     {*/
    	 UpdateCriticalStaInfo (criticalList, receivedFromAids, enqueuedToAids, receivedTimes, sentTimes, numExpectedDlPacketsForAids, outputpath);
    	 UpdateSensorStaInfo (sensorList, receivedFromAids, receivedTimes, sentTimes);

    	 delete m_rps;
    	 m_rps = new RPS;


    	 Time tProcessing = MilliSeconds (5);

    	 if (!IsInfoAvailableForAllSta())
    	 {
    		 DistributeStationsToRaws ();
    	 }
    	 else
    	 {
    		 //DistributeStationsToRaws ();
    		 std::cout << std::endl << std::endl;
    		 Time startTime = Simulator::Now();
    		 m_success = OptimizeRaw(criticalList, sensorList, 32, BeaconInterval, prevRps, pageslice, dtimCount, tProcessing, outputpath, simulationTime);


    		 if (!m_success)
    			 DistributeStationsToRaws ();
    		 //NS_LOG_UNCOND ("Start time=" << startTime << "End time=" << Simulator::Now());
    	 }
     //}
     currentId++; //beaconInterval counter

     //ControlRps (criticalList);
     m_rps->Print(std::cout);
     /*NS_LOG_UNCOND (" +++ ALL SLOTS +++" );
     	for (auto& s : allSlots)
     	{
     		NS_LOG_UNCOND ("startAID-endAID=" << (int)s.GetStartAid() << "-" << (int)s.GetEndAid() << "\t start time=" << s.GetSlotStartTime() << "\t duration=" << s.GetSlotDuration());
     	}*/


     m_aidList.clear();
     m_aidListPaged.clear();
     return *m_rps;
}

Time
S1gRawCtr::GetBeaconTxDuration (std::vector<uint16_t> criticalList)
{
	uint32_t beaconSize (27 + 26), pagedSubblocks (0), pageBitmapLen(0);

	// Assumption: only 1 TIM - this will not work for multiple TIMs. Paged subblocks then must be counted separately for each TIM
	auto allToPage = m_aidListPaged;
	allToPage.insert(allToPage.end(), m_aidForcePage.begin(), m_aidForcePage.end());
	if (allToPage.size())
	{
		std::sort(allToPage.begin(), allToPage.end());
		pagedSubblocks = std::unique(allToPage.begin(), allToPage.end(), [](const uint16_t a, const uint16_t b){return ((a >> 3) & 0x07) == ((b >> 3) & 0x07); }) - allToPage.begin();

		pageBitmapLen =1; //TODO this can be 0,1,2,3,4 and should be read from the new page slice that will be constructed in this beacon
	}


	uint32_t numTimPaged (allToPage.size() > 0 ? 1 : 0);
	uint32_t rpsSize (2 + 6 * m_rps->GetNumberOfRawGroups());
	uint32_t timSize (4 + numTimPaged * (1 + 2*31 + pagedSubblocks));
	uint32_t pageSliceSize (6 + pageBitmapLen);
	//NS_LOG_UNCOND("rps=" << rpsSize << ", timSize=" << timSize << ", pageSliceSize=" << pageSliceSize);
	beaconSize += rpsSize + timSize + pageSliceSize;
	//Here we assume data rate is 300000 bps, Nss=1 and symbol duration is 40 us
	Time beaconTxDuration (MicroSeconds (240 + 40 * std::ceil((8 + 6 + 8 * beaconSize) / 12.)));
	NS_LOG_DEBUG ("S1gRawCtr::ControlRps -- beaconTxDuration = " << beaconTxDuration.GetMicroSeconds() << ", beaconSize=" << beaconSize << ", allToPage.size()=" << allToPage.size() << ", numTimPaged=" << numTimPaged);
	return beaconTxDuration;
}

void
S1gRawCtr::ControlRps (std::vector<uint16_t> criticalList)
{

	Time rawlenCritical (Time (0)), rawlenSensors (Time (0));
	Time beaconTxDuration = GetBeaconTxDuration (criticalList);
	for (int i = 0; i < this->m_rps->GetNumberOfRawGroups(); i++)
	{
			if (std::find (criticalList.begin(), criticalList.end(), m_rps->GetRawAssigmentObj(i).GetRawGroupAIDStart()) != criticalList.end())
				rawlenCritical += m_rps->GetRawAssigmentObj(i).GetSlotDuration() * m_rps->GetRawAssigmentObj(i).GetSlotNum();
			else
				rawlenSensors += m_rps->GetRawAssigmentObj(i).GetSlotDuration() * m_rps->GetRawAssigmentObj(i).GetSlotNum();
	}
	if (this->m_beaconInterval - beaconTxDuration.GetMicroSeconds() < (rawlenCritical + rawlenSensors).GetMicroSeconds())
	{
		//raw too large
		if (rawlenCritical.GetMicroSeconds() > m_beaconInterval - beaconTxDuration.GetMicroSeconds())
		{
			NS_LOG_UNCOND ("Critical RAWs take more channel time than available. Delete the last most freq. RAW");
			while (rawlenCritical.GetMicroSeconds() > m_beaconInterval - beaconTxDuration.GetMicroSeconds())
			{
				//delete one of the most freqent RAWs
				std::vector<uint16_t> counts;
				for (auto& aid : criticalList)
				{
					counts.push_back(std::count_if(this->m_allSlots.begin(), this->m_allSlots.end(), [&aid](Slot& a){return a.GetAid() == aid;}));
				}
				auto mostFreqRawIt = std::max_element(counts.begin(), counts.end());
				int mostFreqRawIndex = std::distance(counts.begin(), mostFreqRawIt);
				int aidToDelete = criticalList[mostFreqRawIndex];
				//find the last RAW with that AID
				for (int i = m_rps->GetNumberOfRawGroups() - 1; i >= 0 ; i--)
				{
					if (m_rps->GetRawAssigmentObj(i).GetRawGroupAIDStart() == aidToDelete)
					{
						rawlenCritical -= m_rps->GetRawAssigmentObj(i).GetSlotDuration() * m_rps->GetRawAssigmentObj(i).GetSlotNum();
						m_allSlots.erase(m_allSlots.begin()+i);
						m_rps->DeleteRawAssigmentObj(i);
						break;
					}
				}
			}
			//NS_ASSERT (false);
		}
		else if (rawlenSensors > Time() && this->m_sensorStations.size() > 0)
		{
			Time desiredDurationTotal = MicroSeconds(m_beaconInterval) - beaconTxDuration - rawlenCritical;
			//NS_LOG_UNCOND ("sensors desiredDurationTotal = " << desiredDurationTotal);
			int i = 0;
			bool fixed (false);
			std::map<int, Time> rawIndexToDurationSensors;
			while (rawlenSensors > desiredDurationTotal && i < m_rps->GetNumberOfRawGroups())
			{
				// if not critical
				if (!(std::find (criticalList.begin(), criticalList.end(), m_rps->GetRawAssigmentObj(i).GetRawGroupAIDStart()) != criticalList.end()))
				{
					uint16_t start_aid (m_rps->GetRawAssigmentObj(i).GetRawGroupAIDStart());
					uint16_t end_aid (m_rps->GetRawAssigmentObj(i).GetRawGroupAIDEnd());
					Time currentRawDuration = m_rps->GetRawAssigmentObj(i).GetSlotDuration()*m_rps->GetRawAssigmentObj(i).GetSlotNum();
					Time desired = currentRawDuration - (rawlenSensors - desiredDurationTotal);
					if (desired >= (end_aid - start_aid + 1) * MicroSeconds(1300))
					{
						//ok, assign count
						uint32_t count = (desired.GetMicroSeconds()/m_rps->GetRawAssigmentObj(i).GetSlotNum() - 500) / 120;
						rawlenSensors -= m_rps->GetRawAssigmentObj(i).GetSlotDuration()*m_rps->GetRawAssigmentObj(i).GetSlotNum() - MicroSeconds(count * 120 + 500) * m_rps->GetRawAssigmentObj(i).GetSlotNum();
						RPS::RawAssignment newRawAss = m_rps->GetRawAssigmentObj(i);
						if (count < 256)
							newRawAss.SetSlotFormat(0);
						else if (count < 2048)
							newRawAss.SetSlotFormat(1);
						newRawAss.SetSlotDurationCount(count);
						m_rps->ReplaceRawAssignmentAt(newRawAss, i);
						NS_LOG_UNCOND ("RAW reduced at beacon=" << this->currentId + 1);
						fixed = true;
						break;
					}
					else
					{
						//NS_LOG_UNCOND ("RAW no." << i << " in beacon no." << this->currentId << " is too short to reduce.");
						rawIndexToDurationSensors.insert(std::pair<int, Time>(i, currentRawDuration));
					}
				}
				i++;
			}

			if (!fixed)
			{
				NS_ASSERT (!rawIndexToDurationSensors.empty()); //if this is empty, critical RAWs are too long and I cannot shrink them. Too many control loops!
				double coef = desiredDurationTotal.GetNanoSeconds() * 1.0 / rawlenSensors.GetNanoSeconds();
				//NS_LOG_UNCOND ("++coef=" << coef);
				for (std::map<int, Time>::iterator it = rawIndexToDurationSensors.begin(); it != rawIndexToDurationSensors.end(); it++)
				{
					double scaledRawDuration = coef * (*it).second.GetMicroSeconds();
					//NS_LOG_UNCOND ("scaledRawDuration = " << scaledRawDuration << ", slotnum = " << (int)m_rps->GetRawAssigmentObj((*it).first).GetSlotNum());
					//TODO Slot durations are shrinked now. It could happen that slots are too short for 1 TX. I didn't handle this case
					//In that case we should reduce the number of slots in order to increase the slot duration
					uint16_t scaledCount = (scaledRawDuration / m_rps->GetRawAssigmentObj((*it).first).GetSlotNum()- 500) / 120;
					RPS::RawAssignment newRawAss = m_rps->GetRawAssigmentObj((*it).first);
					if (scaledCount < 256)
						newRawAss.SetSlotFormat(0);
					else if (scaledCount < 2048)
						newRawAss.SetSlotFormat(1);
					//NS_LOG_UNCOND ("scaled count = " << (int)scaledCount << ", format = " << (int)newRawAss.GetSlotFormat());
					newRawAss.SetSlotDurationCount(scaledCount);
					m_rps->ReplaceRawAssignmentAt(newRawAss, (*it).first);
					//NS_LOG_UNCOND ("Multiple RAWs scaled down at beacon=" << this->currentId + 1);
				}
			}

		}
	}

}

uint32_t
S1gRawCtr::GetDlSlotCount (void) const
{
	return 10;
}

Time
S1gRawCtr::GetDlSlotDuration (void) const
{
	return MicroSeconds (500 + 120 * GetDlSlotCount ());
}

uint32_t
S1gRawCtr::GetUlSlotCount (void) const
{
	return 10;
}

Time
S1gRawCtr::GetUlSlotDuration (void) const
{
	return MicroSeconds (500 + 120 * GetUlSlotCount ());
}

Time
S1gRawCtr::GetProcessingTime (void) const
{
	return MilliSeconds (10);
}

std::vector<uint16_t>
S1gRawCtr::GetAidsToForcePage (void) const
{
	return m_aidForcePage;
}

void
S1gRawCtr::DistributeStationsToRaws ()
{
	m_aidForcePage.clear();
	std::vector<Slot> criticalSlots;
	/*for (CriticalStationsCI ci = this->m_criticalStations.begin(); ci != m_criticalStations.end(); ci++)
	{
		SensorActuator * sta = LookupCriticalSta ((*ci)->GetAid());
		//AASIGN ASAP RAW IS PAGED OR EXTEND EXISTING ONE TAKE INTO ACCOUTN PENDING DLPACKETS

		auto aid = (*ci)->GetAid();
		NS_LOG_DEBUG ("AID=" << aid << " is critical and has interval=" << sta->m_tInterval << ", m_tSent=" << sta->m_tSent << ", m_tSentPrev=" << sta->m_tSentPrev);
		if (sta->m_tInterval != Time ())
		{
			Time sinceLastReception = sta->m_tSuccessLast;
			if (sta->m_tSuccessLast > Simulator::Now() - 10 * sta->m_tInterval)
			//Time reserve = sta->m_tInterval > MicroSeconds (m_beaconInterval) ? Simulator::Now() - 5 * sta->m_tInterval - MilliSeconds (10) : Simulator::Now() - 5 * MicroSeconds (m_beaconInterval);
			//if (sta->m_tSent + sta->m_tInterval < Simulator::Now() + MicroSeconds (this->m_beaconInterval) && sta->m_tSent > reserve && std::find(m_aidForcePage.begin(), m_aidForcePage.end(), sta->GetAid()) == m_aidForcePage.end())
			{
				//I expect there will be at least 1 TX by sta in the next beacon
				m_aidForcePage.push_back(sta->GetAid());
			}
			else
			{
				//the last sta->m_tSent is too old, assign generic RAWs
				NS_LOG_DEBUG ("The last m_tSent time IS TOO OLD, ASSIGN GENERIC RAWs");
				Slot s;
				s.SetAid(aid);
				Time sst = GetSoonestSlotStartTime (criticalSlots, GetUlSlotCount() + 10, aid);
				s.SetSlotStartTime(sst);
				auto count = GetUlSlotCount() + 10;
				if (count < 256)
					s.SetSlotFormat(0);
				else if (count < 2048)
					s.SetSlotFormat(1);
				s.SetSlotCount(count);
				criticalSlots.push_back(s);
				continue;
			}

			uint16_t n (1);
			while (sta->m_tSent + n * sta->m_tInterval < Simulator::Now() + MicroSeconds (this->m_beaconInterval))
			{

				if (sta->m_tSent + n * sta->m_tInterval > Simulator::Now())
				{
					Slot s;
					s.SetAid(aid);
					s.SetSlotStartTime(sta->m_tSent + n * sta->m_tInterval);
					if (GetUlSlotCount() < 256)
						s.SetSlotFormat(0);
					else if (GetUlSlotCount() < 2048)
						s.SetSlotFormat(1);
					s.SetSlotCount(GetUlSlotCount());
					criticalSlots.push_back(s);
					//DL
					s.SetSlotStartTime(sta->m_tSent + n * sta->m_tInterval + GetProcessingTime() + MilliSeconds (1));
					if (GetDlSlotCount() < 256)
						s.SetSlotFormat(0);
					else if (GetDlSlotCount() < 2048)
						s.SetSlotFormat(1);
					s.SetSlotCount(this->GetDlSlotCount());
					if (s.GetSlotStartTime() + s.GetSlotDuration() < Simulator::Now() + MicroSeconds(this->m_beaconInterval))
						criticalSlots.push_back(s);
				}
				else
				{
					//sta->m_tSent + n * sta->m_tInterval is in the past, meaning it wasn't delivered
					//assign a RAW to this station at next TBTT + 1ms to allow late packets to be sent
					//Time sst = Simulator::Now() + MilliSeconds (1);
					Time sst = GetSoonestSlotStartTime (criticalSlots, this->GetUlSlotCount(), aid);
					auto slotIt = std::find_if (criticalSlots.begin(), criticalSlots.end(), [&aid, &sst](const Slot s){return s.GetAid() == aid && s.GetSlotStartTime() == sst;});
					if (slotIt == criticalSlots.end())
					{
						//no previous attempts to add a slot to aid that starts at sst
						//see if that slot is added to some other AID
						// FORCE PAGING HERE TODO
						Slot s;
						s.SetAid(aid);
						//sst = GetSoonestSlotStartTime (allSlots, GetUlSlotCount() + sta->m_pendingDownlinkPackets * GetDlSlotCount(), aid);
						s.SetSlotStartTime(sst);
						uint16_t count;
						if (n == 1)
							count= this->GetUlSlotCount() + sta->m_pendingDownlinkPackets * this->GetDlSlotCount();
						else
							count = GetUlSlotCount();
						if (count < 256)
							s.SetSlotFormat(0);
						else if (count < 2048)
							s.SetSlotFormat(1);
						s.SetSlotCount(count);
						//sta->m_pendingDownlinkPackets = 0;
						sta->m_paged = true;
						criticalSlots.push_back(s);
						//DL
						s.SetSlotStartTime(sst + GetProcessingTime() + MilliSeconds (1));
						//count = GetDlSlotCount();
						if (count < 256)
							s.SetSlotFormat(0);
						else if (count < 2048)
							s.SetSlotFormat(1);
						s.SetSlotCount(count);
						if (s.GetSlotStartTime() + s.GetSlotDuration() < Simulator::Now() + MicroSeconds(this->m_beaconInterval))
							criticalSlots.push_back(s);
					}
					else
					{
						//already added slot to aid that starts at sst, do not add another slot but extend the existing one
						uint16_t count = GetUlSlotCount() + slotIt->GetSlotCount();
						if (count < 256)
							slotIt->SetSlotFormat(0);
						else if (count < 2048)
							slotIt->SetSlotFormat(1);
						slotIt->SetSlotCount(count);
						Time dlSst = sst + GetProcessingTime() + MilliSeconds (1);
						auto dlSlotIt = std::find_if (criticalSlots.begin(), criticalSlots.end(), [&aid, &dlSst](const Slot s){return s.GetAid() == aid && s.GetSlotStartTime() == dlSst;});
						if (dlSlotIt != criticalSlots.end())
						{
							count = GetDlSlotCount() + dlSlotIt->GetSlotCount();
							if (count < 256)
								dlSlotIt->SetSlotFormat(0);
							else if (count < 2048)
								dlSlotIt->SetSlotFormat(1);
							dlSlotIt->SetSlotCount(count);
						}
						else
						{
							Slot s;
							s.SetAid(slotIt->GetAid());
							s.SetSlotStartTime(dlSst);
							count = GetDlSlotCount();
							if (count < 256)
								s.SetSlotFormat(0);
							else if (count < 2048)
								s.SetSlotFormat(1);
							s.SetSlotCount(count);
							if (s.GetSlotStartTime() + s.GetSlotDuration() < Simulator::Now() + MicroSeconds(this->m_beaconInterval))
								criticalSlots.push_back(s);
						}
					}
				}
				n++;
			}
		}
		else
		{
			//couldn't calculate sta's interval because never received, maybe it didn't have the opportunity?
			//assign a slot to it
			Slot s;
			s.SetAid(aid);
			Time sst = GetSoonestSlotStartTime (criticalSlots, GetUlSlotCount() + 10, aid);
			s.SetSlotStartTime(sst);
			auto count = GetUlSlotCount() + 10;
			if (count < 256)
				s.SetSlotFormat(0);
			else if (count < 2048)
				s.SetSlotFormat(1);
			s.SetSlotCount(count);
			criticalSlots.push_back(s);
		}
	}*/

	Slot s;

	CriticalStationsCI ci = this->m_criticalStations.begin();
	uint16_t minaidcritical = (*std::min_element(m_criticalStations.begin(), m_criticalStations.end(), [](const SensorActuator *a, const SensorActuator *b){return a->GetAid() < b->GetAid(); }))->GetAid();
	uint16_t maxaidsensor;
	if (m_sensorStations.size())
		maxaidsensor = (*std::max_element(m_sensorStations.begin(), m_sensorStations.end(), [](const Sensor *a, const Sensor *b){return a->GetAid() < b->GetAid(); }))->GetAid();
	else
		maxaidsensor = (*std::max_element(m_criticalStations.begin(), m_criticalStations.end(), [](const SensorActuator *a, const SensorActuator *b){return a->GetAid() < b->GetAid(); }))->GetAid();
	s.SetStartAid(minaidcritical);
	s.SetEndAid(maxaidsensor);
	s.SetSlotStartTime(Time ());
	s.SetSlotFormat(1);
	s.SetSlotCount(809);
	criticalSlots.push_back(s);
	EncodeRaws (criticalSlots);
}

void S1gRawCtr::EncodeRaws (std::vector<Slot> criticalSlots)
{
	auto allSlots = criticalSlots;
/*	std::sort(criticalSlots.begin(), criticalSlots.end(), [](const Slot a, const Slot b){return a.GetSlotStartTime() < b.GetSlotStartTime();});

	//make sure there is no overlapping slots
	if (m_sensorStations.size() > 0)
	{
		uint16_t minaidsensor = (*std::min_element(m_sensorStations.begin(), m_sensorStations.end(), [](const Sensor *a, const Sensor *b){return a->GetAid() < b->GetAid(); }))->GetAid();
		uint16_t maxaidsensor = (*std::max_element(m_sensorStations.begin(), m_sensorStations.end(), [](const Sensor *a, const Sensor *b){return a->GetAid() < b->GetAid(); }))->GetAid();
		//NS_LOG_UNCOND ("+min aid sensor=" << (int)minaidsensor << ", max aid sensor=" << maxaidsensor);

		for (int i = 0; i < criticalSlots.size() - 1; i++)
		{
			Time end = criticalSlots[i].GetSlotStartTime() + criticalSlots[i].GetSlotDuration();
			if (end > criticalSlots[i + 1].GetSlotStartTime())
				criticalSlots[i + 1].SetSlotStartTime(end);

			Time tdiff = criticalSlots[i + 1].GetSlotStartTime() - end;

			if (tdiff >= MicroSeconds(1300))
			{
				//assign sensor RAW
				Slot s;
				s.SetStartAid(minaidsensor);
				s.SetEndAid(maxaidsensor);
				auto count = (tdiff.GetMicroSeconds() - 500) / 120;
				if (count < 256)
					s.SetSlotFormat(0);
				else if (count < 2048)
					s.SetSlotFormat(1);
				s.SetSlotCount(count);
				s.SetSlotStartTime(end);
				allSlots.push_back(s);
			}
		}
		std::sort(allSlots.begin(), allSlots.end(), [](const Slot a, const Slot b){return a.GetSlotStartTime() < b.GetSlotStartTime();});

		Time end = criticalSlots[criticalSlots.size() - 1].GetSlotStartTime() + criticalSlots[criticalSlots.size() - 1].GetSlotDuration();
		Time tdiff = Simulator::Now() + MicroSeconds (m_beaconInterval) - end;
		//NS_LOG_UNCOND ("++last crit RAW ends at = " << end << ", next tbtt=" << Simulator::Now() + MicroSeconds (m_beaconInterval) << ", diff=" << tdiff);
		//assign Sensors at the end if there is time
		if (tdiff >= MicroSeconds(1300))
		{
			//assign sensor RAW
			Slot s;
			s.SetStartAid(minaidsensor);
			s.SetEndAid(maxaidsensor);
			auto count = (tdiff.GetMicroSeconds() - 500) / 120;
			if (count < 256)
				s.SetSlotFormat(0);
			else if (count < 2048)
				s.SetSlotFormat(1);
			s.SetSlotCount(count);
			s.SetSlotStartTime(end);
			allSlots.push_back(s);
		}
	}
	//NS_LOG_UNCOND ("allSlots size = " << allSlots.size() << ", crit slots size=" << criticalSlots.size());

	//if there are concurrent slots with the same start and end AIDs, merge those slots to minimise RAWs
	//if there are concurrent slots
	//Slot prevSlot = allSlots[0];
	for (int h = 0; h < allSlots.size() - 1; h++)
	{
		auto nextSlot = allSlots[h+1];
		if (allSlots[h].GetStartAid() == nextSlot.GetStartAid() && allSlots[h].GetEndAid() == nextSlot.GetEndAid())
		{
			Time newDuration = nextSlot.GetSlotStartTime() + nextSlot.GetSlotDuration() - allSlots[h].GetSlotStartTime();
			uint16_t newcount = (newDuration.GetMicroSeconds() - 500) / 120;
			if (newcount < 256)
				allSlots[h].SetSlotFormat(0);
			else if (newcount < 2048)
				allSlots[h].SetSlotFormat(1);
			allSlots[h].SetSlotCount(newcount);
			allSlots.erase(allSlots.begin() + h + 1);
			h--;
		}
	}
*/

	//ASSIGN SLOTS TO RPS
	RPS::RawAssignment *m_raw = new RPS::RawAssignment;
	m_raw->SetRawControl(0);  //support paged STA or not
	m_raw->SetSlotCrossBoundary(1);
	for (auto& s : allSlots)
	{
		uint16_t aid_start = s.GetStartAid();
		uint16_t aid_end = s.GetEndAid();
		uint32_t rawinfo = (aid_end << 13) | (aid_start << 2) | 0x0;
		m_raw->SetRawGroup(rawinfo);
		m_raw->SetSlotNum(1);
		m_raw->SetSlotFormat (s.GetSlotFormat());
		m_raw->SetSlotDurationCount(s.GetSlotCount());
		m_rps->SetRawAssignment(*m_raw);
	}
	delete m_raw;
	m_allSlots = allSlots;
}

Time
S1gRawCtr::GetSoonestSlotStartTime (std::vector<Slot> allSlots, uint16_t minCount, uint16_t aid) const
{
	std::vector<Slot>::iterator it;
	Time startTime = Simulator::Now() + MilliSeconds (1);
	Time minDuration = MicroSeconds (500 + 120 * minCount);
	std::sort(allSlots.begin(), allSlots.end(), [](const Slot a, const Slot b){return a.GetSlotStartTime() < b.GetSlotStartTime();});

	for (it = allSlots.begin(); it != allSlots.end(); it++)
	{
		if (it->GetSlotStartTime() >= startTime + minDuration)
			return startTime;
		else if (it->GetAid() == aid)
		{
			auto nextit = it;
			std::advance(nextit, 1);
			if (nextit != allSlots.end())
				if (it->GetSlotStartTime() + it->GetSlotDuration() + minDuration <= nextit->GetSlotStartTime())
					return it->GetSlotStartTime();
				else
				{
					startTime = nextit->GetSlotStartTime() + nextit->GetSlotDuration();
					it++;
				}
			else if (it->GetSlotStartTime() + it->GetSlotDuration() + minDuration < Simulator::Now() + MicroSeconds (m_beaconInterval))
				return it->GetSlotStartTime();

		}
		else
			startTime = it->GetSlotStartTime() + it->GetSlotDuration();
	}
	return startTime;
	/*NS_LOG_UNCOND ("ALL SLOTS:" );
	for (auto& s : allSlots)
	{
		NS_LOG_UNCOND ("AID=" << (int)s.GetAid() << "\t start ime=" << s.GetSlotStartTime() << "\t duration=" << s.GetSlotDuration());
	}
	NS_LOG_UNCOND ("S1gRawCtr::GetSoonestSlotStartTime cannot find any slot for aid=" << (int)aid <<" that is >= wanted minCount=" << (int)minCount);
	NS_ASSERT (false);*/
}

void
S1gRawCtr::deleteRps ()
{
    delete m_rps;
    delete m_prevRps;
}

void S1gRawCtr::gandalf()
{/*
    std::cout <<
    	"                           ,---.\n" <<
        "                          /    |\n" <<
        "                         /     |\n" <<
        "                        /      |\n" <<
        "                       /       |\n" <<
        "                  ___,'        |\n" <<
        "                <  -'          :\n" <<
        "                 `-.__..--'``-,_\_\n" <<
        "                    |o/ <o>` :,.)_`>\n" <<
        "                    :/ `     ||/)\n" <<
        "                    (_.).__,-` |\\\n" <<
        "                    /( `.``   `| :\n" <<
        "                    \'`-.)  `  ; ;\n" <<
        "                    | `       /-<\n" <<
        "                    |     `  /   `.\n" <<
        "    ,-_-..____     /|  `    :__..-'\\\n" <<
        "   /,'-.__\\\\  ``-./ :`      ;       \\\n" <<
        "   `\ `\  `\\\\  \ :  (   `  /  ,   `. \\\n" <<
        "     \` \   \\\\   |  | `   :  :     .\ \\\n" <<
        "      \ `\_  ))  :  ;     |  |      ): :\n" <<
        "     (`-.-'\ ||  |\ \   ` ;  ;       | |\n" <<
        "      \-_   `;;._   ( `  /  /_       | |\n" <<
        "       `-.-.// ,'`-._\__/_,'         ; |\n" <<
        "          \:: :     /     `     ,   /  |\n" <<
        "           || |    (        ,' /   /   |\n" <<
        "           ||                ,'   /    |" << std::endl;
*/}

void S1gRawCtr::darth()
{
	std::cout <<
					  "           _.-'~~~~~~`-._\n" <<
			          "          /      ||      \\\n" <<
			          "         /       ||       \\\n" <<
			          "        |        ||        |\n" <<
			          "        | _______||_______ |\n" <<
			          "        |/ ----- \\/ ----- \\|\n" <<
			          "       /  (     )  (     )  \\\n" <<
			          "      / \\  ----- () -----  / \\\n" <<
			          "     /   \\      /||\\      /   \\\n" <<
			          "    /     \\    /||||\\    /     \\\n" <<
			          "   /       \\  /||||||\\  /       \\\n" <<
			          "  /_        \\o========o/        _\\\n" <<
			          "    `--...__|`-._  _.-'|__...--'\n" <<
			          "            |    `'    |" << std::endl;
}
    /*
     void
     S1gRawCtr::configureRAW (RPS * m_rps, RPS::RawAssignment *  m_raw )
     {
     uint16_t RawControl = 0;
     uint16_t SlotCrossBoundary = 1;
     uint16_t SlotFormat = 1;
     uint16_t SlotDurationCount;
     uint16_t SlotNum = 1;
     uint16_t page = 0;
     uint32_t aid_start = 0;
     uint32_t aid_end = 0;
     uint16_t rawinfo;



     SlotDurationCount = (m_beaconInterval/(SlotNum) - 500)/120;
     NS_ASSERT (SlotDurationCount <= 2037);


     //RPS * m_rps = new RPS;
     //RPS * m_rps;

     //Release storage.
         //NS_LOG_UNCOND ("S1gRawCtr::UpdateRAWGrouppingee =");

         //RPS::RawAssignment *m_raw = new RPS::RawAssignment;

     m_raw->SetRawControl (RawControl);//support paged STA or not
     m_raw->SetSlotCrossBoundary (SlotCrossBoundary);
     m_raw->SetSlotFormat (1);
     m_raw->SetSlotDurationCount (300);//to change
     m_raw->SetSlotNum (SlotNum);

     aid_start = 1;
     aid_end = 1;
     rawinfo = (aid_end << 13) | (aid_start << 2) | page;
     m_raw->SetRawGroup (rawinfo);

        //NS_LOG_UNCOND ("S1gRawCtr::UpdateRAWGrouppingff =");

     m_rps->SetRawAssignment(*m_raw);

        //NS_LOG_UNCOND ("S1gRawCtr::UpdateRAWGrouppinggg =");

     rpslist.rpsset.push_back (m_rps); //only one RPS in rpslist actually, update info
        //NS_LOG_UNCOND ("S1gRawCtr::UpdateRAWGrouppinghh =");

 } */

//configure RAW based on grouping result

void
S1gRawCtr::configureRAW ( )
{
    uint16_t RawControl = 0;
    uint16_t SlotCrossBoundary = 1;
    uint16_t SlotFormat = 1;
    uint16_t SlotDurationCount;
    uint16_t SlotNum = 1;
    uint16_t page = 0;
    uint32_t aid_start = 0;
    uint32_t aid_end = 0;
    uint32_t rawinfo;

    //Release storage.
    rpslist.rpsset.clear();

    //uint16_t NGroups = m_aidList.size () + m_aidOffloadList.size();
    uint16_t NGroups = m_numSendSensorAllowed + m_aidOffloadList.size();

    if (NGroups == 0)
      {
        SlotDurationCount = m_slotDurationCount;
        RPS::RawAssignment *m_raw = new RPS::RawAssignment;

        m_raw->SetRawControl (RawControl);
        m_raw->SetSlotCrossBoundary (SlotCrossBoundary);
        m_raw->SetSlotFormat (SlotFormat);
        m_raw->SetSlotDurationCount (SlotDurationCount);
        m_raw->SetSlotNum (SlotNum);

        aid_start = 1;
        aid_end = 1;
        rawinfo = (aid_end << 13) | (aid_start << 2) | page;
        m_raw->SetRawGroup (rawinfo);

        m_rps->SetRawAssignment(*m_raw);
        delete m_raw;
        rpslist.rpsset.push_back (m_rps);
        return;
      }
    //SlotDurationCount = ((m_beaconInterval-100)/(SlotNum*NGroups) - 500)/120;
    //SlotDurationCount = (m_rawslotDuration - 500)/120;
    //NS_ASSERT (SlotDurationCount <= 2037);


    //NS_LOG_UNCOND ("m_aidList =" << m_aidList.size () << ", m_aidOffloadList=" << m_aidOffloadList.size ());
    
    m_beaconOverhead = ((m_aidList.size () * 6 + 60) * 8 + 14 )/12 * 40 + 560;
    NS_LOG_UNCOND ("m_beaconOverhead = " << m_beaconOverhead);

    
    for (std::vector<uint16_t>::iterator it = m_aidList.begin(); it != m_aidList.end(); it++)
      {

          RPS::RawAssignment *m_raw = new RPS::RawAssignment;
          Sensor * stationTransmit = LookupSensorSta (*it);
          uint16_t num = stationTransmit->GetTransInOneBeacon ();
          //SlotDurationCount = (num * m_rawslotDuration - 500)/120;
          uint64_t revisedslotduration = std::ceil(num * (m_beaconInterval-m_beaconOverhead) * 1.0 / m_numSendSensorAllowed);
          SlotDurationCount = std::ceil((revisedslotduration - 500.0)/120.0);

          NS_ASSERT (SlotDurationCount <= 2037);


          m_raw->SetRawControl (RawControl);//support paged STA or not
          m_raw->SetSlotCrossBoundary (SlotCrossBoundary);
          m_raw->SetSlotFormat (SlotFormat);
          m_raw->SetSlotDurationCount (SlotDurationCount);//to change
          //m_raw->SetSlotDurationCount (725);//to change


          m_raw->SetSlotNum (SlotNum);


          aid_start = *it;
          aid_end = *it;
          //aid_start = 2;
          //aid_end = 66;
          NS_LOG_UNCOND ("sensor, aid_start =" << aid_start << ", aid_end=" << aid_end << ", SlotDurationCount = " << SlotDurationCount << ", transmit num one beacon = " << num);

          rawinfo = (aid_end << 13) | (aid_start << 2) | page;
          m_raw->SetRawGroup (rawinfo);

          m_rps->SetRawAssignment(*m_raw);
          delete m_raw;
      }
    
    //set remaining channel to another raw
    /*
    RPS::RawAssignment *m_rawAll = new RPS::RawAssignment;
    SlotDurationCount = (m_offloadRawslotDuration - 500)/120;
    NS_ASSERT (SlotDurationCount <= 2037);
    
    m_rawAll->SetRawControl (RawControl);//support paged STA or not
    m_rawAll->SetSlotCrossBoundary (SlotCrossBoundary);
    m_rawAll->SetSlotFormat (SlotFormat);
    m_rawAll->SetSlotDurationCount (SlotDurationCount);//to change
    m_rawAll->SetSlotNum (SlotNum);
    aid_start = 1;
    aid_end = m_stations.size();
    NS_LOG_UNCOND ("sensor, aid_start =" << aid_start << ", aid_end=" << aid_end << ", SlotDurationCount = " << SlotDurationCount);
    
    rawinfo = (aid_end << 13) | (aid_start << 2) | page;
    m_rawAll->SetRawGroup (rawinfo);
    
    m_rps->SetRawAssignment(*m_rawAll);
    delete m_rawAll;
    //finished
    */



    uint64_t offloadcount = (m_offloadRawslotDuration - 500)/120;
    //printf("offloadcount is %u\n",  offloadcount);
    //NS_LOG_UNCOND ("m_offloadRawslotDuration = " << m_offloadRawslotDuration << ", offloadcount =" << offloadcount );

    for (std::vector<uint16_t>::iterator it = m_aidOffloadList.begin(); it != m_aidOffloadList.end(); it++)
     {
        RPS::RawAssignment *m_raw2 = new RPS::RawAssignment;

        m_raw2->SetRawControl (RawControl);//support paged STA or not
        m_raw2->SetSlotCrossBoundary (SlotCrossBoundary);
        m_raw2->SetSlotFormat (SlotFormat);
        m_raw2->SetSlotDurationCount (offloadcount); //to change
         //m_raw2->SetSlotDurationCount (99); //to change
        m_raw2->SetSlotNum (SlotNum);

        aid_start = *it;
        aid_end = *it;
         //aid_start = 1;
         //aid_end = 1;
        rawinfo = (aid_end << 13) | (aid_start << 2) | page;
        NS_LOG_UNCOND ("offload, aid_start =" << aid_start << ", aid_end=" << aid_end << ", offloadcount =" << offloadcount);

        m_raw2->SetRawGroup (rawinfo);
        m_rps->SetRawAssignment(*m_raw2);
        delete m_raw2;
     }


    rpslist.rpsset.push_back (m_rps); //only one RPS in rpslist actually, update info every beacon in this algorithm.
    //printf("rpslist.rpsset.size is %u\n",  rpslist.rpsset.size());

    //delete m_rps;
}

SensorActuator *
S1gRawCtr::LookupCriticalSta (uint16_t aid)
{
	for (CriticalStationsCI it = m_criticalStations.begin(); it != m_criticalStations.end(); it++)
	{
		if (aid == (*it)->GetAid ())
		{
			return (*it);
		}
	}
	return nullptr;
}

//what if lookup fails, is it possibile?
Sensor *
S1gRawCtr::LookupSensorSta (uint16_t aid)
{
   for (StationsCI it = m_sensorStations.begin(); it != m_sensorStations.end(); it++)
   {
       if (aid == (*it)->GetAid ())
        {
           return (*it);
        }
   }
    return nullptr;
}

//what if lookup fails, is it possibile?
OffloadStation *
S1gRawCtr::LookupOffloadSta (uint16_t aid)
{
    for (OffloadStationsCI it = m_offloadStations.begin(); it != m_offloadStations.end(); it++)
    {
        if (aid == (*it)->GetAid ())
        {
            return (*it);
        }
    }
    return nullptr;
}

void
S1gRawCtr::calculateRawSlotDuration (uint16_t numsta, uint16_t successprob)
{
    m_rawslotDuration = (m_slotDurationCount*120)+500; //for test.
    m_offloadRawslotDuration = (m_slotDurationCount*120)+500;
}

} //namespace ns3
