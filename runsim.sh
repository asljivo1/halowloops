#Ts = 32
#Nloops=4, 20 sensors -> thr=138.24
./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-24-5-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=4 --DataMode=\"MCS2_8\" --seed=1" -j4


#Nloops=2, 100 sensors -> thr=115.2
./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-102-3-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4


#Nloops=1, 100+150 sensors -> thr=83.2+108.8
./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-151-2-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4



