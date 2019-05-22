clear
rm rawOpt.log
rm OptRaw_c++.lp
rm OptRaw_c++.mps

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=20 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80000 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=8 --EnableVirtualAids=false --Nsta=3" -j4

#./waf --run test --command-template=" gdb --args %s <args> --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=10 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64000 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=4 --EnableVirtualAids=false --Nsta=8"

#./waf --run test --command-template=" gdb --args %s <args> --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=20 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80000 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=10 --EnableVirtualAids=false --Nsta=2"

#--MaxNumPackets=10

#src/wifi/model/s1g-raw-control.cc

#TO CHECK
##################################################
#SIGFPE (m=10): ./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=20 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=100000 --nControlLoops=4 --DataMode=\"MCS2_8\" --seed=10 --EnableVirtualAids=false --Nsta=4" -j4
