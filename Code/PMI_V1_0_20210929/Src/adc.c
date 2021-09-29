/**
  ******************************************************************************
  * File Name          : ADC.c
  * Description        : This file provides code for the configuration
  *                      of the ADC instances.
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

/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */
ADC_SAMPLE_Value adc_sample_value = {0,0};
extern ADC_SAMPLE_FLAG  adc_sample_flag;
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration    
    PA1     ------> ADC1_IN1 
    */
    GPIO_InitStruct.Pin = Sensor_output_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(Sensor_output_GPIO_Port, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();
  
    /**ADC1 GPIO Configuration    
    PA1     ------> ADC1_IN1 
    */
    HAL_GPIO_DeInit(Sensor_output_GPIO_Port, Sensor_output_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
/********************************************************
功能：DMA采样
*********************************************************/
void ADC_Get(uint16_t *ndmaBuf)
{
	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ndmaBuf,ADC_SAMPLE_CNT);
	HAL_Delay(1);
	HAL_ADC_Stop_DMA(&hadc1);
}
/********************************************************
功能：获取ADC采样值
*********************************************************/
uint16_t ndmaBuf[ADC_SAMPLE_CNT];     //DMA采集到的10次数据存入该数组
uint32_t Get_ADC_Sample_Value(void)
{
	uint8_t ui;
	ADC_Get(ndmaBuf);    //采样
	uint32_t lsampleValue = 0;
	for(ui=0;ui<ADC_SAMPLE_CNT;ui++)
	{
		lsampleValue += ndmaBuf[ui];
	}
	lsampleValue = lsampleValue *100/ ADC_SAMPLE_CNT ;    //10次之后求平均值
	return lsampleValue;
}
/********************************************************
功能：ADC转换位电压值  毫伏 mv 并且打印
*********************************************************/
uint32_t getMeasureVoltage(void)
{
  uint32_t lvolteValue;
  adc_sample_flag.usampleFlag = 0;
  uint32_t lvalue = Get_ADC_Sample_Value();
  lvolteValue = (lvalue*33)/4096;        //转换为电压计算  这是mv
  return lvolteValue;   //返回电压值
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
