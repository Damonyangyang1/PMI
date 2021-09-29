/**
  ******************************************************************************
  * File Name          : gpio.h
  * Description        : This file contains all the functions prototypes for 
  *                      the gpio  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#define Sensor_output_Pin GPIO_PIN_1
#define Sensor_output_GPIO_Port GPIOA
/* USER CODE END Includes */
#define LED2_Pin 											GPIO_PIN_13
#define LED2_GPIO_Port 										GPIOC
//LCD GPIO
#define CS_Pin 												GPIO_PIN_3
#define CS_GPIO_Port 										GPIOA
#define SCL_Pin 											GPIO_PIN_7
#define SCL_GPIO_Port 										GPIOA
#define SDA_Pin 											GPIO_PIN_6
#define SDA_GPIO_Port 										GPIOA
#define RES_Pin 											GPIO_PIN_5
#define RES_GPIO_Port 										GPIOA
#define DC_Pin 												GPIO_PIN_4
#define DC_GPIO_Port 										GPIOA
#define BLK_Pin 											GPIO_PIN_2
#define BLK_GPIO_Port 										GPIOA
//KEY GPIO
#define KEY_UP_Pin 											GPIO_PIN_13
#define KEY_UP_GPIO_Port 									GPIOB
#define KEY_DOWN_Pin 										GPIO_PIN_12
#define KEY_DOWN_GPIO_Port 									GPIOB
#define KEY_LEFT_Pin 										GPIO_PIN_11
#define KEY_LEFT_GPIO_Port 									GPIOB
#define KEY_RIGHT_Pin 										GPIO_PIN_10
#define KEY_RIGHT_GPIO_Port 								GPIOB
#define KEY_ESC_Pin 										GPIO_PIN_15
#define KEY_ESC_GPIO_Port 									GPIOB
#define KEY_ENTER_Pin 										GPIO_PIN_14
#define KEY_ENTER_GPIO_Port 								GPIOB

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
#define LED_ON              HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET)
#define LED_OFF				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET)
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
