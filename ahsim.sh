clear 
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=500 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-loops.txt\"  --TrafficInterval=1000 --CycleTime=80 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4

#./waf --run test --command-template="gdb --args %s <args> --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=1 --payloadSize=300 --rho=400 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-loops.txt\"  --TrafficInterval=500 --CycleTime=500 --nControlLoops=2 " -j4



./waf --run "test --BeaconInterval=60416 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-60416-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=80896 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-80896-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=91136 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-91136-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4
