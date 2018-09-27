#2 loop 
./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-202-3-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=60416 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-202-3-1-60416-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-202-3-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=60416 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-202-3-1-60416-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-302-3-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-352-3-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4



