# file location
$HEADER_FILE = "..\Core\Inc\ecc_pub_key.h"
$ECC_PARAM = ".\ecc_param.key"
$ECC_PRIK = ".\ecc.key"
$ECC_PUBK = ".\ecc_pub.key"
$OPENSSL = ".\openssl.exe"

#STEP1. Generate Public Key
Write-Host "`r`nSTEP1. ECC key generation..."

Write-Host "$OPENSSL ecparam -name prime256v1 -genkey -out $ECC_PARAM"
#Invoke-Expression "$OPENSSL ecparam -name prime256v1 -genkey -out $ECC_PARAM"

Write-Host "$OPENSSL ecparam -in $ECC_PARAM -genkey -out $ECC_PRIK"
#Invoke-Expression "$OPENSSL ecparam -in $ECC_PARAM -genkey -out $ECC_PRIK"

Write-Host "$OPENSSL pkey -in $ECC_PRIK -pubout -out $ECC_PUBK"
#Invoke-Expression "$OPENSSL pkey -in $ECC_PRIK -pubout -out $ECC_PUBK"

#STEP2. Read Pubic Key
Write-Host "`r`nSTEP2. Read Pubic Key"
Write-Host "$OPENSSL ec -pubin -in $ECC_PUBK -text -noout | Select -Index @(2,3,4,5,6)"
$ErrorActionPreference = 'SilentlyContinue'
$pubKStr = Invoke-Expression "$OPENSSL ec -pubin -in $ECC_PUBK -text -noout | Select -Index @(2,3,4,5,6)"
$pubKstr
$ErrorActionPreference = 'Continue'

#STEP3. Create Header File
Write-Host "`r`nSTEP3. Create Header File"

$pubKStr = $pubKStr -replace '\s', ''
$pubKStr = $pubKStr -replace ':', ',0x'

$pubKStrLine1 = $pubKStr | Select -Index 0
$pubKStrLine2 = "0x" + ($pubKStr | Select -Index 1)
$pubKStrLine3 = "0x" + ($pubKStr | Select -Index 2)
$pubKStrLine4 = "0x" + ($pubKStr | Select -Index 3)
$pubKStrLine5 = "0x" + ($pubKStr | Select -Index 4)

$pubKStrLine1 = $pubKStrLine1.Substring(3, $pubKStrLine1.Length-5 )
$pubKStrLine2 = $pubKStrLine2.Substring(0, $pubKStrLine2.Length-2 )
$pubKStrLine3 = $pubKStrLine3.Substring(0, $pubKStrLine3.Length-2 )
$pubKStrLine4 = $pubKStrLine4.Substring(0, $pubKStrLine4.Length-2 )

$HeaderStr =
"#ifndef __ECC_PUB_KEY_H_
#define __ECC_PUB_KEY_H_
static uint8_t SIGN_ECC_PUB_KEY[] = {
`t$pubKStrLine1
`t$pubKStrLine2
`t$pubKStrLine3
`t$pubKStrLine4
`t$pubKStrLine5
};
#endif /*__ECC_PUB_KEY_H_*/"

$HeaderStr = $HeaderStr -replace '`n', '`r`n'

Write-Host $HeaderStr
$HeaderStr | Out-File -Encoding utf8 -FilePath $HEADER_FILE
