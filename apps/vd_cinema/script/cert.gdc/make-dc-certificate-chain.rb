#!/usr/bin/ruby

# Copyright (c) 2016 Nexell Co., Ltd.
# Author: Biela Jo <doriya@@nexell.co.kr>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#
# Definition
#
rootca_prefix     = "ca"
inter_prefix      = "intermediate"
leaf_prefix       = "leaf"

cert_chain_file   = "dc-certificate-chain"

rootca_cnf_file   = rootca_prefix + ".cnf"
rootca_priv_file  = rootca_prefix + ".key"
rootca_cert_file  = rootca_prefix + ".self-signed.pem"

inter_cnf_file    = inter_prefix + ".cnf"
inter_csr_file    = inter_prefix + ".csr"
inter_priv_file   = inter_prefix + ".key"
inter_cert_file   = inter_prefix + ".signed.pem"

leaf_cnf_file     = leaf_prefix + ".cnf"
leaf_csr_file     = leaf_prefix + ".csr"
leaf_priv_file    = leaf_prefix + ".key"
leaf_cert_file    = leaf_prefix + ".signed.pem"

bit_of_key        = 2048

product_info_file = "/mnt/mmc/product_info.txt"
product_date      = "2009-01-01 00:00:00"

rootca_cnf = "\
[ req ]
distinguished_name = req_distinguished_name
x509_extensions    = v3_ca

[ v3_ca ]
keyUsage               = keyCertSign,cRLSign
basicConstraints       = critical,CA:true,pathlen:3
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid:always,issuer:always

[ req_distinguished_name ]
O  = Unique organization name
OU = Organization unit
CN = Entity and dnQualifier
"

inter_cnf = "\
[ default ]
distinguished_name = req_distinguished_name
x509_extensions    = v3_ca

[ v3_ca ]
keyUsage               = keyCertSign,cRLSign
basicConstraints       = critical,CA:true,pathlen:2
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid:always,issuer:always

[ req_distinguished_name ]
O  = Unique organization name
OU = Organization unit
CN = Entity and dnQualifier
"

leaf_cnf = "\
[ default ]
distinguished_name = req_distinguished_name
x509_extensions    = v3_ca

[ v3_ca ]
# pathlen not needed. Would the cert be rejected because of pathlen marked critical?
# Answer: no. see SMPTE 430-2-2006 section 6.2 Validation Rules - Check 5
keyUsage               = digitalSignature,keyEncipherment
basicConstraints       = critical,CA:false
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid,issuer:always

[ req_distinguished_name ]
O  = Unique organization name
OU = Organization unit
CN = Entity and dnQualifier
"


#
# Clean-up files ( for Test )
#
#old = Dir.glob( [ "#{rootca_prefix}.*", "#{inter_prefix}.*", "#{leaf_prefix}.*", "#{cert_chain_file}" ] )
#old.each do |file|
#	File.delete( file )
#	`sync`
#end


#
# Product Information
#
product_name = ''
product_serial = ''

if File.file?(product_info_file)
	File.open( product_info_file ).each do |line|
		product_info = line.split(/= */);

		case product_info[0]
		when "PRODUCT_NAME"
			if product_info[1].chomp != ''
				product_name = "." + product_info[1].chomp
			end
		when "PRODUCT_SERIAL"
			if product_info[1].chomp != ''
				product_serial = "." + product_info[1].chomp
			end
		end
	end
end


#
# Make Serial Number
#
if File.file?("/sys/devices/platform/cpu/uuid")
	cert_serial = `cat /sys/devices/platform/cpu/uuid`.chomp
	cert_serial = cert_serial.gsub( ':', '' )
	cert_serial = '0x' + cert_serial[0..15].upcase
else
	require 'securerandom'
	cert_serial = SecureRandom.hex(8)
	cert_serial = '0x' + cert_serial
end


#
# Check Zero Files
#
puts ""
puts "------------------------------------------------------------\n"
puts "-                                                          -\n"
puts "-  Check Files                                             -\n"
puts "-                                                          -\n"
puts "------------------------------------------------------------\n"

list = Dir.glob( [ "*" ] )
list.each do |file|
	if !file.include?(".cnf") && !file.include?(".csr") && !file.include?(".key") && !file.include?(".pem")
		next
	end

	if File.zero?(file)
		puts "> " + file + " : Unstable file."
		File.delete( file )
		`sync`
	else
		puts "> " + file + " : OK."
	end
end


#
# Check CNF files for Certificate
#
if !File.file?(rootca_cnf_file)
	File.open( rootca_cnf_file, "w" ) { |f| f.write( rootca_cnf ) }
	`sync`
end

if !File.file?(inter_cnf_file)
	File.open( inter_cnf_file, 'w' ) { |f| f.write( inter_cnf ) }
	`sync`
end

if !File.file?(leaf_cnf_file)
	File.open( leaf_cnf_file, 'w' ) { |f| f.write( leaf_cnf ) }
	`sync`
end


#
# Make Certificate
#
# Make RootCA Certificate
if !File.file?(rootca_priv_file) || !File.file?(rootca_cert_file)
	puts "------------------------------------------------------------\n"
	puts "-                                                          -\n"
	puts "-  Make RootCA Certificate                                 -\n"
	puts "-                                                          -\n"
	puts "------------------------------------------------------------\n"

	`openssl genrsa -out "#{rootca_priv_file}" "#{bit_of_key}"`
	`sync`

	rootca_dnq = `openssl rsa -outform PEM -pubout -in "#{rootca_priv_file}" | openssl base64 -d | dd bs=1 skip=24 2>/dev/null | openssl sha1 -binary | openssl base64`.chomp
	rootca_dnq = rootca_dnq.gsub( '/', '\/' )
	rootca_subject = '/O=CA.SAMSUNG.CO.KR/OU=CA.SAMSUNG.CO.KR/CN=.SERVERS.PRODUCTS.CA.SAMSUNG.CO.KR/dnQualifier=' + rootca_dnq

	`date "#{product_date}";hwclock -w`
	`openssl req -new -x509 -sha256 -config "#{rootca_cnf_file}" -days 3650 -set_serial "#{cert_serial}" -subj "#{rootca_subject}" -key "#{rootca_priv_file}" -outform PEM -out "#{rootca_cert_file}"`
	`sync`
