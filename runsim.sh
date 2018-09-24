#Ts = 64
#Nloops=1, 100 diff BIs, 150+200 sensors -> max thr=118.4
./waf --run "test --BeaconInterval=25600 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-25600-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=39936 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-101-2-1-39936-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4


./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-151-2-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-201-2-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4

#Nloops=2, 150,200 sensors -> thr=108,134
./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-152-3-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-202-3-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#Nloops=4, 20 sensors -> thr=138.24
./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-104-5-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=4 --DataMode=\"MCS2_8\" --seed=1" -j4









