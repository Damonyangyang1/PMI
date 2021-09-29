#include "fun.h"
#include "stdlib.h"

extern TIMECNT   TimeCnt;  //这是tim的文件调用过来的
uint8_t KeyMainFlag=0;
extern MEASUREDATAINFO MeasureData;
extern CURSORINFO PageCursor;
extern uint32_t _SETTING_VALUE[MAX_STATION_NUM][MAX_PARAM_NUM]; 
extern SETTINGDATAINFO SettingData;
extern CORRECTVALUEINFO CorrectValue;
ADC_SAMPLE_FLAG  adc_sample_flag = {0};


/******************************************
@breif:		定时处理函数
//根据频率调用相应的函数
*******************************************/
void Ms_Treat(void)
{
	//10ms处理
	if(TimeCnt.b10ms == 1)      
	{
		TimeCnt.b10ms = 0 ; 	//10ms标志清零
		KeyMainFlag = 1;  		//10ms检测键盘动作
	}
	//100ms处理
	if(TimeCnt.b100ms == 1)   	
	{
		TimeCnt.b100ms = 0;		//100ms闪烁一次

		if(PageCursor.uBlinkFlag) //取反一次
			{
				PageCursor.uBlinkFlag = 0;
			}
		else PageCursor.uBlinkFlag = 1;
	}
	//500ms处理
	if(TimeCnt.b500ms == 1)     
	{
		TimeCnt.b500ms = 0 ;
		MeasureData.uMeasureFlag = 1;  //开启ADC采样

		#ifdef TEST
		MeasureData.lDeta+=0.0010;
		#endif
	} 
}


/******************************************
功能：浮点数转字符二维数组  
	  转换结果：
	  {{'1','\0'},{'2','\0'},{'3','\0'},{'4','\0'}} 
			即123.4
规定：uType=8时double最大为9999.9999
	  uType=4时double最大为999.9
*******************************************/
void Double2StrArr(double lNum,uint8_t (*uArr)[2],uint8_t uType)
{
	uint32_t lTemp = 0;
	if(uType == 8)
	{
		lTemp = (lNum+0.00001)*10000;
		uArr[7][0] = lTemp/10000000+'0';
		uArr[7][1] = '\0';
		uArr[6][0] = lTemp%10000000/1000000+'0';
		uArr[6][1] = '\0';
		uArr[5][0] = lTemp%1000000/100000+'0';
		uArr[5][1] = '\0';
		uArr[4][0] = lTemp%100000/10000+'0';
		uArr[4][1] = '\0';
		uArr[3][0] = lTemp%10000/1000+'0';
		uArr[3][1] = '\0';
		uArr[2][0] = lTemp%1000/100+'0';
		uArr[2][1] = '\0';
		uArr[1][0] = lTemp%100/10+'0';
		uArr[1][1] = '\0';
		uArr[0][0] = lTemp%10+'0';
		uArr[0][1] = '\0';
	}
	else if(uType == 4)
	{
		lTemp = (lNum+0.00001)*10;
		uArr[3][0] = lTemp/1000+'0';
		uArr[3][1] = '\0';
		uArr[2][0] = lTemp%1000/100+'0';
		uArr[2][1] = '\0';
		uArr[1][0] = lTemp%100/10+'0';
		uArr[1][1] = '\0';
		uArr[0][0] = lTemp%10+'0';
		uArr[0][1] = '\0';
	}
	
}

/******************************************
功能：字符二维数组转浮点数  
      数组示意：{{'1','\0'},{'2','\0'},{'3','\0'},{'4','\0'}} 
				即123.4
规定：1.uType=8时double最大为9999.9999
	    uType=4时double最大为999.9
*******************************************/
double StrArr2Double(uint8_t (*uArr)[2],uint8_t uType)
{
	double lTemp=0 ;
	if(uType == 8)
	{
		lTemp += (uArr[7][0]-'0')*1000;
		lTemp += (uArr[6][0]-'0')*100;
		lTemp += (uArr[5][0]-'0')*10;
		lTemp += (uArr[4][0]-'0');
		lTemp += (uArr[3][0]-'0')*0.1;
		lTemp += (uArr[2][0]-'0')*0.01;
		lTemp += (uArr[1][0]-'0')*0.001;
		lTemp += (uArr[0][0]-'0')*0.0001;
		return lTemp; //解决精度问题
	}
	else if(uType==4)
	{
		lTemp += (uArr[3][0]-'0')*100;
		lTemp += (uArr[2][0]-'0')*10;
		lTemp += (uArr[1][0]-'0');
		lTemp += (uArr[0][0]-'0')*0.1;
		return lTemp;
	}
	return NULL;
}