end

# Make Intermediate Certificate
if !File.file?(inter_priv_file) || !File.file?(inter_cert_file) || !File.file?(inter_csr_file)
	puts "------------------------------------------------------------\n"
	puts "-                                                          -\n"
	puts "-  Make Intermediate Certificate                           -\n"
	puts "-                                                          -\n"
	puts "------------------------------------------------------------\n"

	`openssl genrsa -out "#{inter_priv_file}" "#{bit_of_key}"`
	`sync`

	inter_dnq = `openssl rsa -outform PEM -pubout -in "#{inter_priv_file}" | openssl base64 -d | dd bs=1 skip=24 2>/dev/null | openssl sha1 -binary | openssl base64`.chomp
	inter_dnq = inter_dnq.gsub( '/', '\/' )
	inter_subject = "/O=CA.SAMSUNG.CO.KR/OU=CA.SAMSUNG.CO.KR/CN=.DISPLAY.SERVERS.PRODUCTS.CA.SAMSUNG.CO.KR/dnQualifier=" + inter_dnq
	`openssl req -new -config "#{inter_cnf_file}" -days 3649 -subj "#{inter_subject}" -key "#{inter_priv_file}" -out "#{inter_csr_file}"`
	`sync`

	`date "#{product_date}";hwclock -w`
	`openssl x509 -req -sha256 -days 3649 -CA "#{rootca_cert_file}" -CAkey "#{rootca_priv_file}" -set_serial "#{cert_serial}" -in "#{inter_csr_file}" -extfile "#{inter_cnf_file}" -extensions v3_ca -out "#{inter_cert_file}"`
	`sync`
end

# Make Leaf Certificate
if !File.file?(leaf_priv_file) || !File.file?(leaf_cert_file) || !File.file?(leaf_csr_file)
	puts "------------------------------------------------------------\n"
	puts "-                                                          -\n"
	puts "-  Make Leaf Certificate                                   -\n"
	puts "-                                                          -\n"
	puts "------------------------------------------------------------\n"

	`openssl genrsa -out "#{leaf_priv_file}" "#{bit_of_key}"`
	`sync`

	leaf_dnq = `openssl rsa -outform PEM -pubout -in "#{leaf_priv_file}" | openssl base64 -d | dd bs=1 skip=24 2>/dev/null | openssl sha1 -binary | openssl base64`.chomp
	leaf_dnq = leaf_dnq.gsub( '/', '\/' )
	leaf_subject = "/O=CA.SAMSUNG.CO.KR/OU=CA.SAMSUNG.CO.KR/CN=PR SPB" + product_serial + product_name + ".LED.PRODUCT.CA.SAMSUNG.COM/dnQualifier=" + leaf_dnq
	`openssl req -new -config "#{leaf_cnf_file}" -days 3648 -subj "#{leaf_subject}" -key "#{leaf_priv_file}" -outform PEM -out "#{leaf_csr_file}"`
	`sync`

	`date "#{product_date}";hwclock -w`
	`openssl x509 -req -sha256 -days 3648 -CA "#{inter_cert_file}" -CAkey "#{inter_priv_file}" -set_serial "#{cert_serial}" -in "#{leaf_csr_file}" -extfile "#{leaf_cnf_file}" -extensions v3_ca -out "#{leaf_cert_file}"`
	`sync`
end


#
# Verify Certificate
#
puts "------------------------------------------------------------\n"
puts "-                                                          -\n"
puts "-  Vertify Certificate                                     -\n"
puts "-                                                          -\n"
puts "------------------------------------------------------------\n"

# Force Time Setting
`date "#{product_date}";hwclock -w`

# Reomve Certificate Chain
if File.file?(cert_chain_file)
	File.delete(cert_chain_file)
	`sync`
end

# Show Product Information
puts "Product Information"
puts "> name   : " + (product_name == '' ? "none" : product_name[1..-1])
puts "> serial : " + (product_serial == '' ? "none" : product_serial[1..-1])
puts ""

# Show Certificate Information
[ ["Self-singed RootCA Certificate", rootca_cert_file], ["Intermediate Certificate", inter_cert_file], ["Leaf Certificate", leaf_cert_file] ].each do |t|
	puts "#{t.first} ( #{t.last} ) :\n"
	puts "> #{`openssl x509 -noout -subject -in #{t.last}`}"
	puts "signed by\n"
	puts "> #{`openssl x509 -noout -issuer -in #{t.last}`}\n"
end

# Verify Certificate
puts "> #{`openssl verify -CAfile #{rootca_cert_file} #{rootca_cert_file}`}"
`cat "#{rootca_cert_file}" >> "#{cert_chain_file}"`

puts "> #{`openssl verify -CAfile "#{cert_chain_file}" "#{inter_cert_file}"`}"
`cat "#{inter_cert_file}" >> "#{cert_chain_file}"`

puts "> #{`openssl verify -CAfile "#{cert_chain_file}" "#{leaf_cert_file}"`}"
`cat "#{leaf_cert_file}" >> "#{cert_chain_file}"`

`sync`
puts "\nDONE."