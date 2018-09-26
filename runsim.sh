#Ts = 64

#all BIs 100 stas
./waf --run "test --BeaconInterval=19456 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-19456-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=9216 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-9216-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=9216 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-9216-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=9216 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-9216-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=9216 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-9216-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=9216 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-9216-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=9216 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-9216-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4
