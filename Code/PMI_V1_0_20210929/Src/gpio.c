/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
//引脚mapping
//              GND   电源地                 Up       PB13
//              VCC   接5V或3.3v电源         Down     PB12
//              SCL   接PA7                 Left     PB11
//              SDA   接PA6                 Right    PB10
//              RES   接PA5                 Esc      PB15
//              DC    接PA4                 Enter    PB14
//              CS    接PA3                 ADC      PA1
//				      BL	  接PA2
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);         //上电默认LED控制引脚高电平，灯灭

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BLK_Pin|DC_Pin|RES_Pin|CS_Pin|SCL_Pin|SDA_Pin, GPIO_PIN_RESET);


  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin PAPin */
  GPIO_InitStruct.Pin = BLK_Pin|DC_Pin|RES_Pin|CS_Pin|SCL_Pin|SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PBPin PBPin PBPin PBPin */
  GPIO_InitStruct.Pin = KEY_UP_Pin|KEY_DOWN_Pin|KEY_LEFT_Pin|KEY_RIGHT_Pin|KEY_ESC_Pin|KEY_ENTER_Pin ;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
