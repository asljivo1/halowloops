clear
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=3 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false --MaxNumPackets=10 --Nsta=10" -j4

#./waf --run test --command-template=" gdb --args %s <args> --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=3 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false --MaxNumPackets=10 --Nsta=10"

#--MaxNumPackets=10
