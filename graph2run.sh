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
NumLoops=3

./scratch/rawgenerateloops.sh 50 $(($NumLoops+1)) 1 25600
./runloop.sh $NumLoops 25600 51200

./scratch/rawgenerateloops.sh 50 $(($NumLoops+1)) 1 15360
./runloop.sh $NumLoops 15360 51200

./scratch/rawgenerateloops.sh 50 $(($NumLoops+1)) 1 40960
./runloop.sh $NumLoops 40960 102400

./scratch/rawgenerateloops.sh 50 $(($NumLoops+1)) 1 61440
./runloop.sh $NumLoops 61440 153600


for k in 307200 409600 512000
do
	./scratch/rawgenerateloops.sh 50 $(($NumLoops+1)) 1 102400
	./runloop.sh $NumLoops 102400 $k
done
