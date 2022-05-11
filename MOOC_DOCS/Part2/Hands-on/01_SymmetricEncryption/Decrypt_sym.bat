@echo off
if "%3" == "" goto args_count_wrong
if "%4" == "" goto args_count_ok

:args_count_wrong
echo Syntax error.
echo 3 arguments are required : key_value file_in file_out 
exit /b 1

:args_count_ok
echo *************************************
echo Input message in ascii  :
type %2
echo.
echo -------------------------------------
echo Input message hexdecimal:
..\Tools\hexdump %2
echo *************************************
 ..\Tools\openssl enc -d -aes-128-ecb -k %1 -nosalt  -in %2  -out %3 
echo Decryption done...
echo *************************************
echo Result in ascii:
type %3
echo.
echo -------------------------------------
echo Result in hexdecimal:
..\Tools\hexdump %3 