/******************************************
功能：double小数转char字符串
	  转出的字符串去除整数多余0
	  只保留小数点最前面的一个0
规定：1.double最大为9999.9999
注意：由于字符长度不定，使用malloc分配，需要手动释放
*******************************************/
uint8_t* Double2Char(double lnum,uint8_t uType)
{
	if(uType == 8)
	{
		uint32_t lTemp = (lnum+0.00001)*10000;
		uint8_t  uTemp[9];
		uint8_t ui =0;
		uTemp[0] = lTemp/10000000+'0';
		uTemp[1] = lTemp%10000000/1000000+'0';
		uTemp[2] = lTemp%1000000/100000+'0';
		uTemp[3] = lTemp%100000/10000+'0';
		uTemp[4] = '.';
		uTemp[5] = lTemp%10000/1000+'0';
		uTemp[6] = lTemp%1000/100+'0';
		uTemp[7] = lTemp%100/10+'0';
		uTemp[8] = lTemp%10+'0';
		for(;ui < 3;ui++)
		{
			if(uTemp[ui] != '0')break;
		}
		uint8_t ulen = (9- ui),uj=0;
		uint8_t *uResData = (uint8_t*)malloc(ulen+1);  
		for( ;uj < ulen;uj++)
		{
			uResData[uj] = uTemp[ui++];
		}
		uResData[uj] = '\0';
		return uResData;
	}
	else if(uType == 6)
	{
		uint32_t lTemp = (lnum+0.00001)*10000;
		uint8_t  uTemp[7];
		uint8_t ui =0;
		uTemp[0] = lTemp/100000+'0';
		uTemp[1] = lTemp%100000/10000+'0';
		uTemp[2] = '.';
		uTemp[3] = lTemp%10000/1000+'0';
		uTemp[4] = lTemp%1000/100+'0';
		uTemp[5] = lTemp%100/10+'0';
		uTemp[6] = lTemp%10+'0';
		for(;ui < 1;ui++)
		{
			if(uTemp[ui] != '0')break;
		}
		uint8_t ulen = (7- ui),uj=0;
		uint8_t *uResData = (uint8_t*)malloc(ulen+1);  
		for( ;uj < ulen;uj++)
		{
			uResData[uj] = uTemp[ui++];
		}
		uResData[uj] = '\0';
		return uResData;
	}
}

/******************************************
功能：double小数转char字符串
	 *转出的字符串保留整数多余0
规定：1.uType=8时 double最大为9999.9999
		uType=4时 double最大为999.9
注意：由于字符长度不定，使用malloc分配，需要手动释放
*******************************************/
uint8_t  uTemp[9];
uint8_t* _Double2Char(double lnum,uint8_t uType)
{
	uint32_t lTemp =0;
	
	uint8_t ui =0;
	if(uType == 8)
	{
		lTemp = (lnum+0.00001)*10000;
		uTemp[0] = lTemp/10000000+'0';
		uTemp[1] = lTemp%10000000/1000000+'0';
		uTemp[2] = lTemp%1000000/100000+'0';
		uTemp[3] = lTemp%100000/10000+'0';
		uTemp[4] = '.';
		uTemp[5] = lTemp%10000/1000+'0';
		uTemp[6] = lTemp%1000/100+'0';
		uTemp[7] = lTemp%100/10+'0';
		uTemp[8] = lTemp%10+'0';
		uint8_t *uResData = (uint8_t*)malloc(10);  
		for( ;ui < 9;ui++)
		{
			uResData[ui] = uTemp[ui];
		}
		uResData[ui] = '\0';
		return uResData;
	}
	else if(uType == 4)
	{
		lTemp = (lnum+0.00001)*10;
		uTemp[0] = lTemp/1000+'0';
		uTemp[1] = lTemp%1000/100+'0';
		uTemp[2] = lTemp%100/10+'0';
		uTemp[3] = '.';
		uTemp[4] = lTemp%10+'0';
		uint8_t *uResData = (uint8_t*)malloc(6);  
		for( ;ui < 5;ui++)
		{
			uResData[ui] = uTemp[ui];
		}
		uResData[ui] = '\0';
		return uResData;
	}
	return NULL;

}

