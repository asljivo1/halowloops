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

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("S1gRawCtr");

//NS_OBJECT_ENSURE_REGISTERED (S1gRawCtr);

Slot::Slot () : m_assignedAid(0), m_slotCount(0), m_slotStartTime(Time ()), m_slotDuration(Time ()), m_slotFormat (1)
{}

Slot::~Slot ()
{}

uint16_t
Slot::GetAid (void) const
{
	NS_ASSERT (m_startAid == m_endAid);
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
	NS_ASSERT((!m_slotFormat & (count < 256)) || (m_slotFormat & (count < 2048)));
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

SensorActuator::SensorActuator (void) : m_pendingDownlinkPackets(0), m_paged (false), m_nTx (0), m_oldRawStart (INT_MAX), m_newRawStart (INT_MAX), m_tSent (Time ()), m_tSentPrev (Time ()), m_tInterval (Time ()), m_tIntervalMin (Time ()), m_tIntervalMax (Time ()), m_tEnqMin (Time ())
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
	m_prevPrevRps = nullptr;
	m_rps = new RPS;
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
	NS_LOG_UNCOND ("receivedFromAids.size () = " << receivedFromAids.size () << ", m_sensorStations.size() = " << m_sensorStations.size() << ", currentBeacon = " << currentId);

}

void
S1gRawCtr::UpdateCriticalStaInfo (std::vector<uint16_t> criticalAids, std::vector<uint16_t> receivedFromAids, std::vector<uint16_t> enqueuedToAids, std::vector<Time> receivedTimes, std::vector<Time> sentTimes, std::string outputpath)
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
						else
							NS_ASSERT (stationTransmit->m_tInterval == stationTransmit->m_tSent - stationTransmit->m_tSentPrev);
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
	NS_LOG_UNCOND ("receivedFromAids.size () = " << receivedFromAids.size () << ", m_criticalStations.size() = " << m_criticalStations.size() << ", m_aidListPaged.size = " << this->m_aidListPaged.size() << ", currentBeacon = " << currentId);
	NS_LOG_UNCOND ("enqueuedToAids = " << enqueuedToAids.size());
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

