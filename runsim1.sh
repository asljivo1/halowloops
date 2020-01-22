rm results-coap/*
rm optimization/*
rm *.txt

# m=8, 51.2, tptx=3

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=10 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=9 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=8 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=7 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=6 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=5 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=4 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=3 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=2 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=100 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=3 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5 --TxTimeMip=3000 --m=8" -j4

