# BI=80896 all Tss
./waf --run "test --BeaconInterval=80896 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-51-2-1-80896-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4
./waf --run "test --BeaconInterval=80896 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-51-2-1-80896-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4
./waf --run "test --BeaconInterval=80896 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-51-2-1-80896-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4
./waf --run "test --BeaconInterval=80896 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-51-2-1-80896-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4
./waf --run "test --BeaconInterval=80896 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-51-2-1-80896-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4
./waf --run "test --BeaconInterval=80896 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-51-2-1-80896-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4



