#!/bin/bash
# filename launch_multiple_clients.sh

GENERAL2=129.219.102.8
GENERAL1=129.219.10.241
PORTNUM=65432

SERVER_IP=${GENERAL2}
for i in {0..5}
do
    CLIENTNUM=i
    ./client SERVER_IP PORTNUM $CLIENTNUM > output_client_${i}.tmp & 
done
echo "All client operations running in background"
