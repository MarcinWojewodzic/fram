/*
 * FRAM.c
 *
 *  Created on: Apr 4, 2025
 *      Author: user
 */
#include "main.h"

#include "FRAM.h"

#include "stdio.h"
#ifndef FRAM_USE_EXTERNAL_DATA_BLOCK
__IO static Fram_TypeDef Fram = { 0 };
static Fram_TypeDef *DATA_GetFramPtr(void)
{
   return (Fram_TypeDef *)&Fram;
}
#endif
static const FramDensityTable_TypeDef FramDensity[] = { { FRAM_DENSITY_16Kb, 2048 },   { FRAM_DENSITY_64Kb, 8192 },   { FRAM_DENSITY_128Kb, 16384 },
                                                        { FRAM_DENSITY_256Kb, 32768 }, { FRAM_DENSITY_512Kb, 65536 }, { FRAM_DENSITY_1Mb, 131072 },
                                                        { FRAM_DENSITY_2Mb, 262144 },  { FRAM_DENSITY_4Mb, 524288 },  { FRAM_DENSITY_8Mb, 1048576 } };
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief The function assigns the capacity read from the ID
 *
 * @param none
 *
 * @retval Status [FRAM_STATUS_UNKNOW_DENSITY] - When no matching capacity is found
 * @retval Status [FRAM_STATUS_OK] - When all is well
 *
 */
static FramStatus_TypeDef ExtFram_FindDensity(void)
{
   FramStatus_TypeDef Status = FRAM_STATUS_UNKNOW_DENSITY;
   Fram_TypeDef *F           = DATA_GetFramPtr();
   for(int i = 0; i < FRAM_DENSITY_TABLE_SIZE; i++)
   {
      if(F->ID.Bit.Density == FramDensity[i].FramDensityCode)
      {
         F->FramSize = FramDensity[i].FramDensity;
         Status      = FRAM_STATUS_OK;
      }
   }
   return Status;
}

/**
 *
 * @brief Function reads ID from FRAM
 *
 * @param [*IdRegister] - Pointer to the variable corresponding to the FRAM ID
 *
 * @retval [FRAM_STATUS_ERROR_CS] - When there is a problem with the operation of the CS FRAM pin
 * @retval [FRAM_STATUS_ERROR_SPI] - When there is a problem with transmission via SPI
 * @retval Status [FRAM_STATUS_OK] - When all is well
 *
 */
