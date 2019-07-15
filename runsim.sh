./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=10 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=9 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=8 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=7 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=6 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=5 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=4 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=3 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=2 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=1 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5" -j4

#rm optimization/*
#rm results-coap/*
#rm *.txt

#./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=2 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=7 --EnableVirtualAids=false --Nsta=15" -j4







#./waf --run test --command-template=" gdb --args %s <args> --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=2 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=7 --EnableVirtualAids=false --Nsta=15"

#./waf --run test --command-template=" gdb --args %s <args> --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=3 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=60000 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=10 --EnableVirtualAids=false --Nsta=50 --ProcessingDelay=5"

#--MaxNumPackets=10

#src/wifi/model/s1g-raw-control.cc

#TO CHECK
##################################################
#SIGFPE (m=10): ./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=2 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-10-4-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=51200 --nControlLoops=1 --DataMode=\"MCS2_8\" --seed=7 --EnableVirtualAids=false --Nsta=15" -j4
#b src/wifi/model/s1g-raw-control.cc:1897

#b src/wifi/model/s1g-raw-control.cc:1736 1714 1399 2150
