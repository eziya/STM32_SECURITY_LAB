/**
  ******************************************************************************
  * @file    Commonappli.h
  * @author  MCD Application Team
  * @brief   Global configuration file for the application
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef COMMON_APPLI_H
#define COMMON_APPLI_H

/* Defines to fine tune Application behaviour */

#define MAX_SIZE_BUFFER_I2_FOR_DATAS 496
#define ERASE_CERTIFICATE_BEFORE_UPDATE   /* Allow to activate erase certificate before update */
#define READ_CERTIFICATE_AND_VERIFY /* Allow to verify certificate read correspond to the one expected */
#define MAX_SIZE_ZONE_STSAFE_A 1696 /* This is maximum size of zone for STSAFE-A110 personalized with
                                       SPL2 sample */

/* Allow to activate more logs displayed on screen inside secure API library */
/*#define PRINTF_ON*/

#define FORCE_PAIRING  /* Allow to force pairing of STSAFE even if already done before, */
/* This allow to write in flash pairing keys even if STM32 flash is erased before */
#define NOK 1
#define OK 0

/* functions definition */

#define STS_CHK(ret, f) if (!ret) ret = f;

#endif /* COMMON_APPLI_H */
