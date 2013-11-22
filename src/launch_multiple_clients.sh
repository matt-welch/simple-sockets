#!/bin/bash
# filename launch_multiple_clients.sh

make debug

GENERAL2=129.219.102.8
GENERAL1=129.219.10.241
PORTNUM=65432

SERVER_IP=${GENERAL1}
for i in {1..5}
do
    CLIENTNUM=$i
    echo "Launching Client $CLIENTNUM, connecting to server at ${SERVER_IP}:${PORTNUM}"
    ./client $SERVER_IP $PORTNUM $CLIENTNUM > output_client_${CLIENTNUM}.tmp & 
    sleep 1 
done
echo "All client operations running in background"
