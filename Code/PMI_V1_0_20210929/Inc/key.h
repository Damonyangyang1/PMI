#ifndef _H_KEY_H
#define _H_KEY_H

#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"

#define		   NULL_PRESS                   0
#define		   UP_PRESS						1
#define		   DOWN_PRESS					2
#define		   RIGHT_PRESS					3
#define		   LEFT_PRESS					4
#define		   ENTER_PRESS					5
#define		   ESC_PRESS					6

/* 定义按键值 */
#define        KEY_NULL_VALUE        		0x00

#define        KEY_UP_VALUE                 0X02
#define        KEY_DOWN_VALUE               0X04
#define        KEY_RIGHT_VALUE              0X08
#define        KEY_LEFT_VALUE               0X10
#define        KEY_ENTER_VALUE              0X20
#define        KEY_ESC_VALUE                0X40
/* 定义按键状态值 */
#define        KEY_PRESS                0X10
#define        KEY_LONG                 0X20
#define        KEY_CONTINUE             0X40
#define        KEY_FREE                 0X80
/* 定义按键处理状态 */
#define        KEY_INIT_STATE           0
#define        KEY_WOBBLE_STATE         1
#define        KEY_PRESS_STATE          2
#define        KEY_LONG_STATE           3
#define        KEY_CONTINUE_STATE       4
#define        KEY_RELEASE_STATE        5
/* 长按时间 */
#define        KEY_LONG_PERIOD            200        /* 2S */
#define        KEY_CONTINUE_PERIOD        50         /* 500ms */

//高电平触发
//#define        IS_KEY_PRESS        (KEY_UP == 1) || (KEY_DOWN == 1) || (KEY_RIGHT == 1) || (KEY_LEFT == 1) || (KEY_ENTER == 1) || (KEY_ESC == 1)
//#define        IS_KEY_FREE         (KEY_UP == 0) && (KEY_DOWN == 0) && (KEY_RIGHT == 0) && (KEY_LEFT == 0) && (KEY_ENTER == 0) && (KEY_ESC == 0)

//低电平触发
#define        IS_KEY_PRESS        (KEY_UP == 0) || (KEY_DOWN == 0) || (KEY_RIGHT == 0) || (KEY_LEFT == 0) || (KEY_ENTER == 0) || (KEY_ESC == 0)
#define        IS_KEY_FREE         (KEY_UP == 1) && (KEY_DOWN == 1) && (KEY_RIGHT == 1) && (KEY_LEFT == 1) && (KEY_ENTER == 1) && (KEY_ESC == 1)

#define KEY_UP             HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin)          	
#define KEY_DOWN           HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port,KEY_DOWN_Pin)
#define KEY_RIGHT          HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port,KEY_RIGHT_Pin)          	
#define KEY_LEFT           HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port,KEY_LEFT_Pin)
#define KEY_ENTER          HAL_GPIO_ReadPin(KEY_ENTER_GPIO_Port,KEY_ENTER_Pin)          	
#define KEY_ESC            HAL_GPIO_ReadPin(KEY_ESC_GPIO_Port,KEY_ESC_Pin)



uint8_t get_Key_Value(void);
uint8_t Key_Scan(void);
uint8_t get_PressKey(void);

uint8_t KeyMonitor(void);
uint8_t MeasurePage_KeyMonitor(void);
#endif

