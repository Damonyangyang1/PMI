#ifndef __LED_H
#define __LED_H	 

#include "main.h"

#define 	RGB_LED_PIN 	 GPIO_PIN_15
#define 	RGB_LRD_PORT     GPIOA

#define RGB_LED_LOW              HAL_GPIO_WritePin(RGB_LRD_PORT, RGB_LED_PIN, GPIO_PIN_RESET)
#define RGB_LED_HIGH			 HAL_GPIO_WritePin(RGB_LRD_PORT, RGB_LED_PIN, GPIO_PIN_SET)

//#define		RGB_LED_HIGH	(RGB_LRD_PORT->BSRR=RGB_LED_PIN)
//#define 	RGB_LED_LOW		(RGB_LRD_PORT->BRR=RGB_LED_PIN )




#define IS_RCC_APB2_PERIPH(PERIPH) ((((PERIPH) & 0xFFC00002) == 0x00) && ((PERIPH) != 0x00))
#define EVCR_PORTPINCONFIG_MASK     ((uint16_t)0xFF80)
#define LSB_MASK                    ((uint16_t)0xFFFF)
#define DBGAFR_POSITION_MASK        ((uint32_t)0x000F0000)
#define DBGAFR_SWJCFG_MASK          ((uint32_t)0xF0FFFFFF)
#define DBGAFR_LOCATION_MASK        ((uint32_t)0x00200000)
#define DBGAFR_NUMBITS_MASK         ((uint32_t)0x00100000)

#define RCC_APB2Periph_AFIO              ((uint32_t)0x00000001)

 

void RGB_LED_Init(void);

void RGB_LED_Write0(void);

void choicelight(uint8_t x,uint8_t xclocr,uint8_t bgclocr);

void RGB_LED_Reset(void);

		 				    
#endif
