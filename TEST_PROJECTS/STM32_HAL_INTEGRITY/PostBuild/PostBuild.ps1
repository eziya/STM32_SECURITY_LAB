$bin_app   = '..\Debug\STM32_HAL_INTEGRITY.bin'
$bin_pad   = '..\Debug\STM32_HAL_INTEGRITY_PAD.bin'
$bin_hash  = '..\Debug\STM32_HAL_INTEGRITY_HASH.bin'
$bin_dgst = '..\Debug\dgst.bin'
$filesize  = (Get-Item $bin_app).length
$pagesize  = 2048
$padsize   = $pagesize - ($filesize % $pagesize) + $filesize
$SREC      = '.\srec_cat.exe'
$OPENSSL   = '.\openssl.exe'

#add pad bytes to fw
Invoke-Expression "$SREC $bin_app -binary -fill 0x00 0x0000 $padsize -o $bin_pad -binary"

#get binary hash digest
Invoke-Expression "$OPENSSL dgst -sha256 -binary -out $bin_dgst $bin_pad"

#append digest to fw
Get-Content $bin_pad, $bin_dgst -Encoding Byte | Set-Content $bin_hash -Encoding Byte
