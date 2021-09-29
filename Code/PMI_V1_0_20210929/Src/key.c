#include "key.h"


extern uint8_t KeyMainFlag;



//---------------------------------------------------------
// ��ȡ����ֵ����
//---------------------------------------------------------
uint8_t get_Key_Value(void)
{
//�͵�ƽ����
	if(KEY_UP == 0)            return KEY_UP_VALUE;
    else if(KEY_DOWN == 0)     return KEY_DOWN_VALUE;
	else if(KEY_RIGHT == 0)    return KEY_RIGHT_VALUE;
	else if(KEY_LEFT == 0)     return KEY_LEFT_VALUE;
	else if(KEY_ENTER == 0)    return KEY_ENTER_VALUE;
	else if(KEY_ESC == 0)      return KEY_ESC_VALUE;
    return KEY_NULL_VALUE;
//�ߵ�ƽ����
//  if(KEY_UP == 1)            return KEY_UP_VALUE;
//  else if(KEY_DOWN == 1)     return KEY_DOWN_VALUE;
//	else if(KEY_RIGHT == 1)    return KEY_RIGHT_VALUE;
//	else if(KEY_LEFT == 1)     return KEY_LEFT_VALUE;
//	else if(KEY_ENTER == 1)    return KEY_ENTER_VALUE;
//	else if(KEY_ESC == 1)      return KEY_ESC_VALUE;
//  return KEY_NULL_VALUE;
}

//---------------------------------------------------------
// ����ɨ�躯��
//---------------------------------------------------------
uint8_t Key_Scan(void)
{
	static uint8_t KeyState = KEY_INIT_STATE;
	static uint8_t IsKeyRelease = 1;
	static uint8_t KeyCounter = 0;
	static uint8_t KeyValueTemp = 0;
	uint8_t KeyValue = KEY_NULL_VALUE;
   
	if(IS_KEY_PRESS && IsKeyRelease)
	{
		switch(KeyState)
		{
			case KEY_INIT_STATE:
			{
				KeyState = KEY_WOBBLE_STATE;
				KeyValue = KEY_NULL_VALUE;
			}
				break;
			case KEY_WOBBLE_STATE:
			{
				KeyState = KEY_PRESS_STATE;
				KeyValue = KEY_NULL_VALUE;	
			}
				break;
			case KEY_PRESS_STATE:
			{
				KeyValue = get_Key_Value();
				KeyValueTemp = KeyValue;
				KeyState = KEY_LONG_STATE;		
			}
				break;
			case KEY_LONG_STATE:
			{
				KeyCounter ++;
				if(KeyCounter == KEY_LONG_PERIOD)
				{
					KeyCounter = 0;
					KeyState = KEY_CONTINUE_STATE;
					KeyValue =  (KeyValueTemp | KEY_LONG);
				}
			}
				break;
			case KEY_CONTINUE_STATE:
			{
				KeyCounter ++;
				if(KeyCounter == KEY_CONTINUE_PERIOD)
				{
					KeyCounter = 0;
					KeyValue = (KeyValueTemp | KEY_CONTINUE);
				}
			}
				break;
			default:
				break;
		}
		return KeyValue;
	}
	else if(IS_KEY_FREE)
	{
		KeyState = KEY_INIT_STATE;        /* �󴥷������ص���ʼ״̬ */
		IsKeyRelease = 1;
	}
	return KeyValue;
}

//---------------------------------------------------------
// Key������غ���
//---------------------------------------------------------
uint8_t KeyMonitor(void)
{
	uint8_t KeyValue = 0;
	uint8_t uPressKey = NULL_PRESS;
	if(KeyMainFlag)
	{
		KeyMainFlag = 0;
		KeyValue = Key_Scan();

		switch(KeyValue)
		{
			case KEY_UP_VALUE:
				uPressKey = UP_PRESS;
				break;
			case KEY_DOWN_VALUE:
				uPressKey = DOWN_PRESS;	
				break;
			case KEY_RIGHT_VALUE:
				uPressKey = RIGHT_PRESS;	
				break;
			case KEY_LEFT_VALUE:
				uPressKey = LEFT_PRESS;	
				break;
			case KEY_ENTER_VALUE:
				uPressKey = ENTER_PRESS;
				break;
			case KEY_ESC_VALUE:
				uPressKey = ESC_PRESS;
				break;
			default:
				uPressKey = NULL_PRESS;
				break;
		}	
	}
	return	uPressKey;
}