// Beacon duration), before that use NGroup=1 and initialize by ap-wifi-mac
RPS
S1gRawCtr::UpdateRAWGroupping (std::vector<uint16_t> criticalList, std::vector<uint16_t> sensorList, std::vector<uint16_t> offloadList, std::vector<uint16_t> receivedFromAids, std::vector<Time> receivedTimes, std::vector<Time> sentTimes, std::vector<uint16_t> sentToAids, std::vector<uint16_t> enqueuedToAids, uint64_t BeaconInterval, RPS *prevRps, pageSlice pageslice, uint8_t dtimCount, Time bufferTimeToAllowBeaconToBeReceived, std::string outputpath)
 {
     NS_ASSERT ("S1gRawCtr should not be called");
     //gandalf ();
     m_beaconInterval = BeaconInterval;
     //m_prevRps = prevRps;
     if (m_prevRps == nullptr && criticalList.size())
     {

    	 //initial
    	 uint32_t count = (m_beaconInterval / 10 - 500) / 120;
    	 uint32_t duration = 500 + count * 120;
    	 RPS::RawAssignment *m_raw = new RPS::RawAssignment; //loop's RAW, unless there is no loops in this TIM
    	 m_raw->SetRawControl(0);  //support paged STA or not
    	 m_raw->SetSlotCrossBoundary(1);
    	 m_raw->SetSlotFormat (1);
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
    				 uint32_t rawinfo = (aid_end << 13) | (aid_start << 2) | page;
    				 m_raw->SetRawGroup(rawinfo);
    				 m_raw->SetSlotNum(aid_end - aid_start + 1);
    				 uint32_t count = (m_beaconInterval / 10 - 500) / 120;
    				 if (count < 256)
    					 m_raw->SetSlotFormat(0);
    				 else if (count < 2048)
    					 m_raw->SetSlotFormat(1);
    				 m_raw->SetSlotDurationCount(count);
    				 m_rps->SetRawAssignment(*m_raw);
    			 }
    			 else
    			 {
    				 //loops are not in this tim; assign to sensors (there is no offload stations in my experiments so I am not considering them)
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
    			 }


    		 }
    		 else
    		 {
    			 // there is only one pageslice and all stations belong to it
    			 //loops' RAW
    			 uint16_t aid_start = *std::min_element(criticalList.begin(), criticalList.end());
    			 uint16_t aid_end = *std::max_element(criticalList.begin(), criticalList.end());
    			 uint32_t rawinfo = (aid_end << 13) | (aid_start << 2) | page;
    			 m_raw->SetRawGroup(rawinfo);
    			 uint32_t count = (m_beaconInterval / 10 - 500) / 120;
    			 if (count < 256)
    				 m_raw->SetSlotFormat(0);
    			 else if (count < 2048)
    				 m_raw->SetSlotFormat(1);
    			 m_raw->SetSlotDurationCount(count);
    			 uint32_t numslots;
    			 m_raw->SetSlotNum(criticalList.size());
    			 uint32_t duration = 500 + 120 * count;
    			 NS_LOG_UNCOND ("bufferTimeToAllowBeaconToBeReceived us = " << bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds());
    			 if (duration * criticalList.size() >= m_beaconInterval - bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds())
    			 {
    				 count = 1 + (2 * 1240 - 500) / 120;
    				 duration = 500 + 120 * count;
    				 m_raw->SetSlotDurationCount(count);
    				 if (duration * criticalList.size() >= m_beaconInterval - bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds())
    				 {
    					 NS_LOG_UNCOND ("CANNOT ASSIGN SEPARATE SLOT TO " << criticalList.size() << " CRITICAL STATIONS!");
    					 numslots = (m_beaconInterval - bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds()) / (500 + 120 * count);
    					 m_raw->SetSlotNum(numslots);
    				 }
    			 }
    			 m_rps->SetRawAssignment(*m_raw);

    			 // rest of the stations' RAW
    			 if (!sensorList.empty())
    			 {
    				 aid_start = *std::min_element(sensorList.begin(), sensorList.end());
    				 aid_end = *std::max_element(sensorList.begin(), sensorList.end());
    				 rawinfo = (aid_end << 13) | (aid_start << 2) | page;
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
    			 }

    		 }
    	 }
    	 delete m_raw;
    	 this->m_prevRps = new RPS;
    	 *m_prevRps = *m_rps;
     }
     else if (m_prevRps != nullptr && criticalList.size())
     {
    	 UpdateCriticalStaInfo (criticalList, receivedFromAids, enqueuedToAids, receivedTimes, sentTimes, outputpath);
    	 UpdateSensorStaInfo (sensorList, receivedFromAids, receivedTimes, sentTimes);
    	 //this->UdpateSensorStaInfo(sensorList, receivedFromAids, outputpath);

    	 //not initial, there was a non-empty RPS in the previous beacon but no successful receptions??????
    	 //2 possibilities:
    	 //   1) Unlucky RAW configuration: packets were enqueued at STA after RAW so they couldn't be TXed
    	 //	  2) t_int is longer than beacon interval, unused RAW
    	 // AP cannot know which case is it at this point

    	 //our strategy is to keep the same RAW grouping but shift in time for beacon_interval/2
    	 // TODO experiment with this setting in the end!!!!
    	 /*
    	 for (CriticalStationsCI ci = this->m_criticalStations.begin(); ci != m_criticalStations.end(); ci++)
    	 {
    		 if ((*ci)->m_nTx == 0)// TODO this will execute at the very end of simulation in case I schedule for the next beacon normally
    		 {//TODO make sure this doesn't worsen the schedule in this case, add && m_tint == 0 or set states or something?

    			 //state P_NRX
    			 // keep the same RAW configuration but shift it in time for BI/2 for all loops
    			 // not all loops have been present in m-prevRps, some might just be associated
    			 SensorActuator * sta = LookupCriticalSta ((*ci)->GetAid());
    			 if (sta->m_oldRawStart < INT_MAX)
    				 sta->m_newRawStart = sta->m_oldRawStart + bufferTimeToAllowBeaconToBeReceived.GetMicroSeconds() + m_prevRps->GetRawAssigmentObjFromAid((*ci)->GetAid()).GetSlotDuration() <= m_beaconInterval/2 ? sta->m_oldRawStart + m_beaconInterval/2 : 0;
    			 else
    				 sta->m_newRawStart = 0;
    			 //This m_newRawStart is a wanted value, but it can be the same for multiple loops at this point. Later make sure all loops have their own non-overlapping slots
    		 }
    		 else
    		 {
    			 //state P_RX
    		 }
    	 }
    	  */

    	 delete m_rps;
    	 m_rps = new RPS;


    	 DistributeStationsToRaws ();

    	 // I've populated m_criticalStations, m_aidListPaged, m_aidList;
    	 // I have to determine m_tInterval, m_tEnqMin etc for RAW assignment


     }
     /*else if (!m_t_succ.empty() && m_prevRps)
     {
    	 //not initial, there was a non-empty RPS in the previous beacon and successful receptions
    	 // see if some STAs actually didn't have successfull transmissions, in that case is the previous else if but for a STA

     }*/
     currentId++; //beaconInterval counter

     ControlRps (criticalList);
     //std::ofstream coutpom = std::cout;
     m_rps->Print(std::cout);
     /*NS_LOG_UNCOND (" +++ ALL SLOTS +++" );
     	for (auto& s : allSlots)
     	{
     		NS_LOG_UNCOND ("startAID-endAID=" << (int)s.GetStartAid() << "-" << (int)s.GetEndAid() << "\t start time=" << s.GetSlotStartTime() << "\t duration=" << s.GetSlotDuration());
     	}*/
     std::ofstream pom;
     std::string strr = "lastrps.txt";
     pom.open(strr.c_str(), std::ios::out | std::ios::trunc);
     m_rps->Print(pom);
     pom.close();

     std::ofstream os;
     outputpath += std::to_string(currentId) + ".txt";
     os.open(outputpath.c_str(), std::ios::out | std::ios::trunc);
     m_rps->Print(os);
     os.close();

     m_aidList.clear();
     m_aidListPaged.clear();
     return *m_rps;
}

