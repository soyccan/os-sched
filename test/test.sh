#!/bin/sh

make -C .. clean
make -C .. DEBUG=0

# starting from 200
round=199
while [ $round -lt 1000 ]; do
    for x in FIFO RR SJF PSJF; do
        for y in $(seq 1 5); do

            round=$(($round + 1))
            [ -e "$round" ] && continue

            echo Round $round
            mkdir -pv $round/err

            f=${x}_${y}
            echo Executing $f

            sudo ../demo < ../OS_PJ1_Test/TIME_MEASUREMENT.txt > $round/${f}_timeunit_stdout
            dmesg | grep '\[Project1\]' | tail -n 15 > $round/${f}_timeunit_dmesg
            python timeunit.py $round/${f}_timeunit_stdout $round/${f}_timeunit_dmesg >> $round/${f}_timeunit_res

            n=$(cat ../OS_PJ1_Test/$f.txt | tr -d '\r' | head -n 2 | tail -n 1)

            sudo ../demo < ../OS_PJ1_Test/$f.txt > $round/${f}_stdout.txt
            dmesg | grep '\[Project1\]' | tail -n 15 > $round/${f}_dmesg.txt


            python accuracy.py \
                $n \
                ../corr_out.1/${f}.txt \
                $round/${f}_stdout.txt \
                $round/${f}_dmesg.txt \
                > $round/err/${f}.txt
        done
    done
done
