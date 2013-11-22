#!/bin/bash
# filename launch_multiple_clients.sh

make 

GENERAL2=129.219.102.8
GENERAL1=129.219.10.241
PORTNUM=65432
SERVER_IP=${GENERAL1}
if [ -z $1 ]
then
    NUM_CLIENTS=5
else
    NUM_CLIENTS=$1
fi


if [ -e "inc.txt" ]
then
    echo $(echo "$(cat inc.txt) + 1" | bc ) > inc.txt
else
    echo 0 > inc.txt
fi

echo "Beginning multiple client test with incarnation number = $(cat inc.txt)"

for (( i=1; i<=$NUM_CLIENTS; i++ ))
do
    CLIENTNUM=$i
    echo "Launching Client $CLIENTNUM, connecting to server at ${SERVER_IP}:${PORTNUM}"
    ./UDPClient $SERVER_IP $PORTNUM $CLIENTNUM > output_client_${CLIENTNUM}.tmp & 
    sleep 1 
done
echo "All client operations running in background"
