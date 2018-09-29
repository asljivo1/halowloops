#2 loops
#80
./waf --run "test --BeaconInterval=39936 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-172-3-1-39936-0-31.txt\"  --TrafficInterval=1000 --CycleTime=80 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#128
./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-232-3-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-252-3-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=128 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#160
./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-242-3-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-262-3-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#256
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-272-3-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-262-3-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=256 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#512
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-272-3-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-302-3-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=2 --DataMode=\"MCS2_8\" --seed=1" -j4

#4 loops
#160
./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-214-5-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=4 --DataMode=\"MCS2_8\" --seed=1" -j4

./waf --run "test --BeaconInterval=70656 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-224-5-1-70656-0-31.txt\"  --TrafficInterval=1000 --CycleTime=160 --nControlLoops=4 --DataMode=\"MCS2_8\" --seed=1" -j4

#512
./waf --run "test --BeaconInterval=102400 --pagePeriod=1 --pageSliceLength=1 --pageSliceCount=0 --simulationTime=300 --payloadSize=64 --rho=100 --TrafficType=\"coap\" --PrintStats=true --RAWConfigFile=\"./OptimalRawGroup/RawConfig-254-5-1-102400-0-31.txt\"  --TrafficInterval=1000 --CycleTime=512 --nControlLoops=4 --DataMode=\"MCS2_8\" --seed=1" -j4
