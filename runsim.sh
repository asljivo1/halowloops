clear
#MCS8
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=60 --payloadSize=64 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-1-5-102400-0-31.txt\"  --TrafficInterval=1000 --rho=100 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false --nControlLoops=0" -j4

#./waf --run test --command-template=" gdb --args %s <args> --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=60 --payloadSize=64 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-1-5-102400-0-31.txt\"  --TrafficInterval=1000 --rho=100 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false --nControlLoops=0"


./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=60 --payloadSize=64 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-1-5-102400-0-31.txt\"  --TrafficInterval=1000 --rho=110 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false --nControlLoops=0" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=60 --payloadSize=64 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-1-5-102400-0-31.txt\"  --TrafficInterval=1000 --rho=120 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false --nControlLoops=0" -j4

#MCS7
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=60 --payloadSize=64 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-1-5-102400-0-31.txt\"  --TrafficInterval=1000 --rho=120 --DataMode=\"MCS2_7\" --seed=1 --EnableVirtualAids=false --nControlLoops=0" -j4
