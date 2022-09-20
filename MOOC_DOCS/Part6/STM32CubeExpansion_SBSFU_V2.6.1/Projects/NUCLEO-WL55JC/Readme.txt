SBSFU functionalities are supported on STM32WL series, and delivered in STM32CubeWL package.
Please, go to https://www.st.com/en/embedded-software/stm32cubewl.html to download latest 
version of the package.

Refer to UM2767 "Getting started with the SBSFU of STM32CubeWL".

This user manual describes how to get started with the STM32CubeWL SBSFU (Secure Boot and Secure Firmware Update).

In a dual-core configuration, the Secure Boot is made of two parts (one per core):
• Cortex®-M4 boot: The Secure Boot checks static protections, checks the Cortex®-M0+ boot configuration, activates the
  Cortex®-M4 runtime protections and boots the Cortex®-M0+.
• Cortex®-M0+ boot: The Secure Boot checks static protections, activates Cortex®-M0+ runtime protections, verifies the
  authenticity and integrity of user application code before every execution to make sure that invalid or malicious code cannot
  be run, and then signals to both cores that user applications are valid.

Regarding LoRaWAN® firmware update over-the-air with STM32CubeWL, more details are provided in application note
LoRaWAN® firmware update over the air with STM32CubeWL (AN5554).

Examples are provided for single-slot configuration to maximize firmware image size, and for dual-slot configuration to ensure
safe image installation and enable over-the-air firmware update capability commonly used in IoT devices.

For a complex system (such as protocol stack, middleware and user application), the firmware image configuration can be
extended up to three firmware images. In this application, one firmware image is used for the single-core configuration while
two firmware images are available for the dual-core configuration. Examples can be configured to use asymmetric or symmetric
cryptographic schemes with or without firmware encryption.

In the dual-core configuration, the secure Key Management Services (KMS) provide cryptographic services to the user
application through the PKCS#11 APIs (KEY ID-based APIs) that are executed inside a protected and isolated environment.
User application keys are stored in the protected and isolated environment for their secured update: authenticity check, data
decryption, and data integrity check.

In the single-core configuration, the same services are offered but they are not executed inside a protected and isolated
environment.