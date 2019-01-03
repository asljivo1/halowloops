#Tcyc = 32
./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-52-3-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#Tcyc = 64
./waf --run "test --BeaconInterval=29696 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-132-3-1-29696-0-31.txt\"  --TrafficInterval=1000 --CycleTime=64 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4


#Tcyc = 128
./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-202-3-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#Tcyc = 160
./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-262-3-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#Tcyc = 256
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-272-3-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#Tcyc = 512
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-272-3-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#./waf --run "test --BeaconInterval=51200 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-2-1-51200-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4
