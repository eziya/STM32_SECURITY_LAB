# file location
$BIN_BL = "..\Debug\STM32_HAL_SB_BL.bin"
$BIN_APP = "..\..\STM32_HAL_SB_APP\Debug\STM32_HAL_SB_APP.bin"
$BIN_ALL = "..\..\STM32_HAL_SB_APP\Debug\STM32_HAL_SB_ALL.bin"

# meta = (meta_info + app_hash + padding)
# meta_block = meta + meta_hash + meta_signature + padding

$TEMP_META_INFO_TXT = "..\Debug\MetaInfo.txt"
$TEMP_META_INFO = "..\Debug\MetaInfo.bin"
$TEMP_APP_HASH = "..\Debug\AppHash.bin"
$TEMP_META_INFO_APP_HASH = "..\Debug\MetaInfoAppHash.bin"
$TEMP_META = "..\Debug\Meta.bin"
$TEMP_META_HASH = "..\Debug\MetaHash.bin"
$TEMP_META_SIGTXT = "..\Debug\MetaSig.txt"
$TEMP_META_SIG = "..\Debug\MetaSig.bin"
$TEMP_META_HASH_SIG = "..\Debug\MetaHashSig.bin"
$TEMP_META_BLOCK = "..\Debug\MetaBlock.bin"

$ECC_PRIK = ".\ecc.key"
$ECC_PUBK = ".\ecc_pub.key"

$OPENSSL = ".\openssl.exe"
$SREC = ".\srec_cat.exe" 
$HEXDUMP = ".\hexdump.exe"
$CERTUTL = "certutil"

$padding = 0x00
$BLSize = 0xA000
$MetaBlockSize = 0x200
$MagicStr = "0x08192A3C"
$VERSION = 1



# STEP0. Information
Write-Host "============================================================================================================================================="
Write-Host "Boot loader file: $BIN_BL"
Write-Host "Application file: $BIN_APP"
Write-Host "Application version: $VERSION"



# STEP1. Generate meta_info: Magic Number(4bytes) + FW Size(4bytes) + Version(4bytes)
Write-Host "`r`nSTEP1. Generate meta info: Magic Number(4bytes) + FW Size(4bytes) + Version(4bytes)"

# Convert magic number to big-endian
$MagicArr = [byte[]]($MagicStr -replace '^0x' -split '(..)' -ne '' -replace '^','0x')
[Array]::Reverse($MagicArr)
$MagicStr = [BitConverter]::ToString($MagicArr)
$MagicStr = $MagicStr -replace '-', ''

# Convert application size to big-endian
$AppSize = (Get-Item $BIN_APP).Length
$AppSizeStr = $AppSize.ToString("X8")
$AppSizeArr = [byte[]]($AppSizeStr -replace '^0x' -split '(..)' -ne '' -replace '^','0x')
[Array]::Reverse($AppSizeArr)
$AppSizeStr = [BitConverter]::ToString($AppSizeArr)
$AppSizeStr = $AppSizeStr -replace '-', ''

# Convert version to big-endian
$VersionStr = $VERSION.ToString("X8")
$VersionArr = [byte[]]($VersionStr -replace '^0x' -split '(..)' -ne '' -replace '^','0x')
[Array]::Reverse($VersionArr)
$VersionStr = [BitConverter]::ToString($VersionArr)
$VersionStr = $VersionStr -replace '-', ''

# merge magic number, application size and version into meta_info
$MetaInfoStr = $MagicStr + $AppSizeStr + $VersionStr
Write-Host $MetaInfoStr

# Convert meta_info text to binary
$MetaInfoStr | Out-File -FilePath $TEMP_META_INFO_TXT
Invoke-Expression "$CERTUTL -f -decodehex $TEMP_META_INFO_TXT $TEMP_META_INFO"
Invoke-Expression "$HEXDUMP $TEMP_META_INFO"



# STEP2. Generate app_hash
Write-Host "`r`nSTEP2. Generate application hash digest"

# generate app_hash
Write-Host "$OPENSSL dgst -sha256 -binary -out $TEMP_APP_HASH $BIN_APP"
Invoke-Expression "$OPENSSL dgst -sha256 -binary -out $TEMP_APP_HASH $BIN_APP"
Invoke-Expression "$HEXDUMP $TEMP_APP_HASH"



# STEP3. Merge meta_info & app_hash binary
Write-Host "`r`nSTEP3. Merge meta_info & app_hash binary..."

# merge two binaries into meta_info_app_hash
Write-Host "Get-Content $TEMP_META_INFO, $TEMP_APP_HASH -Encoding Byte | Set-Content $TEMP_META_INFO_APP_HASH -Encoding Byte"
Get-Content $TEMP_META_INFO, $TEMP_APP_HASH -Encoding Byte | Set-Content $TEMP_META_INFO_APP_HASH -Encoding Byte

