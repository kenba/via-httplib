#!/bin/bash

# Create certificate authority signing key and certificate, valid for 10 years
# Note: take care to remember the PEM pass phrase!
openssl req -new -x509 -days 3650 -keyout ca-key.pem -out ca-certificate.pem

echo ""
echo "-----------------------------------------------------------------------"
echo "Create server private key and cerificate"
echo "-----------------------------------------------------------------------"
echo ""

# Create server private key
openssl genrsa -out server-key.pem 4096

# Create server certificate signing request
# Note: use `localhost` as common name for local testing`
openssl req -new -sha256 -key server-key.pem -out server-csr.pem

# Create and sign a server certificate, valid for a year
openssl x509 -req -days 365 -in server-csr.pem -CA ca-certificate.pem -CAkey ca-key.pem -CAcreateserial -out server-certificate.pem

mkdir server
mv server-key.pem server/.
mv server-certificate.pem server/.
rm server-csr.pem

echo ""
echo "-----------------------------------------------------------------------"
echo "Create client private key and cerificate"
echo "-----------------------------------------------------------------------"
echo ""

# Create client private key
openssl genrsa -out client-key.pem 4096

# Create client certificate signing request
openssl req -new -sha256 -key client-key.pem -out client-csr.pem

# Create and sign a client certificate, valid for a year
openssl x509 -req -days 365 -in client-csr.pem -CA ca-certificate.pem -CAkey ca-key.pem -CAcreateserial -out client-certificate.pem

mkdir client
mv client-key.pem client/.
mv client-certificate.pem client/.
rm client-csr.pem
