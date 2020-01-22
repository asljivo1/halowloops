#!/bin/bash

if [ ! -d optimization/ ]
then
	mkdir optimization
#else
	#rm optimization/*
fi

if [ ! -d results-coap/ ]
then
        mkdir results-coap
#else
	#rm results-coap/*
fi

rm *.txt

# nLoops=3, CycleTime=51.2,  m=8, tptx=3
#./runsim1.sh
#mkdir 3L51T_m8tptx3/
#mv optimization 3L51T_m8tptx3/
#mv results-coap 3L51T_m8tptx3/
#mkdir optimization
#mkdir results-coap


# nLoops=3, CycleTime=51.2,  m=8, tptx=5
#./runsim2.sh
#mkdir 3L51T_m8tptx5/
#mv optimization 3L51T_m8tptx5/
#mv results-coap 3L51T_m8tptx5/
#mkdir optimization
#mkdir results-coap


# nLoops=3, CycleTime=51,  m=16, tptx=3
./runsim.sh
mkdir 3L51T_m16tptx3/
mv optimization 3L51T_m16tptx3/
mv results-coap 3L51T_m16tptx3/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=51,  m=16, tptx=5
./runsim0.sh
mkdir 3L51T_m16tptx5/
mv optimization 3L51T_m16tptx5/
mv results-coap 3L51T_m16tptx5/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=100,  m=8, tptx=3
./runsim5.sh
mkdir 3L100T_m8tptx3/
mv optimization 3L100T_m8tptx3/
mv results-coap 3L100T_m8tptx3/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=100,  m=8, tptx=5
./runsim6.sh
mkdir 3L100T_m8tptx5/
mv optimization 3L100T_m8tptx5/
mv results-coap 3L100T_m8tptx5/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=100,  m=16, tptx=3
./runsim3.sh
mkdir 3L100T_m16tptx3/
mv optimization 3L100T_m16tptx3/
mv results-coap 3L100T_m16tptx3/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=100,  m=16, tptx=5
./runsim4.sh
mkdir 3L100T_m16tptx5/
mv optimization 3L100T_m16tptx5/
mv results-coap 3L100T_m16tptx5/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=150,  m=8, tptx=3
./runsim9.sh
mkdir 3L150T_m8tptx3/
mv optimization 3L150T_m8tptx3/
mv results-coap 3L150T_m8tptx3/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=150,  m=8, tptx=5
./runsim10.sh
mkdir 3L150T_m8tptx5/
mv optimization 3L150T_m8tptx5/
mv results-coap 3L150T_m8tptx5/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=150,  m=16, tptx=3
./runsim7.sh
mkdir 3L150T_m16tptx3/
mv optimization 3L150T_m16tptx3/
mv results-coap 3L150T_m16tptx3/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=150,  m=16, tptx=5
./runsim8.sh
mkdir 3L150T_m16tptx5/
mv optimization 3L150T_m16tptx5/
mv results-coap 3L150T_m16tptx5/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=200,  m=8, tptx=3
./runsim13.sh
mkdir 3L200T_m8tptx3/
mv optimization 3L200T_m8tptx3/
mv results-coap 3L200T_m8tptx3/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=200,  m=8, tptx=5
./runsim14.sh
mkdir 3L200T_m8tptx5/
mv optimization 3L200T_m8tptx5/
mv results-coap 3L200T_m8tptx5/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=200,  m=16, tptx=3
./runsim11.sh
mkdir 3L200T_m16tptx3/
mv optimization 3L200T_m16tptx3/
mv results-coap 3L200T_m16tptx3/
mkdir optimization
mkdir results-coap


# nLoops=3, CycleTime=200,  m=16, tptx=5
./runsim12.sh
mkdir 3L200T_m16tptx5/
mv optimization 3L200T_m16tptx5/
mv results-coap 3L200T_m16tptx5/
mkdir optimization
mkdir results-coap


./runall.sh
