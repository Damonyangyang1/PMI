#include "delay.h"

/******************************************
@breif:		us延迟
*******************************************/
static uint8_t  fac_us=0;//us延时倍乘数

//void delay_us(uint32_t nus)
//{
////    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * nus);       //HAL_RCC_GetHCLKFreq获取的是主频值
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
//	SysTick->LOAD=nus*fac_us; //时间加载	  		 
//	SysTick->VAL=0x00;        //清空计数器
//	SysTick->CTRL=0x01 ;      //开始倒数 	 
//	do
//	{
//		temp=SysTick->CTRL;
//	}
//	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
//	SysTick->CTRL=0x00;       //关闭计数器
//	SysTick->VAL =0X00;       //清空计数器	
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
@breif:		ms延迟
*******************************************/
void delayms(uint16_t nms)
{	 		  	  
	HAL_Delay(nms);
} 
/******************************************
@breif:		s延迟
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
