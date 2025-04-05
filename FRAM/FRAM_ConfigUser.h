/*
 * FRAM_ConfigUser.h
 *
 *  Created on: Apr 4, 2025
 *      Author: user
 */

#ifndef FRAM_CONFIGUSER_H_
#define FRAM_CONFIGUSER_H_
#include "main.h"
#define FRAM_SPI_Transmit FRAM_Transmit
#define FRAM_SPI_Receive  FRAM_Receive
#define FRAM_CS_SET       FRAM_CsHight
#define FRAM_CS_RESET     FRAM_CsLow
#define FRAM_GET_TICK     HAL_GetTick
#define FRAM_Delay        HAL_Delay
#endif /* FRAM_CONFIGUSER_H_ */
