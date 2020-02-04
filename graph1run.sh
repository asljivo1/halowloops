#!/bin/bash

if [ ! -d optimization/ ]
then
        mkdir optimization
fi

if [ ! -d results-coap/ ]
then
        mkdir results-coap
fi
rm *.txt

# Tcyc=51ms, loops=1-5, BI=15ms
for i in {1..5}
do
	./scratch/rawgenerateloops.sh 50 $(($i+1)) 1 15360
	./runloop.sh $i 50000 15360
done

# Tcyc=51ms, loops=1-5, BI=25ms
for i in {1..5}
do
        ./scratch/rawgenerateloops.sh 50 $(($i+1)) 1 25600
        ./runloop.sh $i 50000 25600
done

# 204ms, loops=1-5, BI=102ms
for i in {1..5}
do
        ./scratch/rawgenerateloops.sh 50 $(($i+1)) 1 102400
        ./runloop.sh $i 200000 102400
done

# 204ms, loops=1-5, BI=51ms
for i in {1..5}
do
        ./scratch/rawgenerateloops.sh 50 $(($i+1)) 1 51200
        ./runloop.sh $i 200000 51200
done
