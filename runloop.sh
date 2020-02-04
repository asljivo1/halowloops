#!/bin/bash

if [ $# -ne 3 ]
then
echo "parameters missing"
exit 1
fi

NumLoops=$1
CycleTime=$3
Bi=$2

g=$(($NumLoops+1))

#tptx=$((1000*$2))

for i in {2..10}
do
./waf --run "test --BeaconInterval=$Bi --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-50-$g-1-$Bi-0-31.txt\"  --TrafficInterval=1000 --CycleTime=$CycleTime --nControlLoops=$NumLoops --DataMode=\"MCS2_8\" --seed=$i  --Nsta=50 " -j4
done
