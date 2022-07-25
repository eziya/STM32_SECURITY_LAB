@ECHO off

REM Note on input parameters
REM Param1: Bootloader Binary file (with path)
REM Param2: App Binary file (with path)

SET "bl_binname=%~n1"
SET "bl_binpath=%~d1%~p1"
SET "app_binname=%~n2"
SET "app_binpath=%~d2%~p2"
SET "utilpath=%~d0%~p0"
SET "outputname=out"

SET "bl_bin="%bl_binpath%\%bl_binname%.bin""
SET "app_bin="%app_binpath%\%app_binname%.bin""

SET "out_appbin="%app_binpath%\%outputname%_app_meta.bin""
SET "out_allbin="%app_binpath%\%outputname%_full.bin""

SET TMP_DIR=.\tmp

SET "tmp_paddfile=%TMP_DIR%\%outputname%.pad"
SET "tmp_hashfile=%TMP_DIR%\%outputname%.sha"
SET "tmp_metahashfile=%TMP_DIR%\%outputname%_meta.sha"
SET "tmp_metasigfile=%TMP_DIR%\%outputname%_meta.sig"
SET "tmp_metafile=%TMP_DIR%\%outputname%.meta"
SET "tmp_metabin="%TMP_DIR%\%outputname%_meta.bin""
SET "tmp_meta_to_sign=%TMP_DIR%\fw_meta_to_sign.bin"
SET "tmp_magic_size_ver=%TMP_DIR%\fw_magic_size_ver.bin"


SET DIR_OPENSSL=%utilpath%\ExtTools
SET DIR_TOOLS=%utilpath%\ExtTools

SET OPENSSL=%DIR_OPENSSL%\openssl.exe 
SET XXD=%DIR_TOOLS%\xxd.exe 
SET SED=%DIR_TOOLS%\sed.exe
SET WC=%DIR_TOOLS%\wc.exe
SET AWK=%DIR_TOOLS%\awk.exe
SET SREC=%DIR_TOOLS%\srec_cat.exe
SET HEXDUMP=%DIR_TOOLS%\hexdump.exe

SET "keyfile=%utilpath%\ecc.key"

IF NOT EXIST %keyfile% (
ECHO Key file %keyfile% does not exist! 
ECHO Please run openssl tool to generate the key file
ECHO e.g. %OPENSSL% ecparam -name prime256v1 -genkey -out %keyfile%
GOTO eof
)

IF NOT EXIST %bl_bin% (
ECHO Bootloader binary file %bl_bin% does not exist!
GOTO eof
)

IF NOT EXIST %app_bin% (
ECHO Application binary file %app_bin% does not exist!
GOTO eof
)

pushd %utilpath%

IF EXIST %TMP_DIR% DEL /Q %TMP_DIR%\*
IF NOT EXIST %TMP_DIR% mkdir %TMP_DIR%

SET padding=0x00
SET fsize=

:: Get the version of the fw and save the data to temp version file

SET /p Version=Version in decimal format (For instance 1): 

ECHO Version = %Version%

ECHO Bootloader binary file to be processed: %bl_bin%
ECHO App binary file to be processed: %app_bin%


REM Process app header 
ECHO ########## Generate hash of app bin ############  
	%OPENSSL% dgst -sha256 -binary %app_bin% > %tmp_hashfile%
	%HEXDUMP% %tmp_hashfile%
	
