#!/bin/bash - 

if [ $# -gt 0 ]; then
	server_port=$1
else
	server_port=9999
fi

DIR="$( cd "$( dirname "$0" )" && pwd )"

cert_file=$DIR"/client.pem"
ca_file=$DIR"/server.crt"

socat - SSL:127.0.0.1:$server_port,verify=1,cert=$cert_file,cafile=$ca_file
