@ECHO off

REM Note on input parameters
REM Param1: Binary file (with path)

SET "binname=%~n1"
SET "outputname=out"
SET "binpath=%~d1%~p1"
SET "utilpath=%~d0%~p0"

SET "input_bin="%binpath%\%binname%.bin""

SET DIR_OPENSSL=%utilpath%\ExtTools
SET DIR_TOOLS=%utilpath%\ExtTools

SET OPENSSL=%DIR_OPENSSL%\openssl.exe 
SET SREC=%DIR_TOOLS%\srec_cat.exe
SET AWK=%DIR_TOOLS%\awk.exe
SET GREP=%DIR_TOOLS%\grep.exe
SET SED=%DIR_TOOLS%\sed.exe
SET HEXDUMP=%DIR_TOOLS%\hexdump.exe
SET "keyfile=%utilpath%\ecc.key"

SET "out_padbin="%binpath%\%outputname%_with_pad.bin""
SET "out_hashbin="%binpath%\%outputname%_with_hash.bin""
SET "out_sigbin="%binpath%\%outputname%_with_sig.bin""
SET "out_sigkpubbin="%binpath%\%outputname%_with_sig_kpub.bin""

SET TMP_DIR=.\tmp
SET "tmp_hashfile=%TMP_DIR%\%outputname%.sha"
SET "tmp_keypub=%TMP_DIR%\%outputname%.pub"
SET "tmp_sigfile=%TMP_DIR%\%outputname%.sig"

IF NOT EXIST %keyfile% (
ECHO Key file %keyfile% does not exist! 
ECHO Please run openssl tool to generate the key file
ECHO e.g. %OPENSSL% ecparam -name prime256v1 -genkey -out %keyfile%
GOTO eof
)

IF NOT EXIST %input_bin% (
ECHO Binary file %input_bin% for signing does not exist!
GOTO eof
)

pushd %utilpath%

IF EXIST %TMP_DIR% DEL /Q %TMP_DIR%\*
IF NOT EXIST %TMP_DIR% mkdir %TMP_DIR%

IF EXIST %out_hashbin%  	DEL /Q %out_hashbin% 
IF EXIST %out_padbin%  		DEL /Q %out_padbin% 
IF EXIST %out_sigkpubbin% 	DEL /Q %out_sigkpubbin%
IF EXIST %out_sigbin% 		DEL /Q %out_sigbin%

SET padding=0x00
SET fsize=

ECHO binary file to be handled: %input_bin%

FOR %%A IN (%input_bin%) DO SET fsize=%%~zA
	
	ECHO ########## Calculate padding size ############ 
	SET /A pagesize=2048
	SET /A padsize = %pagesize% - %fsize% %% %pagesize% + %fsize%
		ECHO "filesize is %fsize%" 
		ECHO "page size is %pagesize%"
		ECHO "Size after padding is %padsize%" 
	
	%SREC% %input_bin% -binary -fill %padding% 0x0000 %padsize% -o %out_padbin% -binary
	
	ECHO ########## Generate hash ############  
	ECHO %OPENSSL% dgst -sha256 -binary %out_padbin% > %tmp_hashfile%
	%OPENSSL% dgst -sha256 -binary %out_padbin% > %tmp_hashfile%

		ECHO FW HASH
		%HEXDUMP% %tmp_hashfile%
		
	ECHO ########## Sign hash biary ############  
	%OPENSSL% dgst -sha256 -sign %keyfile% -out %tmp_sigfile% %out_padbin%
	REM %OPENSSL% pkeyutl -sign -in %tmphashfile% -inkey %keyfile% -pkeyopt digest:sha256 -out %tmp_sigfile%
	
	ECHO ########## Convert signature from asn1 to raw binary ############  
	REM convert the ASN1 encoded signature to raw signature binary
	IF EXIST temp.txt DEL temp.txt
	FOR /F "tokens=4 delims=:" %%A IN ('%OPENSSL% asn1parse -in %tmp_sigfile% -inform=DER') DO >>temp.txt ECHO(%%A
	CERTUTIL -f -decodehex temp.txt %tmp_sigfile% >nul
	IF EXIST temp.txt DEL temp.txt

		ECHO FW Signature
		%HEXDUMP% %tmp_sigfile%
	
	ECHO "################# Generate public key from private ecc key "
	%OPENSSL% pkey -in ecc.key -pubout -out ecc_pub.key 
	ECHO "################# Convert public key to binary "
	%OPENSSL% ec -pubin -in ecc_pub.key -text -noout |%GREP% ":"|%GREP% "    "|%SED% 's/    /-/g'|%SED% 's/:/,:/g'|%AWK% -F ":" "{ print $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15}"|%SED% 's/-04,//'|%SED% 's/  //g' >> temp.txt 
	CERTUTIL -f -decodehex temp.txt %tmp_keypub% >nul
	IF EXIST temp.txt DEL temp.txt

		ECHO Public Key
		%HEXDUMP% %tmp_keypub%
			
	ECHO ########## Generate bin + padding + hash ############  
	COPY /B %out_padbin% + %tmp_hashfile%  %out_hashbin%  >nul
	
	ECHO ########## Generate bin + padding + hash + sig ############  
	REM concatenate padded binary with hash and signature and output padded binary with hash and signature
    COPY /B %out_hashbin% +	%tmp_sigfile%  %out_sigbin%  >nul

	ECHO ########## Generate bin + padding + hash + sig + pubkey ############  
	REM concatenate padded binary with hash and signature and output padded binary with hash and signature
    ::type %out_hashbin% %tmp_sigfile%  > %out_sigbin%  2>nul
	COPY /B %out_hashbin% +	%tmp_sigfile% + %tmp_keypub% %out_sigkpubbin%  >nul

	ECHO Binary with Signature to be flashed: 
	ECHO %out_sigkpubbin%

:eof
IF EXIST %TMP_DIR% DEL /Q %TMP_DIR%\*
endlocal
pause
