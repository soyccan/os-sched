#!/bin/sh

get_stdev() {
    cat $1 | sed -n 's/.*stdev *= *//p'
}

line='ROUND,TIME UNIT STDEV'
for x in FIFO RR SJF PSJF; do
    for y in $(seq 1 5); do
        line=${line},${x}_${y}
    done
done
echo $line


i=100
while true; do
    [ ! -e $i ] && break

    stdev=$(get_stdev $i/timeunit_res)
    line=$i,$stdev

    for x in FIFO RR SJF PSJF; do
        for y in $(seq 1 5); do
            line=${line},$(get_stdev $i/err/${x}_${y}.txt)
        done
    done

    echo $line

    i=$(($i + 1))
done
