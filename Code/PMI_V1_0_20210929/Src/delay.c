#include "delay.h"

/******************************************
@breif:		us�ӳ�
*******************************************/
static uint8_t  fac_us=0;//us��ʱ������

//void delay_us(uint32_t nus)
//{
////    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * nus);       //HAL_RCC_GetHCLKFreq��ȡ������Ƶֵ
////    while (delay--)
////	{
////		;
////	}
////	__IO uint32_t Delay = nus * 72 / 8;//(SystemCoreClock / 8U / 1000000U)
//
////	do
////	{
////	__NOP();
////	}
////	while (Delay --); 
//	uint32_t temp;	    	 
//	SysTick->LOAD=nus*fac_us; //ʱ�����	  		 
//	SysTick->VAL=0x00;        //��ռ�����
//	SysTick->CTRL=0x01 ;      //��ʼ���� 	 
//	do
//	{
//		temp=SysTick->CTRL;
//	}
//	while(temp&0x01&&!(temp&(1<<16)));//�ȴ�ʱ�䵽��   
//	SysTick->CTRL=0x00;       //�رռ�����
//	SysTick->VAL =0X00;       //��ռ�����	
//}

void delay_us(uint32_t nus)
{
	uint32_t i=0;

	while(nus--)
	{
		for(i = 0 ;i<13;i++)
		{
			__nop();
		}
	}

}

/******************************************
@breif:		ms�ӳ�
*******************************************/
void delayms(uint16_t nms)
{	 		  	  
	HAL_Delay(nms);
} 
/******************************************
@breif:		s�ӳ�
*******************************************/
void delayS(uint16_t ns)
{	 	
	uint16_t ni,nj;
	for(ni=0; ni<ns; ni++)
	{
		for(nj=0; nj<50; nj++)
		{
			delayms(20);
		}
	}
} 
