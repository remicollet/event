#!/bin/bash - 

# generate a private key
openssl genrsa -out server.key 1024

# generate a self signed cert:
openssl req -new -key server.key -x509 -days 3653 -out server.crt

#     enter fields... (may all be empty when cert is only used privately)

# generate the pem file:
cat server.key server.crt > server.pem

# secure permissions:
chmod 600 server.key server.pem