ECHO ########## Generate app meta ############  
	REM MAGIC (4 bytes) + FW SIZE (4 bytes) + version (4 bytes) 
	
	REM MAGIC
	SET "fw_magic=46574d41"
	ECHO |SET /p=">temp.txt ECHO(%fw_magic%" > cmd.bat
	
	REM FW size
	FOR %%A IN (%app_bin%) DO SET fsize=%%~zA
	SET /A fwsize = %fsize%
	
	CSCRIPT //nologo hex.vbs %fwsize% | %WC% -c > hex_len.txt
	FOR /F "tokens=*" %%a in ('TYPE hex_len.txt') do SET /A HEX_LEN=%%a-2
	 SET /A zerosize = 8-%HEX_LEN%
	IF %zerosize% NEQ 0 (
		REM add 0 before the 	
		FOR /L %%B IN (%zerosize%, -1, 1) DO ECHO |SET /p="0" >> cmd.bat
	)
	CSCRIPT //nologo hex.vbs %fwsize% >> cmd.bat	
	
	REM FW version	
	ECHO |SET /p=">>temp.txt ECHO(" >> cmd.bat
	CSCRIPT //nologo hex.vbs %Version% | %WC% -c > hex_len.txt
	FOR /F "tokens=*" %%a in ('TYPE hex_len.txt') do SET /A HEX_LEN=%%a-2
	 SET /A zerosize = 8-%HEX_LEN%
	IF %zerosize% NEQ 0 (
		FOR /L %%B IN (%zerosize%, -1, 1) DO ECHO |SET /p="0" >> cmd.bat
	)
	CSCRIPT //nologo hex.vbs %Version% >> cmd.bat
	
	REM revert the data order to little endian
	CALL cmd.bat
	DEL cmd.bat
	CERTUTIL -f -decodehex temp.txt %tmp_magic_size_ver% >nul	
	DEL temp.txt
	
	FOR /F "tokens=*" %%d in ('%XXD% -e %tmp_magic_size_ver% ^| %AWK% "{print $2 $3 $4}"') DO  (
		ECHO |SET /p=">>temp.txt ECHO(" >> cmd.bat
		ECHO %%d >> cmd.bat
	)
	
	CALL cmd.bat	
	CERTUTIL -f -decodehex temp.txt %tmp_magic_size_ver% >nul	
		ECHO Magic + Size + Version
		%HEXDUMP% %tmp_magic_size_ver%
	
	REM Append FW hash
	COPY /B %tmp_magic_size_ver% + %tmp_hashfile% 	%tmp_meta_to_sign% > nul
	
	REM Pad meta binary to 128 Bytes (to be signed)
	%SREC% %tmp_meta_to_sign% -binary -fill %padding% 0x0000 0x80 -o %tmp_meta_to_sign% -binary
		
		ECHO FW meta data to be signed
		%HEXDUMP% %tmp_meta_to_sign%
	
	IF EXIST cmd.bat DEL cmd.bat
	IF EXIST hex_len.txt DEL hex_len.txt	
	IF EXIST temp.txt DEL temp.txt
	
	REM sign the meta data 
	ECHO ########## Sign hash biary ############  
	ECHO " First generate the hash "
	%OPENSSL% dgst -sha256 -binary %tmp_meta_to_sign% > %tmp_metahashfile%
		ECHO FW meta data hash
		%HEXDUMP% %tmp_metahashfile%
	ECHO " Then generate the signature "
	%OPENSSL% dgst -sha256 -sign %keyfile% -out %tmp_metasigfile% %tmp_meta_to_sign%
		ECHO FW meta data signature
		%HEXDUMP% %tmp_metasigfile%
	
	ECHO ########## Convert signature from asn1 to raw binary ############  
	REM convert the ASN1 encoded signature to raw signature binary
	IF EXIST temp.txt DEL /Q temp.txt
	FOR /F "tokens=4 delims=:" %%A IN ('%OPENSSL% asn1parse -in %tmp_metasigfile% -inform=DER') DO >>temp.txt ECHO(%%A
	TYPE temp.txt
	CERTUTIL -f -decodehex temp.txt %tmp_metasigfile% >nul
	IF EXIST temp.txt DEL /Q temp.txt
	
		ECHO FW meta data signature converted
		%HEXDUMP% %tmp_metasigfile%
	
	REM combine the meta data and signature 
	ECHO ########## Build header data ############  
	COPY /B %tmp_meta_to_sign% + %tmp_metahashfile% + %tmp_metasigfile%  %tmp_metabin%  > nul
	
	REM Add stuffing byptes to make the header file to 512 bytes
	%SREC% %tmp_metabin% -binary -fill %padding% 0x0000 512 -o %tmp_metabin% -binary
		ECHO FW Header FULL (512B with stuffing bytes)
		%HEXDUMP% %tmp_metabin%
	
	ECHO ########## Build Header and app binary ############  
	COPY /B %tmp_metabin% + %app_bin% %out_appbin% > nul
		
REM Process bootloader binary to add padding
ECHO ########## Calculate BL padding size ############ 
	SET BL_SIZE_LIMIT=32768
		
ECHO ########## Write BL padding file: ############
	ECHO BL padding to size %BL_SIZE_LIMIT%, BL padding byte [%padding%]
	%SREC% %bl_bin% -binary -fill %padding% 0x0000 %BL_SIZE_LIMIT% -o %tmp_paddfile% -binary
	
ECHO ########## Build full binary with bootloader and app ############  
	COPY /B %tmp_paddfile% + %out_appbin% %out_allbin% > nul
				
	ECHO Application binary with MetaData(header) file:	
	ECHO %out_appbin%
	ECHO Single binary with Bootloader and Application to be flashed: 
	ECHO %out_allbin%
	
:eof
IF EXIST %TMP_DIR% DEL /Q %TMP_DIR%\*
endlocal
pause
	


