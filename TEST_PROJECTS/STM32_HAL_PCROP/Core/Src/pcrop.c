/*
 * pcrop.c
 *
 *  Created on: 2022. 6. 23.
 *      Author: jihoo
 */

#include <stdio.h>
#include "main.h"

uint32_t cnt = 0;

void mySecretLED(void)
{
  cnt++;
  printf("mySecretLED: %ld\r\n", cnt);
  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