void Gui_Show_Num32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc,uint8_t *s,uint8_t uType)
{
	uint8_t ui,ux=x,uy=y;
	if(uType == 8)
	{
		for(ui=0;ui<9;ui++)
		{
			if(ui != 4)
			{
				Gui_DrawFont_Num32(ux,uy,fc,bc,*s-'0');
				s++;
				ux+=24;
			}
			else s++;
			
		}
	}
	
}


/******************************************
功能：整数转char字符串
	  最大支持65536，去除整数前部多余0
注意：由于字符长度不定，使用malloc分配，需要手动释放
*******************************************/
uint8_t* Int2Char(uint16_t nnum)
{
	uint8_t  uTemp[5];
	uint8_t ui =0;
	uTemp[0] = nnum/10000+'0';
	uTemp[1] = nnum%10000/1000+'0';
	uTemp[2] = nnum%1000/100+'0';
	uTemp[3] = nnum%100/10+'0';
	uTemp[4] = nnum%10+'0';
	for(;ui<4;ui++)
	{
		if(uTemp[ui] != '0')break;
	}
	uint8_t ulen = 5 - ui,uj=0;
	uint8_t *uResData = (uint8_t*)malloc(ulen);  
	for( ;uj<ulen;uj++)
	{
		uResData[uj] = uTemp[ui++];
	}
	uResData[uj] = '\0';
	return uResData;
}

//------------------------------------------------------------------------
//功	能 Flash参数初始化
//------------------------------------------------------------------------
void FlashData_Init(void)
{
	uint8_t uError = 0;
	uint16_t ui = 0,uj=0;
	
	
	SettingData.lStationNo = FlashRead_CurStationNo(); //2021.0924新增 

	FlashReadSettingValue();  //读取参数值
	
	//判断Flash存储的数据是否在范围内
	for(;ui<MAX_STATION_NUM;ui++)
	{
		uError |= ParamTreat(_SETTING_VALUE[ui][TOUPLIMITSYB_INDEX],0,1); 			//取值 0-1
		uError |= ParamTreat(_SETTING_VALUE[ui][TOLOLIMITSYB_INDEX],0,1); 			//取值 0-1
		uError |= ParamTreat(_SETTING_VALUE[ui][MEASUREUPWARNING_INDEX],0,999999999); 			//取值 0-9999.9999
		uError |= ParamTreat(_SETTING_VALUE[ui][MEASURELOWARNING_INDEX],0,999999999); 			//取值 0-9999.9999
		uError |= ParamTreat(_SETTING_VALUE[ui][NOMISIZE_INDEX],0,999999999); 	//公称尺寸值 0-9999.9999
		uError |= ParamTreat(_SETTING_VALUE[ui][TOUPLIMIT_INDEX],0,9999); 	//超差上限值 0-999.9
		uError |= ParamTreat(_SETTING_VALUE[ui][TOLOLIMIT_INDEX],0,999999999); 	//超差下限值 0-9999.9999
		uError |= ParamTreat(_SETTING_VALUE[ui][STUPLIMIT_INDEX],0,999999999); 	//标件上限值 0-9999.9999
		uError |= ParamTreat(_SETTING_VALUE[ui][STLOLIMIT_INDEX],0,999999999); 	//标件下限值 0-9999.9999
		uError |= ParamTreat(_SETTING_VALUE[ui][MEMODE_INDEX],0,2); 			//模式取值 0-2
		uError |= ParamTreat(_SETTING_VALUE[ui][CORRECTSTUPLIMIT_INDEX],0,999999999);
		uError |= ParamTreat(_SETTING_VALUE[ui][CORRECTSTLOLIMIT_INDEX],0,999999999);
		//、TODO设置值的大小判断
	}
	//如果不在正常范围内 清零
	if(uError)
	{
		FlashClearSettingValue();
		for(ui=0;ui<MAX_STATION_NUM;ui++)
			for(uj=0;uj<MAX_PARAM_NUM;uj++)_SETTING_VALUE[ui][uj] = 0;
	}

}

