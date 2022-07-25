@ECHO off

REM Note on input parameters
REM Param1: Binary file (with path)

SET "binname=%~n1"
SET "outputname=out"
SET "binpath=%~d1%~p1"
SET "utilpath=%~d0%~p0"
SET "input_bin="%binpath%\%binname%.bin""

SET "out_padbin="%binpath%\%outputname%_with_pad.bin""
SET "out_hashbin="%binpath%\%outputname%_with_hash.bin""

SET TMP_DIR=.\tmp
SET "tmp_hashfile=%TMP_DIR%\%outputname%.sha"

SET DIR_OPENSSL=%utilpath%\ExtTools
SET DIR_TOOLS=%utilpath%\ExtTools
SET OPENSSL=%DIR_OPENSSL%\openssl.exe 
SET SREC=%DIR_TOOLS%\srec_cat.exe
SET HEXDUMP=%DIR_TOOLS%\hexdump.exe

IF NOT EXIST %input_bin% (
ECHO Binary file %input_bin% does not exist!
GOTO eof
)

pushd %utilpath%

IF EXIST %TMP_DIR% DEL /Q %TMP_DIR%\*
IF NOT EXIST %TMP_DIR% mkdir %TMP_DIR%

IF EXIST %out_hashbin% DEL /Q %out_hashbin%
IF EXIST %out_padbin% DEL /Q %out_padbin%

SET padding=0x00
SET fsize=

ECHO binary file to be handled: %input_bin%

FOR %%A IN (%input_bin%) DO SET fsize=%%~zA
	
	ECHO ########## Calculate padding size ############ 
	SET /A pagesize=2048
	SET /A padsize = %pagesize% - %fsize% %% %pagesize% + %fsize%
		ECHO "filesize is %fsize%" 
		ECHO "page size is %pagesize%"
		ECHO "Total size with paddding is %padsize%" 
	
	ECHO ########## Add padding to binary file: ############
	ECHO size %padsize%, padding byte [%padding%]
	
	%SREC% %input_bin% -binary -fill %padding% 0x0000 %padsize% -o %out_padbin% -binary
	
	ECHO ########## Generate hash ############  
	ECHO %OPENSSL% dgst -sha256 -binary %out_padbin% > %tmp_hashfile%
	%OPENSSL% dgst -sha256 -binary %out_padbin% > %tmp_hashfile%

		ECHO FW HASH Value
		%HEXDUMP% %tmp_hashfile%

	ECHO ########## Generate bin + padding + hash ############  
	COPY /B %out_padbin% + %tmp_hashfile%  %out_hashbin%  >nul
	
	ECHO Binary with HASH to be flashed: 
	ECHO %out_hashbin%

:eof
IF EXIST %TMP_DIR% DEL /Q %TMP_DIR%\*
endlocal
pause



