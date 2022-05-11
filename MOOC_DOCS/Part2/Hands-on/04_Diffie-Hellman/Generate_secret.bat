@echo off
echo *************************************
 ..\Tools\openssl pkeyutl -derive -inkey %1 -peerkey %2 -out %3
echo Secret generated
echo *************************************
echo Result :
echo %3
..\Tools\hexdump.exe %3


