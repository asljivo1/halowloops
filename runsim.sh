# BI=19456 Ts=80
./waf --run "test --BeaconInterval=19456 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-19456-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4

./waf --run "test --BeaconInterval=19456 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-151-2-1-19456-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4

./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4

./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=300 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-151-2-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80 --nControlLoops=1 --DataMode=\"MCS2_0\" --seed=1" -j4


# BI=50 Ts=128


# BI=70,60 Ts=160


# BI=80,70 Ts=256


# BI=102,80 Ts=512

