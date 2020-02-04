#!/bin/bash

if [ $# -ne 4 ]
then
echo "parameters missing"
exit 1
fi

NumSta=$1
NRawGroups=$2
NumSlot=$3
beaconinterval=$4
pageSliceCount=0
pageSliceLen=31
crossSlotBoundary=1









RAWConfigPath="./OptimalRawGroup/RawConfig-$NumSta-$NRawGroups-$NumSlot-$beaconinterval-$pageSliceCount-$pageSliceLen.txt"
#-$pagePeriod-$pageSliceLength-$pageSliceCount

./waf --run "rawgenerateloops --NRawSta=$NumSta --NGroup=$NRawGroups --NumSlot=$NumSlot --RAWConfigPath=$RAWConfigPath --beaconinterval=$beaconinterval --pageSliceCount=$pageSliceCount --pageSliceLen=$pageSliceLen --crossSlotBoundary=$crossSlotBoundary --MCSIndex=8 "

#./waf --run RAW-generate --command-template="gdb --args %s <args> --NRawSta=$NumSta --NGroup=$NRawGroups --NumSlot=$NumSlot --RAWConfigPath=$RAWConfigPath --beaconinterval=$beaconinterval --pageSliceCount=$pageSliceCount --pageSliceLen=$pageSliceLen"

#sed -i "s/10/21/g" $RAWConfigPath


