#64
./waf --run "test --BeaconInterval=39936 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-7-1-7-39936-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=7 --DataMode=\"MCS2_8\" --seed=1" -j4

#128
./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-17-1-17-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=17 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-18-1-18-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=18 --DataMode=\"MCS2_8\" --seed=1" -j4

#256
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-36-1-36-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=36 --DataMode=\"MCS2_8\" --seed=1" -j4

#512
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-40-1-40-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=40 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-40-40-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=40 --DataMode=\"MCS2_8\" --seed=1" -j4
