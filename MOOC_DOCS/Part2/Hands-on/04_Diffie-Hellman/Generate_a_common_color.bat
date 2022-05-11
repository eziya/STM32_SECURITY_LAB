@echo off
del %1
echo *************************************
 ..\Tools\openssl genpkey -genparam -algorithm DH -out %1
echo Common date generated
echo *************************************
echo Result :
..\Tools\openssl pkeyparam -in %1


