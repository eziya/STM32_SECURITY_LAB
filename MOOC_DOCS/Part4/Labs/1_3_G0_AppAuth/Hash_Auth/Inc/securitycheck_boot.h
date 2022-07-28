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
#ifndef __SECURITYCHECK_H
#define __SECURITYCHECK_H

#include "jumper.h"

//#define WRP_PROTECT_ENABLE
//#define RDP_PROTECT_ENABLE
#define SECURE_USER_PROTECT_ENABLE 

#define WRP_START_ADD (0)
#define WRP_END_ADD     ((FW_LIMIT-FLASH_BASE)/FLASH_PAGE_SIZE-1)
#define RDP_LEVEL_CONFIG OB_RDP_LEVEL_1
#define BOOT_NB_PAGE_SEC_USER_MEM   ((FW_LIMIT-FLASH_BASE)/FLASH_PAGE_SIZE)  
    

void CheckApplyStaticProtections(void);
void CheckApplyDynamicProtections(void);

#endif /* __SECURITYCHECK_H */
