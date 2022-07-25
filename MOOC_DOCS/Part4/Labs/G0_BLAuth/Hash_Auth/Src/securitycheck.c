/**
  ******************************************************************************
  * @file    securitycheck.c
  * @brief   Low level security check module. Provides functions to check system
  *          security configurations.
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
#include "main.h"
#include "securitycheck.h"
#include <stdio.h>

#define EXCPT_Security_Error() Fatal_Error_Handler()

/**
  * @brief  Check and if not applied apply the Static security  protections to
  *         critical sections in Flash: RDP, WRP. Static security protections
  *         those protections not impacted by a Reset. They are set using the Option Bytes
  *         When the device is locked (RDP Level2), these protections cannot be changed anymore
  * @param  None
  * @note   If security setting apply fails, enter Error Handler
  */
void CheckApplyStaticProtections(void)
{
  FLASH_OBProgramInitTypeDef flash_option_bytes;
  int32_t isOBChangeToApply = 0;
  
  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

  /* Unlock the Options Bytes *************************************************/
  HAL_FLASH_OB_Unlock();

  /* Get Option Bytes status for WRP AREA_A  **********/
  flash_option_bytes.WRPArea     = OB_WRPAREA_ZONE_A;
  HAL_FLASHEx_OBGetConfig(&flash_option_bytes);

  /* Check/Apply RDP_Level 1. This is the minimum protection allowed */
  /* if RDP_Level 2 is already applied it's not possible to modify the OptionBytes anymore */
  if (flash_option_bytes.RDPLevel == OB_RDP_LEVEL_2)
  {
#ifdef WRP_PROTECT_ENABLE
    if ( (flash_option_bytes.WRPStartOffset > WRP_START_ADD) || 
         (flash_option_bytes.WRPEndOffset < WRP_END_ADD))
    {
      goto ERROR;
    }
#endif /* WRP_PROTECT_ENABLE */
  }
  else
  {
    /* Check/Set Flash configuration *******************************************/
    
#ifdef WRP_PROTECT_ENABLE
    /* Apply WRP setting only if expected settings are not included */
    if ( (flash_option_bytes.WRPStartOffset > WRP_START_ADD) || 
         (flash_option_bytes.WRPEndOffset < WRP_END_ADD))
    {
      flash_option_bytes.WRPStartOffset = WRP_START_ADD;
      flash_option_bytes.WRPEndOffset = WRP_END_ADD;
      
      printf("\r\nTry to apply WRP from page [%d] to [%d]\r\n", 
             flash_option_bytes.WRPStartOffset,
             flash_option_bytes.WRPEndOffset);
      if (HAL_FLASHEx_OBProgram(&flash_option_bytes) != HAL_OK)
      {
        goto ERROR;
      }
      isOBChangeToApply++;
    }
    printf("\r\nWRP already applied from page [%d] to [%d]\r\n", 
       flash_option_bytes.WRPStartOffset,
       flash_option_bytes.WRPEndOffset);
#endif /* WRP_PROTECT_ENABLE */

    
#ifdef RDP_PROTECT_ENABLE
    /* Apply RDP setting only if expected settings are not applied */
    if ( flash_option_bytes.RDPLevel != RDP_LEVEL_CONFIG )
    {
      flash_option_bytes.OptionType      = OPTIONBYTE_RDP | OPTIONBYTE_USER;
      flash_option_bytes.RDPLevel        = RDP_LEVEL_CONFIG;
      if (HAL_FLASHEx_OBProgram(&flash_option_bytes) != HAL_OK)
      {
        goto ERROR;
      }      
      
      printf("\r\nSet RDP to [0x%02x], please power off and power on again.\r\n",
             flash_option_bytes.RDPLevel); 
      isOBChangeToApply++;
    }
    else
    {
      printf("\r\nRDP level set to [0x%02x]\r\n", flash_option_bytes.RDPLevel);       
    }
#endif  /* RDP_PROTECT_ENABLE */

     /* Generate System Reset to reload the new option byte values *************/
     /* WARNING: This means that if a protection can't be set, there will be a reset loop! */
    if ( isOBChangeToApply > 0 )
    {
      HAL_FLASH_OB_Launch();
    }
  }

  /* Lock the Options Bytes ***************************************************/
  HAL_FLASH_OB_Lock();

  /* Lock the Flash to disable the flash control register access (recommended
  to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return;

ERROR:
  /* Lock the Options Bytes ***************************************************/
  HAL_FLASH_OB_Lock();

  /* Lock the Flash to disable the flash control register access (recommended
  to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();
  
  EXCPT_Security_Error();

}


