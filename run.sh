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
echo "Executing Isopod worker with $2 threads"

mkdir log
for ((t=$2-1; t>0; t=$t-1)); do
    prefix="log/log.$(date +%Y-%m-%d)_T"
    suffix=".log"
    filename="$prefix$t$suffix"
    ./bin/main $1 | tee $filename & \
    echo "Worker $t on-line"
done
./bin/main $1 | tee "log/log.$(date +%Y-%m-%d)_T0.log"
echo "Worker 0 on-line"
