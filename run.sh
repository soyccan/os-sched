#!/bin/sh

sudo ./demo < OS_PJ1_Test/TIME_MEASUREMENT.txt
python timeunit.py

p='FIFO RR SJF PSJF'
for x in $p; do
    for y in $(seq 1 5); do
        f=${x}_${y}
        if [ -e "diff/$f.txt" ] || [ ! -e "OS_PJ1_Test/$f.txt" ]; then
            continue
        fi
        echo Executing $f
        sudo ./demo < OS_PJ1_Test/$f.txt > out/$f.txt
        python accuracy.py $f > diff/$f.txt
    done
done
