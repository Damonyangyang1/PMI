#include "eeprom.h"

/******************************************
@breif:	 Flash存储一个u32
*******************************************/
void FlashWriteDWord(uint32_t lAddr,uint32_t lData)
{
	
	uint32_t PageError = 0;

	FLASH_EraseInitTypeDef FlashEraseDef;				//初始化FLASH_EraseInitTypeDef
	FlashEraseDef.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashEraseDef.PageAddress = lAddr;
	FlashEraseDef.NbPages = 1;
	
	HAL_FLASH_Unlock();									//解锁FLASH
	HAL_FLASHEx_Erase(&FlashEraseDef, &PageError);		//擦除FLASH
	HAL_FLASH_Program(TYPEPROGRAM_WORD, lAddr, lData);	//对FLASH烧写
	HAL_FLASH_Lock();									//锁住FLASH
}

/******************************************
@breif:	 Flash读取一个u32
*******************************************/
uint32_t FlashReadDWord(uint32_t lAddr)
{
	return *(__IO uint32_t*)(lAddr);
}

/******************************************
@breif:	 Flash存储一个u32 数组
*******************************************/
void FlashWriteArr(uint32_t lStartAddr,uint32_t* lData,uint16_t nLen)
{
	
	uint32_t PageError = 0;
	uint16_t ni = 0 ;

	FLASH_EraseInitTypeDef FlashEraseDef;				//初始化FLASH_EraseInitTypeDef
	FlashEraseDef.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashEraseDef.PageAddress = lStartAddr;
	FlashEraseDef.NbPages = 1;
	
	HAL_FLASH_Unlock();									//解锁FLASH
	
	HAL_FLASHEx_Erase(&FlashEraseDef, &PageError);		//擦除FLASH
	
	for(ni=0;ni<nLen;ni++)
	{
		HAL_FLASH_Program(TYPEPROGRAM_WORD, lStartAddr, lData[ni]);	//对FLASH烧写
		lStartAddr += 4;
	}
	HAL_FLASH_Lock();									//锁住FLASH
}

/******************************************
@breif:	 Flash读取一个u32 数组
*******************************************/
void FlashReadArr(uint32_t lStartAddr,uint32_t * lDataArr,uint16_t nLen)
{
	uint16_t ni = 0;
	for(ni=0;ni<nLen;ni++) 
	{
		lDataArr[ni] = *(__IO uint32_t*)(lStartAddr);
		lStartAddr += 4;
	}
	
	
}

