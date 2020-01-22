#!/bin/bash

if [ ! -d optimization/ ]
then
	mkdir optimization
else
	rm optimization/*
fi

if [ ! -d results-coap/ ]
then
        mkdir results-coap
else
	rm results-coap/*
fi

rm *.txt

# nLoops=2, CycleTime=200,  m=8, tptx=3
./runsim13.sh
mkdir 3L200T_m8tptx3/
mv optimization 3L200T_m8tptx3/
mv results-coap 3L200T_m8tptx3/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=200,  m=8, tptx=5
./runsim14.sh
mkdir 3L200T_m8tptx5/
mv optimization 3L200T_m8tptx5/
mv results-coap 3L200T_m8tptx5/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=300,  m=16, tptx=3
./runsim15.sh
mkdir 3L300T_m16tptx3/
mv optimization 3L300T_m16tptx3/
mv results-coap 3L300T_m16tptx3/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=300,  m=16, tptx=5
./runsim16.sh
mkdir 3L300T_m16tptx5/
mv optimization 3L300T_m16tptx5/
mv results-coap 3L300T_m16tptx5/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=300,  m=8, tptx=3
./runsim17.sh
mkdir 3L300T_m8tptx3/
mv optimization 3L300T_m8tptx3/
mv results-coap 3L300T_m8tptx3/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=300,  m=8, tptx=5
./runsim18.sh
mkdir 3L300T_m8tptx5/
mv optimization 3L300T_m8tptx5/
mv results-coap 3L300T_m8tptx5/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=400,  m=16, tptx=3
./runsim19.sh
mkdir 3L400T_m16tptx3/
mv optimization 3L400T_m16tptx3/
mv results-coap 3L400T_m16tptx3/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=400,  m=16, tptx=5
./runsim20.sh
mkdir 3L400T_m16tptx5/
mv optimization 3L400T_m16tptx5/
mv results-coap 3L400T_m16tptx5/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=400,  m=8, tptx=3
./runsim21.sh
mkdir 3L400T_m8tptx3/
mv optimization 3L400T_m8tptx3/
mv results-coap 3L400T_m8tptx3/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=400,  m=8, tptx=5
./runsim22.sh
mkdir 3L400T_m8tptx5/
mv optimization 3L400T_m8tptx5/
mv results-coap 3L400T_m8tptx5/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=500,  m=16, tptx=3
./runsim23.sh
mkdir 3L500T_m16tptx3/
mv optimization 3L500T_m16tptx3/
mv results-coap 3L500T_m16tptx3/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=500,  m=16, tptx=5
./runsim24.sh
mkdir 3L500T_m16tptx5/
mv optimization 3L500T_m16tptx5/
mv results-coap 3L500T_m16tptx5/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=500,  m=8, tptx=3
./runsim25.sh
mkdir 3L500T_m8tptx3/
mv optimization 3L500T_m8tptx3/
mv results-coap 3L500T_m8tptx3/
mkdir optimization
mkdir results-coap


# nLoops=2, CycleTime=500,  m=8, tptx=5
./runsim26.sh
mkdir 3L500T_m8tptx5/
mv optimization 3L500T_m8tptx5/
mv results-coap 3L500T_m8tptx5/
mkdir optimization
mkdir results-coap


