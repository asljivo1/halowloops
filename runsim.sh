#64
./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-3-3-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=1" -j4

#64
./waf --run "test --BeaconInterval=39936 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-6-6-1-39936-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=6 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=39936 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-8-8-1-39936-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=8 --DataMode=\"MCS2_8\" --seed=1" -j4

#128
./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-12-12-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=12 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-16-16-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=16 --DataMode=\"MCS2_8\" --seed=1" -j4

#256
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-24-24-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=24 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-30-30-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=30 --DataMode=\"MCS2_8\" --seed=1" -j4

#512
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-45-45-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=45 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-50-50-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=50 --DataMode=\"MCS2_8\" --seed=1" -j4