//------------------------------------------------------------------------
//功    能: 参数默认值处理
//------------------------------------------------------------------------
uint8_t ParamTreat(uint32_t lCompare,uint32_t lMin,uint32_t lMax)
{
    //uint16_t ni,nj;
	 if(lCompare < lMin || lCompare > lMax )
	{
		return 1;
	}
	else return 0; 
}

//2021.0924新增
//读取关机前存储的站号 
uint32_t FlashRead_CurStationNo(void)
{
	uint32_t lStationNo=0;
	lStationNo =FlashReadDWord(STATIONO_BASE_ADDRSS);
	return lStationNo;
}

//2021.0924新增
//写入当前站点号 
void FlashWrite_CurStationNo(uint32_t lStationNo)
{
	FlashWriteDWord(STATIONO_BASE_ADDRSS,lStationNo);
}


//------------------------------------------------------------------------
//功	能 读取Flash存储的设置值 从基地址读取
//------------------------------------------------------------------------
void FlashReadSettingValue(void)
{
	uint16_t ni,nj,nLen=MAX_PARAM_NUM*MAX_STATION_NUM,nk=0;
	//uint32_t lAddr = SETTINGVALUE_BASE_ADDRSS;
	uint32_t lTemp[nLen];
	FlashReadArr(SETTINGVALUE_BASE_ADDRSS,lTemp,nLen);
	for(ni=0;ni<MAX_STATION_NUM;ni++)
	{
		for(nj=0;nj<MAX_PARAM_NUM;nj++)_SETTING_VALUE[ni][nj] = lTemp[nk++];
	}
	
}

/**********************************************
功能 向Flash写入存储的设置值
************************************************/
void FlashWriteSettingValue(void)
{
	uint16_t ni,nj,nLen = MAX_PARAM_NUM*MAX_STATION_NUM,nk=0;
	uint32_t lData[nLen];
	for(ni=0;ni<MAX_STATION_NUM;ni++)
	{
		for(nj=0;nj<MAX_PARAM_NUM;nj++)lData[nk++] = _SETTING_VALUE[ni][nj];
	}
	FlashWriteArr(SETTINGVALUE_BASE_ADDRSS,lData,nLen);
}

/**********************************************
功能 根据站号 初始化所有设置参数
************************************************/
void InitSettingData(uint32_t lStationNo)
{
	//赋值使用
	SettingData.lNominalSize = _SETTING_VALUE[lStationNo][NOMISIZE_INDEX]*0.00001;
	SettingData.lToleranceUpperLimit = _SETTING_VALUE[lStationNo][TOUPLIMIT_INDEX]*0.1;
	SettingData.lToleranceLowerLimit = _SETTING_VALUE[lStationNo][TOLOLIMIT_INDEX]*0.1;
	SettingData.uToleranceUpperLimitSymbol = _SETTING_VALUE[lStationNo][TOUPLIMITSYB_INDEX];
	SettingData.uToleranceLowerLimitSymbol = _SETTING_VALUE[lStationNo][TOLOLIMITSYB_INDEX];
	SettingData.lStandardUpperLimit = _SETTING_VALUE[lStationNo][STUPLIMIT_INDEX]*0.00001;
	SettingData.lStandardLowerLimit = _SETTING_VALUE[lStationNo][STLOLIMIT_INDEX]*0.00001;
	SettingData.lMeasureMode = _SETTING_VALUE[lStationNo][MEMODE_INDEX];
	SettingData.lTUpperLimitWarning = _SETTING_VALUE[lStationNo][MEASUREUPWARNING_INDEX]*0.00001;
	SettingData.lTLowerLimitWarning = _SETTING_VALUE[lStationNo][MEASURELOWARNING_INDEX]*0.00001;
	CorrectValue.lCorrectUpperLimitValue = _SETTING_VALUE[lStationNo][CORRECTSTUPLIMIT_INDEX]*0.00001;
	CorrectValue.lCorrectLowerLimitValue = _SETTING_VALUE[lStationNo][CORRECTSTLOLIMIT_INDEX]*0.00001;
}

