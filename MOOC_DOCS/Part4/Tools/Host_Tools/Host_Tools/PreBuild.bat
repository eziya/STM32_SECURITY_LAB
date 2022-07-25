@ECHO off

REM Note on input parameters
REM Param2: ecc private key file
REM Param2: Folder to put the generated header file

SET "keyfile=%1"
SET "srcpath=%2"
SET "hfile="%srcpath%\ecc_pub_key.h"
SET "utilpath=%~d0%~p0"

SET DIR_OPENSSL=%utilpath%\ExtTools
SET DIR_TOOLS=%utilpath%\ExtTools

SET OPENSSL=%DIR_OPENSSL%\openssl.exe 
SET GREP=%DIR_TOOLS%\grep.exe 
SET AWK=%DIR_TOOLS%\awk.exe 
SET SED=%DIR_TOOLS%\sed.exe 

pushd %utilpath%

ECHO "################# Generate public key from private ecc key "
%OPENSSL% pkey -in ecc.key -pubout -out ecc_pub.key 
::%OPENSSL% ec -pubin -in ecc_pub.key -text -noout

ECHO #ifndef __ECC_PUB_KEY_H_ > %hfile%
ECHO #define __ECC_PUB_KEY_H_ >> %hfile%
ECHO static uint8_t SIGN_ECC_PUB_KEY[] = { >> %hfile%
%OPENSSL% ec -pubin -in ecc_pub.key -text -noout |%GREP% ":"|%GREP% "    "|%SED% 's/    /-/g'|%SED% 's/:/,:/g'|%AWK% -F ":" "{ print $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15}"|%SED% 's/-04,//'|%SED% 's/  //g'| %SED% 's/ / 0x/g' |%SED% 's/-/ 0x/g' >> %hfile% 
ECHO }; >> %hfile%
ECHO #endif /*__ECC_PUB_KEY_H_*/ >> %hfile%

ECHO "################# Get public key hex data "
FOR /F "tokens=* delims=':'" %%A IN ('%OPENSSL% ec -pubin -in ecc_pub.key -text -noout ^|find /v "pub" ^|find /v "ASN1" ^|find /v "NIST" ^|find /v "Public"') DO ECHO "%%A "
::FOR /F "tokens=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 delims=:" %%A IN 	

REM Prepareimage.exe genpub -k %keyfile% %hfile%

popd
