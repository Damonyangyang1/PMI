#ifndef _fun_H_
#define _fun_H_
#include "main.h"


typedef struct
{
	uint8_t usampleFlag;
}ADC_SAMPLE_FLAG;

void Main_interface_streat(void);
void Ms_Treat(void);

uint8_t* Double2Char(double lnum,uint8_t uType);
uint8_t* Int2Char(uint16_t nnum);
uint8_t* _Double2Char(double lnum,uint8_t uType);

void Double2StrArr(double lNum,uint8_t (*uArr)[2],uint8_t uType);
double StrArr2Double(uint8_t (*uArr)[2],uint8_t uType);

void FlashReadSettingValue(void);
void FlashWriteSettingValue(void);
void FlashWrite_CurStationNo(uint32_t lStationNo);
uint32_t FlashRead_CurStationNo(void);

void InitSettingData(uint32_t lStationNo);
void FlashUpdateSettingData(uint32_t lStationNo);	
uint8_t ParamTreat(uint32_t lCompare,uint32_t lMin,uint32_t lMax);

void FlashClearSettingValue(void);
void FlashData_Init(void);
uint8_t *Int_To_Str(uint32_t lnum);

uint8_t ValueCorrect(double x1,double y1,double x2 ,double y2);
double CalDetaValue(uint16_t nValue);

void Gui_Show_Num32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc,uint8_t *s,uint8_t uType);
#endif
