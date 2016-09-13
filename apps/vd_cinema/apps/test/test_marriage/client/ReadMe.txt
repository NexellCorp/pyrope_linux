-. Create private Key
# openssl genrsa -out private_key.pem 1024

-. Create Public Key
# openssl rsa -in private_key.pem -pubout -out public_key.pem

-. Create Certificate
# openssl req -sha1 -new -x509 -key private_key.pem -out certificate.pem -days 365 -subj /CN=192.168.1.220 -set_serial 0

-. Get public key from Certificate
# openssl x509 -pubkey -noout -in signed_certificate.pem

-. Verify Certificate
# openssl verify signed_certificate.pem

-. Display Certification Information
# openssl x509 -in signed_certificate.pem -text

-. Get IP Address
# ifconfig eth0 | sed -n '/addr:/p' | awk '{print $2}' | sed 's/addr://g' | sed '/^$/d'