static FramStatus_TypeDef FRAM_GetID(uint32_t *IdRegister)
{
   uint8_t Command           = FRAM_RDID;
   FramStatus_TypeDef Status = FRAM_STATUS_OK;
   if(FRAM_CS_RESET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   else if(FRAM_SPI_Transmit(&Command, 1) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_SPI_Receive((uint8_t *)IdRegister, 4) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_CS_SET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   return Status;
}

/**
 *
 * @brief The function retrieves the current state of the WEL bit from the SR
 *
 * @param none
 *
 * @retval [FRAM_SR_BIT_SET] - When the bit is set to 1
 * @retval [FRAM_SR_BIT_RESET] - When bit is set to 0 or an error has occurred in transmission via SPI
 *
 */
static FramSRBitStatus_Typedef FRAM_GetWELBit(void)
{
   uint8_t Command           = FRAM_RDSR;
   Fram_TypeDef *F           = DATA_GetFramPtr();
   FramStatus_TypeDef Status = FRAM_STATUS_OK;
   if(FRAM_CS_RESET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   else if(FRAM_SPI_Transmit(&Command, 1) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_SPI_Receive((uint8_t *)&F->SR.Word, 4) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_CS_SET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   if(Status == FRAM_STATUS_OK)
   {
      return (F->SR.Bit.Wel == FRAM_SR_BIT_SET) ? FRAM_SR_BIT_SET : FRAM_SR_BIT_RESET;
   }
   return FRAM_SR_BIT_RESET;
}

/**
 *
 * @brief The function attempts to set the WEL bit
 *
 * @param none
 *
 * @retval [FRAM_STATUS_ERROR_CS] - When there is a problem with the operation of the CS FRAM pin
 * @retval [FRAM_STATUS_ERROR_SPI] - When there is a problem with transmission via SPI
 * @retval [FRAM_TIMEOUT_ERROR] - When the WEL bit failed to set and a timeout occurred
 * @retval Status [FRAM_STATUS_OK] - When all is well
 *
 */
static FramStatus_TypeDef FRAM_SetWELBit(void)
{
   uint8_t Command           = FRAM_WREN;
   Fram_TypeDef *F           = DATA_GetFramPtr();
   FramStatus_TypeDef Status = FRAM_STATUS_OK;
   uint32_t Timeout          = FRAM_GET_TICK();
   if(FRAM_CS_RESET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   else if(FRAM_SPI_Transmit(&Command, 1) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_SPI_Receive((uint8_t *)&F->SR.Word, 4) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_CS_SET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   while(Status == FRAM_STATUS_OK && FRAM_GetWELBit() == FRAM_SR_BIT_RESET)
   {
      if(FRAM_GET_TICK() - Timeout > FRAM_MAX_TIMEOUT)
      {
         Status = FRAM_TIMEOUT_ERROR;
         break;
      }
   }
   return Status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief - The function determines the memory capacity and checks the operation of the SPI
 *
 * @param none
 *
 * @retval [FRAM_STATUS_ERROR_ID_READ] - When an error occurred while downloading the ID
 * @retval [FRAM_STATUS_UNKNOW_DENSITY] - When memory capacity could not be determined
 * @retval [FRAM_STATUS_MANUFACTURER_ID_ERROR] - When, despite no error in reading and no error in finding capacity, manufacturer ID does not agree
 * @retval Status [FRAM_STATUS_OK] - When all is well
 *
 */
FramStatus_TypeDef FRAM_Init(void)
{
   Fram_TypeDef *F           = DATA_GetFramPtr();
   FramStatus_TypeDef Status = FRAM_STATUS_OK;
   if(FRAM_GetID(&F->ID.Word) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_ID_READ;
   }
   else if(ExtFram_FindDensity() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_UNKNOW_DENSITY;
   }
   else if(F->ID.Bit.Mf != MANUFACTURER_ID_FUJITSU)
   {
      Status = FRAM_STATUS_MANUFACTURER_ID_ERROR;
   }
   return Status;
}

/**
 *
 * @brief The function writes a buffer of the specified size into memory
 *
 * @param [Addr] - Address where data is to be entered
 * @param [*Buf] - Pointer to data
 * @param [Size] - Size of data to be entered
 *
 * @retval [FRAM_STATUS_ADDRESS_ERROR] - When the Address exceeds the maximum size
 * @retval [FRAM_SET_WEL_BIT_ERROR] - When the WEL bit fails to be set
 * @retval [FRAM_STATUS_ERROR_CS] - When there is a problem with the operation of the CS FRAM pin
 * @retval [FRAM_STATUS_ERROR_SPI] - When there is a problem with transmission via SPI
 * @retval Status [FRAM_STATUS_OK] - When all is well
 *
 */
FramStatus_TypeDef FRAM_Write(uint32_t Addr, uint8_t *Buf, uint32_t Size)
{
   FramStatus_TypeDef Status = FRAM_STATUS_OK;
   Fram_TypeDef *F           = DATA_GetFramPtr();
   uint8_t TempCommand[4];
   uint8_t SizeCommand = 0;
   SizeCommand         = (F->ID.Bit.Density > FRAM_DENSITY_512Kb) ? 4 : 3;
   TempCommand[0]      = FRAM_WRITE;
   if(SizeCommand == 3)
   {
      TempCommand[1] = (Addr & 0xFF) >> 8;
      TempCommand[2] = (Addr & 0xFF) >> 0;
   }
   else
   {
      TempCommand[1] = (Addr & 0xFF) >> 16;
      TempCommand[2] = (Addr & 0xFF) >> 8;
      TempCommand[3] = (Addr & 0xFF) >> 0;
   }
   if((F->FramSize - Addr - Size) > F->FramSize)
   {
      Status = FRAM_STATUS_ADDRESS_ERROR;
   }
   else if(FRAM_SetWELBit() != FRAM_STATUS_OK)
   {
      Status = FRAM_SET_WEL_BIT_ERROR;
   }
   if(FRAM_CS_RESET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   else if(FRAM_SPI_Transmit(TempCommand, SizeCommand) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_SPI_Transmit(Buf, Size) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_CS_SET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   return Status;
}

/**
 *
 * @brief Function reads data from FRAM memory
 *
 * @param [Addr] - Address where data is to be entered
 * @param [*Buf] - Pointer to data
 * @param [Size] - Size of data to be entered
 *
 * @retval [FRAM_STATUS_ADDRESS_ERROR] - When the Address exceeds the maximum size
 * @retval [FRAM_SET_WEL_BIT_ERROR] - When the WEL bit fails to be set
 * @retval [FRAM_STATUS_ERROR_CS] - When there is a problem with the operation of the CS FRAM pin
 * @retval [FRAM_STATUS_ERROR_SPI] - When there is a problem with transmission via SPI
 * @retval Status [FRAM_STATUS_OK] - When all is well
 *
 */
FramStatus_TypeDef FRAM_Read(uint32_t Addr, uint8_t *Buf, uint32_t Size)
{
   FramStatus_TypeDef Status = FRAM_STATUS_OK;
   Fram_TypeDef *F           = DATA_GetFramPtr();
   uint8_t TempCommand[4];
   uint8_t SizeCommand = 0;
   SizeCommand         = (F->ID.Bit.Density > FRAM_DENSITY_512Kb) ? 4 : 3;
   TempCommand[0]      = FRAM_READ;
   if(SizeCommand == 3)
   {
      TempCommand[1] = (Addr & 0xFF) >> 8;
      TempCommand[2] = (Addr & 0xFF) >> 0;
   }
   else
   {
      TempCommand[1] = (Addr & 0xFF) >> 16;
      TempCommand[2] = (Addr & 0xFF) >> 8;
      TempCommand[3] = (Addr & 0xFF) >> 0;
   }
   if((F->FramSize - Addr - Size) > F->FramSize)
   {
      Status = FRAM_STATUS_ADDRESS_ERROR;
   }
   else if(FRAM_SetWELBit() != FRAM_STATUS_OK)
   {
      Status = FRAM_SET_WEL_BIT_ERROR;
   }
   if(FRAM_CS_RESET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   else if(FRAM_SPI_Transmit(TempCommand, SizeCommand) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_SPI_Receive(Buf, Size) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   else if(FRAM_CS_SET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   return Status;
}

/**
 *
 * @brief The function clears the given area (Sets it to 0)
 *
 * @param [Addr] - Address where data is to be entered
 * @param [Size] - Size of data to be entered
 *
 * @retval [FRAM_STATUS_ADDRESS_ERROR] - When the Address exceeds the maximum size
 * @retval [FRAM_SET_WEL_BIT_ERROR] - When the WEL bit fails to be set
 * @retval [FRAM_STATUS_ERROR_CS] - When there is a problem with the operation of the CS FRAM pin
 * @retval [FRAM_STATUS_ERROR_SPI] - When there is a problem with transmission via SPI
 * @retval Status [FRAM_STATUS_OK] - When all is well
 *
 */
FramStatus_TypeDef FRAM_Clean(uint32_t Addr, uint32_t Size)
{
   FramStatus_TypeDef Status = FRAM_STATUS_OK;
   Fram_TypeDef *F           = DATA_GetFramPtr();
   uint8_t TempCommand[4];
   uint8_t SizeCommand = 0;
   uint8_t TempData    = 0;
   SizeCommand         = (F->ID.Bit.Density > FRAM_DENSITY_512Kb) ? 4 : 3;
   TempCommand[0]      = FRAM_WRITE;
   if(SizeCommand == 3)
   {
      TempCommand[1] = (Addr & 0xFF) >> 8;
      TempCommand[2] = (Addr & 0xFF) >> 0;
   }
   else
   {
      TempCommand[1] = (Addr & 0xFF) >> 16;
      TempCommand[2] = (Addr & 0xFF) >> 8;
      TempCommand[3] = (Addr & 0xFF) >> 0;
   }
   if((F->FramSize - Addr - Size) > F->FramSize)
   {
      Status = FRAM_STATUS_ADDRESS_ERROR;
   }
   else if(FRAM_SetWELBit() != FRAM_STATUS_OK)
   {
      Status = FRAM_SET_WEL_BIT_ERROR;
   }
   if(FRAM_CS_RESET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   else if(FRAM_SPI_Transmit(TempCommand, SizeCommand) != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_SPI;
   }
   if(Status == FRAM_STATUS_OK)
   {
      uint32_t i = 0;
      for(i = 0; i < Size; i++)
      {
         Status = FRAM_SPI_Transmit(&TempData, 1);
      }
   }
   if(Status == FRAM_STATUS_OK && FRAM_CS_SET() != FRAM_STATUS_OK)
   {
      Status = FRAM_STATUS_ERROR_CS;
   }
   return Status;
}

/**
 *
 * @brief Function clears all available memory
 *
 * @param none
 *
 * @retval Returns what the FRAM_Clean function returns
 *
 */
FramStatus_TypeDef FRAM_FullChipErase(void)
{
   Fram_TypeDef *F = DATA_GetFramPtr();
   return FRAM_Clean(0x00000000, F->FramSize);
}
