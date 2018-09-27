#2 loop 
./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-52-3-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-32-3-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-152-3-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=39936 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-152-3-1-39936-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4


