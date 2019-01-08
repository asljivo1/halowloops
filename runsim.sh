#Tcyc = 128
./waf --run "test --BeaconInterval=51200 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-232-3-1-51200-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#Tcyc = 512
./waf --run "test --BeaconInterval=204800 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-282-3-1-204800-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#./waf --run "test --BeaconInterval=51200 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-2-1-51200-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1" -j4
