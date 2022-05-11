@echo off
del %3
echo *************************************
echo Input message in ascii  :
type %2
echo.
echo -------------------------------------
echo Input message hexdecimal:
..\Tools\hexdump %2
echo *************************************
 ..\Tools\openssl rsautl -decrypt -inkey %1 -in %2 -out %3
echo Decryption done...
echo *************************************
echo Result in ascii:
type %3
echo.
echo -------------------------------------
echo Result in hexdecimal:
..\Tools\hexdump %3 