void
S1gRawCtr::ControlRps (std::vector<uint16_t> criticalList)
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
	NS_LOG_UNCOND ("+++++beaconTxDuration = " << beaconTxDuration.GetMicroSeconds() << ", beaconSize=" << beaconSize << ", allToPage.size()=" << allToPage.size() << ", numTimPaged=" << numTimPaged);
	Time rawlenCritical (Time (0)), rawlenSensors (Time (0));
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
		if (rawlenSensors > Time() && this->m_sensorStations.size() > 0)
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
		else if (rawlenCritical.GetMicroSeconds() > m_beaconInterval - beaconTxDuration.GetMicroSeconds())
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
	for (CriticalStationsCI ci = this->m_criticalStations.begin(); ci != m_criticalStations.end(); ci++)
	{
		SensorActuator * sta = LookupCriticalSta ((*ci)->GetAid());
		//AASIGN ASAP RAW IS PAGED OR EXTEND EXISTING ONE TAKE INTO ACCOUTN PENDING DLPACKETS

		auto aid = (*ci)->GetAid();
		NS_LOG_UNCOND ("AID=" << aid << " is critical and has interval=" << sta->m_tInterval << ", m_tSent=" << sta->m_tSent << ", m_tSentPrev=" << sta->m_tSentPrev);
		if (sta->m_tInterval != Time ())
		{
			Time reserve = sta->m_tInterval > MicroSeconds (m_beaconInterval) ? Simulator::Now() - 5 * sta->m_tInterval - MilliSeconds (10) : Simulator::Now() - 5 * MicroSeconds (m_beaconInterval);
			if (sta->m_tSent + sta->m_tInterval < Simulator::Now() + MicroSeconds (this->m_beaconInterval) && sta->m_tSent > reserve && std::find(m_aidForcePage.begin(), m_aidForcePage.end(), sta->GetAid()) == m_aidForcePage.end())
			{
				//I expect there will be at least 1 TX by sta in the next beacon
				m_aidForcePage.push_back(sta->GetAid());
			}
			else
			{
				//the last sta->m_tSent is too old, assign generic RAWs
				NS_LOG_UNCOND ("The last m_tSent time IS TOO OLD, ASSIGN GENERIC RAWs");
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
	}
	std::sort(criticalSlots.begin(), criticalSlots.end(), [](const Slot a, const Slot b){return a.GetSlotStartTime() < b.GetSlotStartTime();});
	auto allSlots = criticalSlots;
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

	/*std::vector<Slot>::iterator it;
	for (it = allSlots.begin(); it != allSlots.end(); it++)
	{
		auto nextit = it;
		std::advance (nextit, 1);
		if ((*it).GetStartAid() == (*nextit).GetStartAid() && it != allSlots.end())
		{
			Time newDuration = (*nextit).GetSlotStartTime() + (*nextit).GetSlotDuration() - (*it).GetSlotStartTime();
			uint16_t newcount = (newDuration.GetMicroSeconds() - 500) / 120;
			if (newcount < 256)
				(*it).SetSlotFormat(0);
			else if (newcount < 2048)
				(*it).SetSlotFormat(1);
			(*it).SetSlotCount(newcount);
			allSlots.erase(nextit);
		}
	}*/

	/*NS_LOG_UNCOND (" +++ ALL SLOTS +++" );
	for (auto& s : allSlots)
	{
		NS_LOG_UNCOND ("startAID-endAID=" << (int)s.GetStartAid() << "-" << (int)s.GetEndAid() << "\t start time=" << s.GetSlotStartTime() << "\t duration=" << s.GetSlotDuration());
	}*/

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
}

void S1gRawCtr::gandalf()
{
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
}

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
