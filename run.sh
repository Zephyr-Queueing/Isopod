#!/bin/sh

if [ $# -ne 2 ]; then
    echo "Usage: server_ip thread_num"
    exit
fi
if [ $2 -le 0 ]; then
    echo "Invalid number of threads"
    exit
fi

echo "Preparing..."
make
echo ""
echo "..."
echo ""
echo "Executing Isopod worker with $2 thread(s)"

mkdir log

t=$(( $2 - 1 ))
while [ "$t" -gt 0 ]; do
    prefix="log/log_T"
    suffix=".log"
    filename="$prefix$t$suffix"
    ./bin/main $1 >> $filename & \
    echo "Worker $t on-line"
    t=$(( t - 1 ))
done

./bin/main $1 >> "log/log_T0.log"
echo "Worker 0 on-line"
