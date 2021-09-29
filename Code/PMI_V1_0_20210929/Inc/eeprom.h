#ifndef __EEPROM_H
#define __EEPROM_H

#include "main.h"

#define  SETTINGVALUE_BASE_ADDRSS  0x0800FC00  //参数设置基地址

//#define  STATIONO_BASE_ADDRSS  0x0800FC00+0x100  //站号存储基地址


void FlashWriteArr(uint32_t lStartAddr,uint32_t* lData,uint16_t nLen);
void FlashReadArr(uint32_t lStartAddr,uint32_t * lDataArr,uint16_t nLen);

uint32_t FlashReadDWord(uint32_t lAddr);
void FlashWriteDWord(uint32_t lAddr,uint32_t lData);


#endif