/**********************************************
功能 更新 设置参数值
************************************************/
void FlashUpdateSettingData(uint32_t lStationNo)
{
	//保存当前配置参数值
	_SETTING_VALUE[lStationNo][MEASUREUPWARNING_INDEX] = SettingData.lTUpperLimitWarning*100000;
	_SETTING_VALUE[lStationNo][MEASURELOWARNING_INDEX] = SettingData.lTLowerLimitWarning*100000;
	_SETTING_VALUE[lStationNo][TOUPLIMITSYB_INDEX] = SettingData.uToleranceUpperLimitSymbol;
	_SETTING_VALUE[lStationNo][TOLOLIMITSYB_INDEX] = SettingData.uToleranceLowerLimitSymbol;
	_SETTING_VALUE[lStationNo][NOMISIZE_INDEX] = SettingData.lNominalSize*100000;
	_SETTING_VALUE[lStationNo][TOUPLIMIT_INDEX] = SettingData.lToleranceUpperLimit*10;//999.9
	_SETTING_VALUE[lStationNo][TOLOLIMIT_INDEX] = SettingData.lToleranceLowerLimit*10 ;
	_SETTING_VALUE[lStationNo][STUPLIMIT_INDEX] = SettingData.lStandardUpperLimit *100000;//9999.9999
	_SETTING_VALUE[lStationNo][STLOLIMIT_INDEX] = SettingData.lStandardLowerLimit*100000;
	_SETTING_VALUE[lStationNo][MEMODE_INDEX] = SettingData.lMeasureMode;
	_SETTING_VALUE[lStationNo][CORRECTSTUPLIMIT_INDEX] = CorrectValue.lCorrectUpperLimitValue*100000;
	_SETTING_VALUE[lStationNo][CORRECTSTLOLIMIT_INDEX] = CorrectValue.lCorrectLowerLimitValue*100000;
	//_SETTING_VALUE[lStationNo][SAVE_INDEX] = CorrectValue.lCorrectLowerLimitValue*100000;
	//重新写入
	FlashWriteSettingValue();
}


/**********************************************
功能 清除Flash存储的设置值 置为0
************************************************/
void FlashClearSettingValue(void)
{
	uint16_t ni,nj,nLen = MAX_PARAM_NUM*MAX_STATION_NUM;
	uint32_t lData[nLen];
	for(ni=0;ni<MAX_STATION_NUM;ni++)
	{
		for(nj=0;nj<MAX_PARAM_NUM;nj++)lData[ni] = 0;
	}
	FlashWriteArr(SETTINGVALUE_BASE_ADDRSS,lData,nLen);
}
/******************************************
@breif:		int类型--->字符串
*******************************************/
uint8_t *Int_To_Str(uint32_t lnum)
{
	uint8_t uTemp[4];      //整数最大到千
	uint8_t ui,uj,ulen;
	uTemp[0] = (lnum/1000) + '0';
	uTemp[1] = (lnum%1000)/100 + '0';
	uTemp[2] = ((lnum%1000)%100)/10 + '0';
	uTemp[3] = ((lnum%1000)%100)%10 + '0';
	for(ui=0;ui<3;ui++)
	{
		if(uTemp[ui] != '0')
		{
			break;
		}
	}
	ulen = 4 - ui;		
	uint8_t *result = (uint8_t *)malloc(ulen+1);
	for(uj=0;uj<ulen;uj++)
	{
		result[uj] = uTemp[ui++];
	}
	result[uj] = '\0';
	return result;
}



/******************************************
@breif:		上下限矫正 获得K值和B值
*******************************************/
uint8_t ValueCorrect(double x1,double y1,double x2 ,double y2)
{
	if(SettingData.lMeasureMode == INNER_MEMODE)
	{
		if((y2>y1)&&(x2>x1))
		{
			CorrectValue.lKValue = (y2 - y1)/((x2 - x1)*1.0);
			CorrectValue.lBValue = y1 - CorrectValue.lKValue * x1 ;
			return 1;
		}
		else return 0;
	}
	else if(SettingData.lMeasureMode == OUTER_MEMODE)
	{
		if((y2>y1)&&(x2<x1))
		{
			CorrectValue.lKValue = (y2 - y1)/((x2 - x1)*1.0);
			CorrectValue.lBValue = y1 - CorrectValue.lKValue * x1 ;
			return 1;
		}
		else return 0;
	}
	else return 0;
	
}


//y=kx+b输出工件的值
double CalDetaValue(uint16_t nValue)
{
	if(CorrectValue.lKValue != 0)
	{
		double lValue = CorrectValue.lKValue * nValue + CorrectValue.lBValue;
		return lValue;
	}
	else 
	{
		return 0;
	}
	
}

