clear
./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-76-2-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32000 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false" -j4

./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-201-2-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64000 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false" -j4

./waf --run "test --BeaconInterval=51200 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-251-2-1-51200-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80000 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false" -j4

./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-271-2-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128000 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-301-2-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256000 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false" -j4

./waf --run "test --BeaconInterval=204800 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-311-2-1-204800-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512000 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false" -j4

#./waf --run test --command-template=" gdb --args %s <args> --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=3 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=5 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false --MaxNumPackets=10"
