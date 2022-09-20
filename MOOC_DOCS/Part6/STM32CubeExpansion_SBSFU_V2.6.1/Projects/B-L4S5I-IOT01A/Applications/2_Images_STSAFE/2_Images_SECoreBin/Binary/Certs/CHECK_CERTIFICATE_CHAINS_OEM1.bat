echo "Check OEM1 chain..."
C:\local\Tools\OpenSSL-Win64\bin\openssl verify -verbose -show_chain -trusted sbsfu_r0_crt.pem -untrusted sbsfu_inter1_oem1_crt.pem -untrusted sbsfu_inter2_oem1_crt.pem sbsfu_fwsign_oem1_crt.pem
pause