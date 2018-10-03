./waf --run "test --BeaconInterval=15360 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-2-2-1-15360-0-31.txt\"  --TrafficInterval=1000 --CycleTime=32 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4


