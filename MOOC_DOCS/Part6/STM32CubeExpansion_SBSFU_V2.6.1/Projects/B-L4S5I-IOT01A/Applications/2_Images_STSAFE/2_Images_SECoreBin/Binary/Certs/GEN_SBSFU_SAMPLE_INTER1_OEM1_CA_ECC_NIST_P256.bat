set OPENSSL=C:\local\Tools\OpenSSL-Win64\bin\openssl.exe
set PARENT_CERT_FOLDER=SBSFU_SAMPLE_ROOT_CA
set CERT_FULLNAME=Intermediate 1 : OEM 1
set WORK_FOLDER=SBSFU_SAMPLE_INTER1_OEM1_CA
set work_radix_name=sbsfu_inter1_oem1
set TEMPLATE_FOLDER=Template_%WORK_FOLDER%

xcopy /E /I /Y %TEMPLATE_FOLDER% %WORK_FOLDER%

echo OFF
echo GENERATING SBSFU Sample %CERT_FULLNAME% CA Keys and Certificate
echo ==============================================================
echo -

echo Generating Private Key
echo ----------------------
%OPENSSL% ecparam -name prime256v1 -genkey -out %WORK_FOLDER%\private\%work_radix_name%_prv.pem
echo .

echo Generating Public Key
echo ---------------------
%OPENSSL% ec -in %WORK_FOLDER%\private\%work_radix_name%_prv.pem -pubout -out %WORK_FOLDER%\certs\%work_radix_name%_pub.pem
echo .

echo Generate Certificate Signing Request
echo ------------------------------------
%OPENSSL% req -new -config %WORK_FOLDER%\openssl.cnf -sha256 -key %WORK_FOLDER%\private\%work_radix_name%_prv.pem -out %WORK_FOLDER%\csr\%work_radix_name%_csr.pem
echo .

echo Now go to Root CA and generate the certificate
pause

echo Generate %CERT_FULLNAME% Certificate from CSR and Sign it
echo --------------------------------------------------------
%OPENSSL% ca -config %PARENT_CERT_FOLDER%\openssl.cnf -extensions v3_intermediate_ca -days 375 -notext -md sha256 -in %WORK_FOLDER%\csr\%work_radix_name%_csr.pem -out %WORK_FOLDER%\certs\%work_radix_name%_crt.pem
echo .

echo Converting %CERT_FULLNAME% certificate to der format
echo ---------------------------------------------------
%OPENSSL% x509 -outform der -in %WORK_FOLDER%\certs\%work_radix_name%_crt.pem -out %WORK_FOLDER%\certs\%work_radix_name%_crt.der
echo .

echo %CERT_FULLNAME% Certificate generated:

%OPENSSL% x509 -in %WORK_FOLDER%\certs\%work_radix_name%_crt.pem -noout -text

copy %WORK_FOLDER%\certs\%work_radix_name%_crt.pem .
copy %WORK_FOLDER%\certs\%work_radix_name%_crt.der .

echo Done creating %CERT_FULLNAME% keys and Certificates
pause