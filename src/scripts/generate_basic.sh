#!/bin/bash
RAM=$((4*1024)) # in Megabytes
dims=""
mIntervals=""
nPointsPerCell=""
for dim in {1..5}
do
	dims="${dims} -d${dim}"
done
for m in 2 10 20 50 100 1000 10000 100000 1000000
do
	mIntervals="${mIntervals} -m${m}"
done
for M in 10 100 1000 10000 100000 1000000
do
	nPointsPerCell="${nPointsPerCell} -M${M}"
done

echo "Dimensions: " $dims
echo "mIntervalsPerDim " $mIntervals
echo "nPointsPerCell " $nPointsPerCell
./bin/hypercube generate $dims $mIntervals $nPointsPerCell --ram $RAM

