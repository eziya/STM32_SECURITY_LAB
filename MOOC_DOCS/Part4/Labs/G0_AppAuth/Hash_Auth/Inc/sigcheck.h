/**
  ******************************************************************************
  * @file    securitycheck.h
  * @brief   Header file of security config check module.
  *          This file provides set of firmware functions to manage Com
  *          functionalities.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef __SIGCHECK_H
#define __SIGCHECK_H

#include "hashcheck.h"

/* Hash data is placed at the last page of the flash*/
#define SIG_ADD (HASH_ADD + HASH_SIZE)
#define SIG_SIZE (64)
#define ECC_PUB_ADD (SIG_ADD + SIG_SIZE)
#define ECC_PUB_SIZE (64)

void FW_Signature_Verify(void);

#endif /* __SIGCHECK_H */
