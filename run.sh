if [ $# -ne 1 ]; then
    echo "Usage: server_ip"
    exit
fi

make
echo ""
echo "---"
echo ""
echo "Executing Isopod worker"
./bin/main $1
