#include "led.h"
//#include "main.h"
//#include "delay.h"



#define     LED_NUM   20 

#define GPIO_Remap_SWJ_JTAGDisable  ((uint32_t)0x00300200)  /*!< JTAG-DP Disabled and SW-DP Enabled */

void RCC_APB2PeriphClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_RCC_APB2_PERIPH(RCC_APB2Periph));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if (NewState != DISABLE)
  {
    RCC->APB2ENR |= RCC_APB2Periph;
  }
  else
  {
    RCC->APB2ENR &= ~RCC_APB2Periph;
  }
}

void GPIO_PinRemapConfig(uint32_t GPIO_Remap, FunctionalState NewState)
{
  uint32_t tmp = 0x00, tmp1 = 0x00, tmpreg = 0x00, tmpmask = 0x00;

  /* Check the parameters */
  //assert_param(IS_GPIO_REMAP(GPIO_Remap));
  assert_param(IS_FUNCTIONAL_STATE(NewState));  
  
  if((GPIO_Remap & 0x80000000) == 0x80000000)
  {
    tmpreg = AFIO->MAPR2;
  }
  else
  {
    tmpreg = AFIO->MAPR;
  }

  tmpmask = (GPIO_Remap & DBGAFR_POSITION_MASK) >> 0x10;
  tmp = GPIO_Remap & LSB_MASK;

  if ((GPIO_Remap & (DBGAFR_LOCATION_MASK | DBGAFR_NUMBITS_MASK)) == (DBGAFR_LOCATION_MASK | DBGAFR_NUMBITS_MASK))
  {
    tmpreg &= DBGAFR_SWJCFG_MASK;
    AFIO->MAPR &= DBGAFR_SWJCFG_MASK;
  }
  else if ((GPIO_Remap & DBGAFR_NUMBITS_MASK) == DBGAFR_NUMBITS_MASK)
  {
    tmp1 = ((uint32_t)0x03) << tmpmask;
    tmpreg &= ~tmp1;
    tmpreg |= ~DBGAFR_SWJCFG_MASK;
  }
  else
  {
    tmpreg &= ~(tmp << ((GPIO_Remap >> 0x15)*0x10));
    tmpreg |= ~DBGAFR_SWJCFG_MASK;
  }

  if (NewState != DISABLE)
  {
    tmpreg |= (tmp << ((GPIO_Remap >> 0x15)*0x10));
  }

  if((GPIO_Remap & 0x80000000) == 0x80000000)
  {
    AFIO->MAPR2 = tmpreg;
  }
  else
  {
    AFIO->MAPR = tmpreg;
  }  
}



void RGB_LED_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//使能端口复用时钟
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//失能JTAG
//	__HAL_AFIO_REMAP_SWJ_NONJTRST();
//	__HAL_RCC_AFIO_CLK_ENABLE();
//	__HAL_AFIO_REMAP_SWJ_NOJTAG();
	
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();

//	__HAL_RCC_AFIO_IS_CLK_ENABLED();



	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = RGB_LED_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(RGB_LRD_PORT, &GPIO_InitStruct);

	/*Configure GPIO pin Output Level */

	HAL_GPIO_WritePin(RGB_LRD_PORT, RGB_LED_PIN, GPIO_PIN_SET);


					 
}


void RGB_LED_Write0(void)
{
	RGB_LED_HIGH;
	__nop();__nop();__nop();__nop();
	RGB_LED_LOW;
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();
}


//void RGB_LED_Write0(void)
//{
//	RGB_LED_HIGH;
//	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();
//	RGB_LED_LOW;
//	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
//	__nop();__nop();__nop();__nop();__nop();
//
//}


/********************************************************/
//
/********************************************************/

void RGB_LED_Write1(void)
{
	RGB_LED_HIGH;
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();
	RGB_LED_LOW;
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();
}

void RGB_LED_Reset(void)
{
	RGB_LED_LOW;
	delay_us(75);
//	delayms(1);
//	RGB_LED_HIGH;
//delay_us(175);
//	RGB_LED_LOW;

}

//发送一个8位信号
void RGB_LED_Write_Byte(uint8_t byte)
{
	uint8_t i;

	for(i=0;i<8;i++)
		{
			if(byte&0x80) //从高到低
				{
					RGB_LED_Write1();//发送高占空比的信号
			}
			else
				{
					RGB_LED_Write0();//发送低占空比的信号
			}
		byte <<= 1;//每一个左移
	}
}


void RGB_LED_Write_24Bits(uint8_t green,uint8_t red,uint8_t blue)
{
	RGB_LED_Write_Byte(green);
	RGB_LED_Write_Byte(red);
	RGB_LED_Write_Byte(blue);
}

//24bit * 20 nun * 1.2us = 567us
//亮灯颜色设定，其他颜色以此类推
void RGB_LED_Red(void)
{
	 uint8_t i;
	//4个LED全彩灯
	for(i=0;i<20;i++)
		{
			RGB_LED_Write_24Bits(0, 0xff, 0);
	}
}

void RGB_LED_Green(void)
{
	uint8_t i;

	for(i=0;i<LED_NUM;i++)//这个是发送多少次
		{
			RGB_LED_Write_24Bits(0xff, 0, 0);
	}
}

void RGB_LED_Blue(void)
{
	uint8_t i;

	for(i=0;i<LED_NUM;i++)
		{
			RGB_LED_Write_24Bits(0, 0, 0xff);
	}
}

//参数 1 ：选中的第几个灯
//参数 2 ：选中灯的颜色
//参数 3 ：整个灯条的背景色
void choicelight(uint8_t x,uint8_t xclocr,uint8_t bgclocr)
{
	uint8_t i=0;
	
	for(i=0;i<20;i++)
	{
		if(i == x)
		{
			RGB_LED_Write_24Bits(0, 0xff, 0); //红灯
		}
		else
		{	
			RGB_LED_Write_24Bits(0xff, 0, 0);
		}
	}
}

//流水灯测试
void waterflow(void)
{
	uint8_t i =0;
	
	choicelight(i,0,0);
	delayms(100);
	i++;
	if(i>=19)
	{
		i = 0 ;
	}
}



