$bin_app   = '..\Debug\STM32_HAL_APP_INTEGRITY.bin'
$bin_pad   = '..\Debug\STM32_HAL_APP_INTEGRITY_PAD.bin'
$bin_hash  = '..\Debug\STM32_HAL_APP_INTEGRITY_HASH.bin'
$bin_dgst = '..\Debug\dgst.bin'
$filesize  = (Get-Item $bin_app).length
$pagesize  = 2048
$padsize   = $pagesize - ($filesize % $pagesize) + $filesize
$SREC      = '.\srec_cat.exe'
$OPENSSL   = '.\openssl.exe'

#STEP1. adding pad bytes to FW
Write-Host "`r`nSTEP1. adding pad bytes to FW"

Write-Host "$SREC $bin_app -binary -fill 0x00 0x0000 $padsize -o $bin_pad -binary"
Invoke-Expression "$SREC $bin_app -binary -fill 0x00 0x0000 $padsize -o $bin_pad -binary"

#STEP2. getting binary hash digest
Write-Host "`r`nSTEP2. getting binary hash digest"

Write-Host "$OPENSSL dgst -sha256 -binary -out $bin_dgst $bin_pad"
Invoke-Expression "$OPENSSL dgst -sha256 -binary -out $bin_dgst $bin_pad"

#STEP3. appending digest to FW
Write-Host "`r`nSTEP3. appending digest to FW"

Write-Host "Get-Content $bin_pad, $bin_dgst -Encoding Byte | Set-Content $bin_hash -Encoding Byte"
Get-Content $bin_pad, $bin_dgst -Encoding Byte | Set-Content $bin_hash -Encoding Byte
