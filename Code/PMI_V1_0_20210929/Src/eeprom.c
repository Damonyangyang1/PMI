#include "eeprom.h"

/******************************************
@breif:	 Flash�洢һ��u32
*******************************************/
void FlashWriteDWord(uint32_t lAddr,uint32_t lData)
{
	
	uint32_t PageError = 0;

	FLASH_EraseInitTypeDef FlashEraseDef;				//��ʼ��FLASH_EraseInitTypeDef
	FlashEraseDef.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashEraseDef.PageAddress = lAddr;
	FlashEraseDef.NbPages = 1;
	
	HAL_FLASH_Unlock();									//����FLASH
	HAL_FLASHEx_Erase(&FlashEraseDef, &PageError);		//����FLASH
	HAL_FLASH_Program(TYPEPROGRAM_WORD, lAddr, lData);	//��FLASH��д
	HAL_FLASH_Lock();									//��סFLASH
}

/******************************************
@breif:	 Flash��ȡһ��u32
*******************************************/
uint32_t FlashReadDWord(uint32_t lAddr)
{
	return *(__IO uint32_t*)(lAddr);
}

/******************************************
@breif:	 Flash�洢һ��u32 ����
*******************************************/
void FlashWriteArr(uint32_t lStartAddr,uint32_t* lData,uint16_t nLen)
{
	
	uint32_t PageError = 0;
	uint16_t ni = 0 ;

	FLASH_EraseInitTypeDef FlashEraseDef;				//��ʼ��FLASH_EraseInitTypeDef
	FlashEraseDef.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashEraseDef.PageAddress = lStartAddr;
	FlashEraseDef.NbPages = 1;
	
	HAL_FLASH_Unlock();									//����FLASH
	
	HAL_FLASHEx_Erase(&FlashEraseDef, &PageError);		//����FLASH
	
	for(ni=0;ni<nLen;ni++)
	{
		HAL_FLASH_Program(TYPEPROGRAM_WORD, lStartAddr, lData[ni]);	//��FLASH��д
		lStartAddr += 4;
	}
	HAL_FLASH_Lock();									//��סFLASH
}

/******************************************
@breif:	 Flash��ȡһ��u32 ����
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

