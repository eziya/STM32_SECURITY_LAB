@echo off
echo *************************************
 ..\Tools\openssl genpkey -paramfile %2 -out Private_%1
echo Private color choosen.
..\Tools\openssl pkey -in Private_%1 -pubout -out Melting_%1
echo Melting color done 
echo *************************************
echo Result :
echo Private color choosen   : Private_%1
echo Melting color generated : Melting_%1


