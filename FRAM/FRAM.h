/*
 * FRAM.h
 *
 *  Created on: Apr 4, 2025
 *      Author: user
 */

#ifndef FRAM_H_
#define FRAM_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "FRAM_ConfigUser.h"
#define FRAM_DENSITY_TABLE_SIZE sizeof(FramDensity) / sizeof(FramDensityTable_TypeDef)
#define MANUFACTURER_ID_FUJITSU 0x04 // Now it should be called RAMXEED
#define FRAM_MAX_TIMEOUT        1000
#define FRAM_RDSR               0x05
#define FRAM_WREN               0x06
#define FRAM_RDID               0x9f
#define FRAM_WRITE              0x02
#define FRAM_READ               0x03
#define FRAM_WRSR               0x01
   typedef enum
   {
      FRAM_SR_BIT_RESET,
      FRAM_SR_BIT_SET
   } FramSRBitStatus_Typedef;
   typedef enum
   {
      FRAM_STATUS_OK,
      FRAM_STATUS_ERROR_SPI,
      FRAM_STATUS_UNKNOW_DENSITY,
      FRAM_STATUS_ERROR_CS,
      FRAM_STATUS_ERROR_ID_READ,
      FRAM_STATUS_MANUFACTURER_ID_ERROR,
      FRAM_STATUS_ADDRESS_ERROR,
      FRAM_TIMEOUT_ERROR,
      FRAM_SET_WEL_BIT_ERROR,
      FRAM_STATUS_ERROR_UNKNOW
   } FramStatus_TypeDef;

   typedef enum
   {
      FRAM_DENSITY_16Kb  = 0b00001,
      FRAM_DENSITY_64Kb  = 0b00011,
      FRAM_DENSITY_128Kb = 0b00100,
      FRAM_DENSITY_256Kb = 0b00101,
      FRAM_DENSITY_512Kb = 0b00110,
      FRAM_DENSITY_1Mb   = 0b00111,
      FRAM_DENSITY_2Mb   = 0b01000,
      FRAM_DENSITY_4Mb   = 0b01001,
      FRAM_DENSITY_8Mb   = 0b01010
   } FramDensity_TypeDef;

   typedef union
   {
      struct
      {
         uint32_t Mf                 : 8;
         uint32_t Pu1                : 8;
         FramDensity_TypeDef Density : 5;
         uint32_t Pu2                : 3;
         uint32_t Pu3                : 8;
      } Bit;
      uint32_t Word;
   } FramIdRegister_TypeDef;

   typedef union
   {
      struct
      {
         uint8_t Res1 : 1;
         uint8_t Wel  : 1;
         uint8_t BP0  : 1;
         uint8_t BP1  : 1;
         uint8_t Res2 : 3;
         uint8_t Wpen : 1;
      } Bit;
      uint8_t Word;
   } FramSreg_TypeDef;

   typedef struct
   {
      FramIdRegister_TypeDef ID;
      FramSreg_TypeDef SR;
      uint32_t FramSize;
   } Fram_TypeDef;

   typedef struct
   {
      FramDensity_TypeDef FramDensityCode;
      uint32_t FramDensity;
   } FramDensityTable_TypeDef;
#ifndef FRAM_SPI_Transmit
#error "First, declare the functions to send over SPI"
#endif
#ifndef FRAM_SPI_Receive
#error "First, declare the functions to receive on SPI"
#endif
#ifndef FRAM_CS_SET
#error "First, declare a function to set CS to a high state"
#endif
#ifndef FRAM_CS_RESET
#error "First, declare a function to set CS to a low state"
#endif
#ifndef FRAM_GET_TICK
#error "First, declare the time retrieval function in ms"
#endif
#ifndef FRAM_Delay
#error "First, declare the delay function"
#endif
   FramStatus_TypeDef FRAM_Init(void);
   FramStatus_TypeDef FRAM_Write(uint32_t Addr, uint8_t *Buf, uint32_t Size);
   FramStatus_TypeDef FRAM_Read(uint32_t Addr, uint8_t *Buf, uint32_t Size);
   FramStatus_TypeDef FRAM_Clean(uint32_t Addr, uint32_t Size);
   FramStatus_TypeDef FRAM_FullChipErase(void);
#ifdef __cplusplus
}
#endif
#endif /* FRAM_H_ */