# add padding bytes to meta_info_app_hash to make meta (128 bytes)
Write-Host "$SREC $TEMP_META_INFO_APP_HASH -binary -fill $padding 0x0000 128 -o $TEMP_META -binary"
Invoke-Expression "$SREC $TEMP_META_INFO_APP_HASH -binary -fill $padding 0x0000 128 -o $TEMP_META -binary"
Invoke-Expression "$HEXDUMP $TEMP_META"



# STEP4. Hash & Signing meta
Write-Host "`r`nSTEP4. hash & signing meta."

# generate meta_hash
Write-Host "$OPENSSL dgst -sha256 -binary -out $TEMP_META_HASH $TEMP_META"
Invoke-Expression "$OPENSSL dgst -sha256 -binary -out $TEMP_META_HASH $TEMP_META"
Invoke-Expression "$HEXDUMP $TEMP_META_HASH"

# generate meta_sig
Write-Host "$OPENSSL pkeyutl -sign -pkeyopt digest:sha256 -inkey $ECC_PRIK -in $TEMP_META_HASH -out $TEMP_META_SIG"
Invoke-Expression "$OPENSSL pkeyutl -sign -pkeyopt digest:sha256 -inkey $ECC_PRIK -in $TEMP_META_HASH -out $TEMP_META_SIG"
Invoke-Expression "$HEXDUMP $TEMP_META_SIG"



# STEP4. Convert signature fomat(ASN1 to raw binary)
Write-Host "`r`nSTEP4. Converting signature format(ASN1 to raw binary)"

# convert meta_sig from ASN1 to raw binary
Write-Host "$OPENSSL asn1parse -in $TEMP_META_SIG -inform=DER"
Invoke-Expression "$OPENSSL asn1parse -in $TEMP_META_SIG -inform=DER"

$prime1Str = (Invoke-Expression "$OPENSSL asn1parse -in $TEMP_META_SIG -inform=DER | Select -Index 1")
$prime2Str = (Invoke-Expression "$OPENSSL asn1parse -in $TEMP_META_SIG -inform=DER | Select -Index 2")

$prime1Idx = ($prime1Str | Select-String -Pattern ':' -AllMatches).Matches.Index[2]
$prime2Idx = ($prime2Str | Select-String -Pattern ':' -AllMatches).Matches.Index[2]

$prime1 = $prime1Str.Substring([int]$prime1Idx+1)
$prime2 = $prime2Str.Substring([int]$prime2Idx+1)

$prime1+$prime2 | Out-File -FilePath $TEMP_META_SIGTXT
Invoke-Expression "$CERTUTL -f -decodehex $TEMP_META_SIGTXT $TEMP_META_SIG"
Invoke-Expression "$HEXDUMP $TEMP_META_SIG"



# STEP6. Merge meta, meta_hash, meta_sig, padding to make meta_block
Write-Host "`r`nSTEP6. Merge meta, meta_hash, meta_sig, padding to make meta_block"

# generate meta_block by adding meta, meta_hash, meta_sig and padding bytes
Write-Host "Get-Content $TEMP_META, $TEMP_META_HASH, $TEMP_META_SIG -Encoding Byte | Set-Content $TEMP_META_BLOCK -Encoding Byte"
Get-Content $TEMP_META, $TEMP_META_HASH, $TEMP_META_SIG -Encoding Byte | Set-Content $TEMP_META_BLOCK -Encoding Byte

Write-Host "$SREC $TEMP_META_BLOCK -binary -fill $padding 0x0000 $MetaBlockSize -o $TEMP_META_BLOCK -binary"
Invoke-Expression "$SREC $TEMP_META_BLOCK -binary -fill $padding 0x0000 $MetaBlockSize -o $TEMP_META_BLOCK -binary"
Invoke-Expression "$HEXDUMP $TEMP_META_BLOCK"



Write-Host "`r`nSTEP7. Append padding bytes to BL"
Write-Host "$SREC $BIN_BL -binary -fill $padding 0x0000 $BLSize -o $BIN_BL -binary"
Invoke-Expression "$SREC $BIN_BL -binary -fill $padding 0x0000 $BLSize -o $BIN_BL -binary"



Write-Host "`r`nSTEP8. Merge Bootloader, Meta Block and Applitcation"
Write-Host "Get-Content $BIN_BL, $TEMP_META_BLOCK, $BIN_APP -Encoding Byte | Set-Content $BIN_ALL -Encoding Byte"
Invoke-Expression "Get-Content $BIN_BL, $TEMP_META_BLOCK, $BIN_APP -Encoding Byte | Set-Content $BIN_ALL -Encoding Byte"
#Invoke-Expression "$HEXDUMP $BIN_ALL"


Write-Host "Done..."
Write-Host "============================================================================================================================================="

