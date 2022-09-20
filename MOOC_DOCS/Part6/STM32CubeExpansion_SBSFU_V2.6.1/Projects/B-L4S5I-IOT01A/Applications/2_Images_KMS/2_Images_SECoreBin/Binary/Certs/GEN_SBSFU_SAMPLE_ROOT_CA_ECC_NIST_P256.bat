set OPENSSL=C:\local\Tools\OpenSSL-Win64\bin\openssl.exe
set CERT_FULLNAME=Root CA
set WORK_FOLDER=SBSFU_SAMPLE_ROOT_CA
set work_radix_name=sbsfu_r0
set TEMPLATE_FOLDER=Template_%WORK_FOLDER%

xcopy /E /I /Y %TEMPLATE_FOLDER% %WORK_FOLDER%

echo OFF
echo GENERATING SBSFU Sample %CERT_FULLNAME% Keys and Certs
echo ==============================================
echo  

echo Generating Private Key ...
%OPENSSL% ecparam -name prime256v1 -genkey -noout -out %WORK_FOLDER%\private\%work_radix_name%_prv.pem
echo  

echo Generating Public Key ...
%OPENSSL% ec -in %WORK_FOLDER%\private\%work_radix_name%_prv.pem -pubout -out %WORK_FOLDER%\certs\%work_radix_name%_pub.pem
echo  

echo Generate SBSFU Sample %CERT_FULLNAME% Certificate (self signed)
%OPENSSL% req -config %WORK_FOLDER%\openssl.cnf -key %WORK_FOLDER%\private\%work_radix_name%_prv.pem -new -x509 -days 1000 -sha256 -extensions v3_ca -out %WORK_FOLDER%\certs\%work_radix_name%_crt.pem
echo  

echo Converting SBSFU Sample %CERT_FULLNAME% certificate to der format
%OPENSSL% x509 -outform der -in %WORK_FOLDER%\certs\%work_radix_name%_crt.pem -out %WORK_FOLDER%\certs\%work_radix_name%_crt.der
echo  

echo SBSFU Sample %CERT_FULLNAME% Certificate generated:

%OPENSSL% x509 -in %WORK_FOLDER%\certs\%work_radix_name%_crt.pem -noout -text

copy %WORK_FOLDER%\certs\%work_radix_name%_crt.pem .
copy %WORK_FOLDER%\certs\%work_radix_name%_crt.der .


echo Done