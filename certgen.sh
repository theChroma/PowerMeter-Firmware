#!/bin/bash
#-----------------------------------------------------------------------------------------------------------------------------------------------------------
echo "[certgen] Setting everything up..."

TEMP_DIR=./certificate_generation/
OUT_DIR=./data/SSL/

set -e
mkdir -p $TEMP_DIR
#-----------------------------------------------------------------------------------------------------------------------------------------------------------
echo "[certgen] Generating CA..."

openssl genrsa -out "${TEMP_DIR}CAkey.pem" 1024

cat > "${TEMP_DIR}CA.conf" << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no
[ req_distinguished_name ]
C = DE
ST = BE
L = Berlin
O = MyCompany
CN = myca.local
EOF
openssl req -new -x509 -days 3650 -key "${TEMP_DIR}CAkey.pem" -out "${TEMP_DIR}CAcert.pem" -config "${TEMP_DIR}CA.conf"
echo "01" > "${TEMP_DIR}CAcert.srl"


#-----------------------------------------------------------------------------------------------------------------------------------------------------------
echo "[certgen] Generating Private Key..."
openssl genrsa -out "${OUT_DIR}PrivateKey.pem" 1024

cat > "${TEMP_DIR}Certificate.conf" << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no
[ req_distinguished_name ]
C = DE
ST = BE
L = Berlin
O = MyCompany
CN = esp32.local
EOF

echo "[certgen] Generating CSR..."
openssl req -new -key "${OUT_DIR}PrivateKey.pem" -out "${TEMP_DIR}SigningRequest.csr" -config "${TEMP_DIR}Certificate.conf"

echo "[certgen] Generating Certificate..."
openssl x509 -days 3650 -CA "${TEMP_DIR}CAcert.pem" -CAkey "${TEMP_DIR}CAkey.pem" -in "${TEMP_DIR}SigningRequest.csr" -req -out "${OUT_DIR}Certificate.pem"

echo "[certgen] Verifing..."
openssl verify -CAfile "${TEMP_DIR}CAcert.pem" "${OUT_DIR}Certificate.pem"

echo "[certgen] Generating in DER Format..."
openssl rsa -in "${OUT_DIR}PrivateKey.pem" -outform DER -out "${OUT_DIR}PrivateKey.der"
openssl x509 -in "${OUT_DIR}Certificate.pem" -outform DER -out "${OUT_DIR}Certificate.der"

#-----------------------------------------------------------------------------------------------------------------------------------------------------------
echo "[certgen] Cleaning up..."
rm -f --recursive $TEMP_DIR

#-----------------------------------------------------------------------------------------------------------------------------------------------------------
echo "[certgen] Successfully generatet Certificates"