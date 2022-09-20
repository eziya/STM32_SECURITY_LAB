/**
  ******************************************************************************
  * @file    sfu_fwimg_state.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for Secure Firmware Update image
  *          state handling.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SFU_FWIMG_STATE_H
#define SFU_FWIMG_STATE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Exported types ------------------------------------------------------------*/
#if defined(ENABLE_IMAGE_STATE_HANDLING)
#if   defined(SFU_MPU_PROTECT_ENABLE)
/**
  * Firmware Image State definitions : information saved in non-initialized RAM shared memory
  * ==> SB_FWIMG_STATE_REGION_RAM_START
  */
#define FWIMG_STATE_TAG_INVALID   0x0035049fU /* image is not valid (typically selftest has failed) and should not boot */
#define FWIMG_STATE_TAG_VALID     0x003574e4U /* image is valid (selftest passed), image OK to boot if signature checks pass */
#define FWIMG_STATE_TAG_VALID_ALL 0x003654aaU /* master image is valid (selftest passed), all other images are automatically validated */
#define FWIMG_STATE_TAG_SELFTEST  0x0036c702U /* self tests to be executed. Image OK to boot one time if signature checks pass */
#define FWIMG_STATE_TAG_NEW       0x00375ff5U /* new image, has never run on the device */

#define FWIMG_STATE          ((uint32_t *)SB_FWIMG_STATE_REGION_RAM_START)
#endif /* SFU_XXX_PROTECT_ENABLE */
#endif /* ENABLE_IMAGE_STATE_HANDLING */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SFU_FWIMG_STATE_H */

