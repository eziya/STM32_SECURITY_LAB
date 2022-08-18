# file location
$BIN_ORIGIN = "..\Debug\STM32_HAL_APP_AUTHENTICATION.bin"
$BIN_PAD = "..\Debug\STM32_HAL_APP_AUTHENTICATION_PAD.bin"
$BIN_PAD_HASH_SIG_PUBK = '..\Debug\STM32_HAL_APP_AUTHENTICATION_PAD_HASH_SIG_PUBK.bin'

$TEMP_HASH = "..\Debug\TempHash.sha"
$TEMP_PUBK = "..\Debug\TempPubK.pub"
$TEMP_PUBKTXT = "..\Debug\TempPubK.txt"
$TEMP_SIG = "..\Debug\TempSig.sig"
$TEMP_SIGTXT = "..\Debug\RawSig.txt"

$ECC_PRIK = ".\ecc.key"
$ECC_PUBK = ".\ecc_pub.key"

$OPENSSL = ".\openssl.exe"
$SREC = ".\srec_cat.exe"
$HEXDUMP =".\hexdump.exe"
$CERTUTL = "certutil"

# padding related variables
$padding = 0x00
$pageSize = 2048
$binSize = (Get-Item $BIN_ORIGIN).Length
$padBinSize = $pageSize - ($binSize % $pageSize) + $binSize


# STEP1. Padding
Write-Host "`r`nSTEP1. Padding..."
Write-Host "Padding Format:$padding"
Write-Host "Page Size:$pageSize"
Write-Host "Bin File Size:$binSize"
Write-Host "Padded File Size:$padBinSize"

Write-Host "$SREC $BIN_ORIGIN -binary -fill $padding 0x0000 $padBinSize -o $BIN_PAD -binary"
Invoke-Expression "$SREC $BIN_ORIGIN -binary -fill $padding 0x0000 $padBinSize -o $BIN_PAD -binary"

# STEP2. Hasing
Write-Host "`r`nSTEP2. Hashing..."

Write-Host "$OPENSSL dgst -sha256 -binary -out $TEMP_HASH $BIN_PAD"
Invoke-Expression "$OPENSSL dgst -sha256 -binary -out $TEMP_HASH $BIN_PAD"
Invoke-Expression "$HEXDUMP $TEMP_HASH"

# STEP3. Signing
Write-Host "`r`nSTEP3. Signing..."

Write-Host "$OPENSSL pkeyutl -sign -pkeyopt digest:sha256 -inkey $ECC_PRIK -in $TEMP_HASH -out $TEMP_SIG"
Invoke-Expression "$OPENSSL pkeyutl -sign -pkeyopt digest:sha256 -inkey $ECC_PRIK -in $TEMP_HASH -out $TEMP_SIG"

#STEP4. Convert signature fomat(ASN1 to raw binary)
Write-Host "`r`nSTEP4. Converting signature format(ASN1 to raw binary)"

Write-Host "$OPENSSL asn1parse -in $TEMP_SIG -inform=DER"
Invoke-Expression "$OPENSSL asn1parse -in $TEMP_SIG -inform=DER"

$prime1Str = (Invoke-Expression "$OPENSSL asn1parse -in $TEMP_SIG -inform=DER | Select -Index 1")
$prime2Str = (Invoke-Expression "$OPENSSL asn1parse -in $TEMP_SIG -inform=DER | Select -Index 2")

$prime1Idx = ($prime1Str | Select-String -Pattern ':' -AllMatches).Matches.Index[2]
$prime2Idx = ($prime2Str | Select-String -Pattern ':' -AllMatches).Matches.Index[2]

$prime1 = $prime1Str.Substring([int]$prime1Idx+1)
$prime2 = $prime2Str.Substring([int]$prime2Idx+1)

$prime1+$prime2 | Out-File -FilePath $TEMP_SIGTXT
Invoke-Expression "$CERTUTL -f -decodehex $TEMP_SIGTXT $TEMP_SIG"
Invoke-Expression "$HEXDUMP $TEMP_SIG"

#STEP5. Convert public key to binary
Write-Host "`r`nSTEP5. Converting public key to binary"

$ErrorActionPreference = 'SilentlyContinue'
Write-Host "$OPENSSL ec -pubin -in $ECC_PUBK -text -noout | Select -Index @(2,3,4,5,6)"

$pubKStr = Invoke-Expression "$OPENSSL ec -pubin -in $ECC_PUBK -text -noout | Select -Index @(2,3,4,5,6)"
$pubKstr
$ErrorActionPreference = 'Continue'

$pubKStr = $pubKStr -join ''
$pubKStr = $pubKStr -replace '\s', ''
$pubKStr = $pubKStr -replace ':', ''
$pubKStr = $pubKStr.Substring(2)
Write-Host $pubKStr

$pubKStr | Out-File -FilePath $TEMP_PUBKTXT
Invoke-Expression "$CERTUTL -f -decodehex $TEMP_PUBKTXT $TEMP_PUBK"
Invoke-Expression "$HEXDUMP $TEMP_PUBK"

#STEP6. Append hash,signature,public key to padded binary
Write-Host "`r`nSTEP6. Append hash,signature,public key to padded binary"
Write-Host "Get-Content $BIN_PAD, $TEMP_HASH, $TEMP_SIG, $TEMP_PUBK -Encoding Byte | Set-Content $BIN_PAD_HASH_SIG_PUBK -Encoding Byte"

Get-Content $BIN_PAD, $TEMP_HASH, $TEMP_SIG, $TEMP_PUBK -Encoding Byte | Set-Content $BIN_PAD_HASH_SIG_PUBK -Encoding Byte

Write-Host "BIN_ORIGIN length:" (Get-Item $BIN_ORIGIN).Length
Write-Host "BIN_PAD length:" (Get-Item $BIN_PAD).Length
Write-Host "TEMP_HASH length:" (Get-Item $TEMP_HASH).Length
Write-Host "TEMP_SIG length:" (Get-Item $TEMP_SIG).Length
Write-Host "TEMP_PUBK length:" (Get-Item $TEMP_PUBK).Length
Write-Host "BIN_PAD_HASH_SIG_PUBK length:" (Get-Item $BIN_PAD_HASH_SIG_PUBK).Length