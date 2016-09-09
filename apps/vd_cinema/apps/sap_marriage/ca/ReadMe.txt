------------------------------------------------------------------------------------------

-. Create RootCA PrivateKey
# openssl genrsa -out rootkey.pem 2048

-. Create RootCA
# openssl req -x509 -config rootca.cnf -new -key rootkey.pem -out rootcert.pem -outform Pem

-. Create PrivateKey
# openssl genrsa -out private_key.pem 2048

-. Create Certificate Request
# openssl req -config sec.cnf -new -key private_key.pem -out request_test.pem

-. Sign Certificate
# openssl ca -config sec.cnf -in request_test.pem

-. Verify Certificate
# openssl verify -CAfile rootcert.pem 00.pem
 
-. Display Certification Information
# openssl x509 -in 00.pem -text


openssl genrsa -out rootkey.pem 2048;openssl req -x509 -config rootca.cnf -new -key rootkey.pem -out rootcert.pem -outform Pem;\
openssl genrsa -out private_key.pem 2048;\
openssl req -config sec.cnf -new -key private_key.pem -out request_test.pem -batch;\
openssl ca -config sec.cnf -in request_test.pem -batch;\
openssl verify -CAfile rootcert.pem 01.pem;openssl x509 -in 01.pem -text



------------------------------------------------------------------------------------------

-. Create private Key
# openssl genrsa -out private_key.pem 2048

-. Create Certificate
# openssl req -sha1 -new -x509 -key private_key.pem -out certificate.pem -days 365 -subj /CN=PR -set_serial 0

-. Get public key from Certificate
# openssl x509 -pubkey -noout -in certificate.pem

-. Verify Certificate
# openssl verify certificate.pem

-. Display Certification Information
# openssl x509 -in certificate.pem -text


openssl genrsa -out private_key.pem 2048;openssl req -sha1 -new -x509 -key private_key.pem -out certificate.pem -days 365 -subj /CN=PR -set_serial 0;openssl verify certificate.pem;openssl x509 -in certificate.pem -text



------------------------------------------------------------------------------------------

-. Certificate Verify

# openssl s_server -accept 43684 -key /root/ca/private_key_test.pem -cert /root/ca/00.pem -CAfile /root/ca/rootcert.pem 

-accept arg   - port to accept on (default is 4433)
-cert arg     - certificate file to use
-CAfile arg   - PEM format file of CA's
-key arg      - Private Key file to use, in cert file if
                not specified (default is server.pem)


# openssl s_client -showcerts -connect 192.168.1.222:43684

-showcerts    - show all certificates in the chain
-connect host:port - who to connect to (default is localhost:4433)

