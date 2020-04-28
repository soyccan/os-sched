#!/bin/sh
if [ -z "$1" ]; then
    echo "Usage: $0 <pid_of_scheduler>"
    exit 255
fi
sudo perf sched timehist -C 0 | awk "/$1/ { print \"\x1b[31m\" \$0 \"\x1b[0m\" } !/$1/ { print \$0 }" | less
