SBSFU functionalities are supported on STM32L5 series, and delivered in STM32CubeL5 package.
Please, go to https://www.st.com/en/embedded-software/stm32cubel5.html to download latest 
version of the package 

Refer to AN5447 "X-CUBE-SBSFU vs TF-M overview"

The SBSFU provides a Root of Trust solution including Secure Boot and Secure
Firmware Update functionalities that is used before executing the application
and provides an example of secure services that are isolated from the
non-secure application but can be used by the non-secure application at run-time.

The Secure Boot and Secure Firmware Update(SBSFU_Boot) is based on the Open Source
TF-M reference implementation that has been ported on STM32 microcontroller to take 
benefit of STM32 HW security features such as CM33 Trust Zone/MPU, Trust Zone
aware peripherals, Memory protections (HDP, WRP) and enhanced life cycle scheme.
it allows the update of the STM32 microcontroller built-in program with new firmware 
versions, adding new features and correcting potential issues. The update process is
performed in a secure way to prevent unauthorized updates and access to confidential
on-device data:

-	The Secure Boot (Root of Trust services) is an immutable code, always
    executed after a system reset, that checks STM32 static protections,
    activates STM32 runtime protections and then verifies the authenticity
    (RSA 3072 signature) and integrity (SHA256) of the application code
    before every execution in order to ensure that invalid or malicious
    code cannot be run. The default RSA key is taken from 
    Middlewares/Third_Party/trustedfirmware/bl2/ext/mcuboot/keys.c 
    and is embedded in the provisionned data area in the secure boot and secure
    firmware update binary.

-	The Secure Firmware Update application is an immutable code that detects
    that a new firmware image is available, that checks its authenticity, and
    that checks the integrity of the code before installing it. The firmware
    update can be done either on the secure part of firmware image or/and on
    the non-secure part of the firmware image.


For more details, refer to AN5447 "X-CUBE-SBSFU vs TF-M overview"
available from the STMicroelectronics microcontroller website www.st.com.

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
