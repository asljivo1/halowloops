#pragma once

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/extension-headers.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <fstream>
#include <sys/stat.h>
#include <string>

using namespace ns3;
using namespace std;

struct Configuration {
	/*
	 * New configuration parameters
	 *
	 * */
	RPSVector rps;
	uint32_t nRps;						// Ordinal number of current RPS element; RPS Index
	uint64_t totalRawSlots = 0;			// Total number of RAW slots in all RAW groups in all RPS elements
	bool enableVirtualAids=false;

	uint64_t maxNumberOfPackets = 4294967295u; ///4294967295u
	string trafficType = "udpecho"; // important - udp is considered to be only uplink in NodeStatistics::GetPacketLoss tcpipcamera tcpfirmware

	// Page slicing
	pageSlice pageS;
	TIM tim;
	uint16_t crossSlotBoundary;

	/*pageSliceCount = 0 means:
	 *
	 *  - if pageSliceLength > 1, 32nd TIM in this DTIM can contain DL information for STAs that do not support
	 * 	  page slicing and for STAs who's AID is within the 32nd block of this page and do support page slicing
	 *
	 *  - if pageSliceLength = 1, all STAs for which the AP has DL BU are included in the *only TIM* that is scheduled
	 *    within the DTIM
	 *
	 * */

	uint32_t pagePeriod=2;  	//  Number of Beacon Intervals between DTIM beacons that carry Page Slice element for the associated page
	uint8_t pageIndex = 0;
	uint32_t pageSliceLength=1; //  Number of blocks in each TIM for the associated page except for the last TIM (1-31) (value 0 is reserved);
	uint32_t pageSliceCount=2;  //  Number of TIMs in a single page period (1-31)
	uint8_t blockOffset = 0;  	//  The 1st page slice starts with the block with blockOffset number
	uint8_t timOffset = 0;    	//  Offset in number of Beacon Intervals from the DTIM that carries the first page slice of the page
	/*
	 * Common configuration parameters
	 * */
	double simulationTime = 60; //60
	uint32_t seed = 1;
	int NRawSta;
	uint32_t Nsta;
	uint32_t BeaconInterval = 102400;

	double datarate = 7.8;
	double bandWidth = 2;
	string rho="200"; //50

	double totaltraffic = 0;//2;

	string visualizerIP = "localhost"; // empty string if no visualization TODO
	int visualizerPort = 7707;
	double visualizerSamplingInterval = 1;

	string name = "test"; // empty string if no visualization TODO
	string APPcapFile = "appcap"; // empty string if no visualization TODO
	string NSSFile = "test.nss";

	/*
	 * Le's config params
	 * */
	uint32_t payloadSize = 200;
	string folder="./scratch/";
	string file="./scratch/mac-sta.txt";
	string TrafficPath="./OptimalRawGroup/traffic/data-32-0.82.txt";
	bool S1g1MfieldEnabled=false;
	string RAWConfigFile = "./OptimalRawGroup/RawConfig-test.txt";
	string DataMode = "MCS2_0";
	string OutputPath = "./OptimalRawGroup/results-coap/";
	bool PrintStats = false;
	/*
	 * Amina's configuration parameters
	 * */
	bool useV6 = false; //false
	int nControlLoops = -1;//  = 100;
	//uint32_t coapPayloadSize = 0;//  = 15;

	uint32_t cycleTime; //us
	uint32_t trafficInterval = 5000; //ms 55,110,210,310,410,515,615,720,820,950,1024 beacon interval *4
	uint32_t trafficIntervalDeviation = 1000; //1000 discuss with Jeroen

	int SlotFormat=0; //0;
	int NRawSlotCount=0; //162;
	uint32_t NRawSlotNum=0;
	uint32_t NGroup=0;

	/*
	 * tcpipcamera configuration parameters
	 * */
	double ipcameraMotionPercentage = 0;//1; //10.1
	uint16_t ipcameraMotionDuration = 0;//10; //60
	uint16_t ipcameraDataRate = 0;//128; //20
	uint32_t MinRTO = 0;//81920000; //819200
	uint32_t TCPConnectionTimeout = 0;//6000000;
	uint32_t TCPSegmentSize  = 0;//3216; //536
	uint32_t TCPInitialSlowStartThreshold = 0;//0xffff;
	uint32_t TCPInitialCwnd = 0;//1;

	int ContentionPerRAWSlot=0; //-1
	bool ContentionPerRAWSlotOnlyInFirstGroup=false; //false

	double propagationLossExponent = 3.67; //3.76
	double propagationLossReferenceLoss = 8;

	bool APAlwaysSchedulesForNextSlot = false;
	uint32_t APScheduleTransmissionForNextSlotIfLessThan = 0;// = 5000;


	uint32_t firmwareSize = 0;// = 1024 * 500;
	uint16_t firmwareBlockSize = 0;// = 1024;
	double firmwareNewUpdateProbability;// = 0.01;
	double firmwareCorruptionProbability;// = 0.01;
	uint32_t firmwareVersionCheckInterval;// = 1000;

	uint16_t sensorMeasurementSize;// = 54; //1024

	uint16_t MaxTimeOfPacketsInQueue = 100; //100

	uint16_t CoolDownPeriod = 3; //60

	Configuration();
	Configuration(int argc, char *argv[]);

};
