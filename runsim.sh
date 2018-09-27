#1 loop 
./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-51-2-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=60416 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-271-2-1-60416-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-276-2-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4


