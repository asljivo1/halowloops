clear
./waf --run "test --BeaconInterval=102400 --pagePeriod=4 --pageSliceLength=1 --pageSliceCount=4 --simulationTime=30 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-3-1-102400-0-31.txt\"  --TrafficInterval=5000 --CycleTime=512 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false" -j4

#./waf --run test --command-template=" gdb --args %s <args> --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=15 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51.2 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false"
