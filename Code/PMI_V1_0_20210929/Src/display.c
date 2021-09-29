#include "display.h"
#include "stdlib.h"
#include "usart.h"

typedef enum
{
	ERROR_RELEASE,
	ERROR_LIMIT,  		  //上下限错误
	ERROR_PARAM_1_NO_SET, //公差尺寸参数未设置
}ENUM_ERROR_INFO;

CURSORINFO 		 PageCursor = {1,0,0};      					  	//页面鼠标
PAGEINFO 		 ScreenPageInfo = {0,MEASURE_PAGE,MEASURE_PAGE};	//显示页面
MEASUREDATAINFO  MeasureData = {0,0,0,0,0,0};			    	    //测量页面数据信息
SETTINGDATAINFO  SettingData = {0,0,0,0};//1507635       	  	    //设置页面信息
CORRECTVALUEINFO CorrectValue;										//传感器曲线

uint32_t _SETTING_VALUE[MAX_STATION_NUM][MAX_PARAM_NUM];   	//10行12列 存放所有站点的设置值
uint32_t _SAVE_DATA[MAX_SAVE_NUM][MAX_SAVE_PARAM_NUM];     //100行3列 存放所有保存值
uint16_t nCur_Save_No; //当前保存的数据点  对应上面的每一行data 

uint8_t g_uErrorNo = ERROR_RELEASE;



//---------------------------------------------------------
// 功能：Screen初始化欢迎页面
//---------------------------------------------------------
void ScreenInit(void)
{
	Lcd_Init();						//LCD初始化
	FlashData_Init();				//Flash存储数据初始化
	InitSettingData(SettingData.lStationNo);
	//上电校准
	ValueCorrect(CorrectValue.lCorrectLowerLimitValue,SettingData.lStandardLowerLimit,CorrectValue.lCorrectUpperLimitValue,SettingData.lStandardUpperLimit); 					
	Lcd_Clear(BLACK);				//清屏
	Gui_DrawFont_GBK32(22,15,TEXTCOLOR,BGCOLOR, "ZONCETA");//显示欢迎页面
	Gui_DrawFont_GBK36(15,50,TEXTCOLOR,BGCOLOR, "气电量仪");
	Gui_DrawFont_GBK32(45,85,TEXTCOLOR,BGCOLOR, "V1.0");
	LCD_LED_SET;					//控制LCD背光亮
    delayms(2000);					//等待
	set_ScreenPageID(MEASURE_PAGE);	//设置显示页面ID为测量页面
	MeasureData.lMaxDeta = 0;
	MeasureData.lMinDeta = 9999.9999;
	Lcd_Clear(BLACK);				//清屏
	delayms(10);					//等待
	#ifdef TEST
	MeasureData.lDeta = 22.9100;
	#endif
}

//---------------------------------------------------------
// 功能：Screen闪烁显示字符串s
//---------------------------------------------------------
void Gui_BlinkShow(uint8_t ux,uint8_t uy,uint8_t *s)
{ 
	//通过定时器100ms改变标志位实现
	if(PageCursor.uBlinkFlag) 
	{
		Gui_DrawFont_GBK16(ux,uy,BGCOLOR,BGCOLOR,s);		//设置文字颜色和背景色一致，及文字消失
	}
	else
	{
		Gui_DrawFont_GBK16(ux,uy,TEXTCOLOR,BGCOLOR,s);		//设置文字颜色为正常，及文字显现
	}
}

//---------------------------------------------------------
// 功能：screen展示测量页面
//---------------------------------------------------------
uint16_t tempPercentColor;
double tempPercent;
uint16_t ntempPercent;
uint16_t tempBackColor =0;
double temp;

uint8_t redflag =1;//红灯控制标志
void ShowMeasurePage(void)
{
	uint8_t *s,*stemp,str[1]="0",*s1,*s2;
	uint32_t lvoltage; //测到的电压值	
	uint8_t ui;
	double percent ;       //进度条百分比
	uint16_t percentColor;//游动光标的颜色 文字内容是空
	uint16_t backColor;   //背景颜色
	uint16_t nPercent;    //游动光标的x轴位置

	uint8_t nPercentWS2821 = 0;
	double lToleranceUpperLimit,lToleranceLowerLimit,lTUpperLimitWarning,lTLowerLimitWarning,ltemp1,ltemp2;
	
	//页面第一次加载，需要清屏，加载无需实时更新的静态文字
	if(ScreenPageInfo.uPageReLoadFlag == MEASURE_PAGE)
	{
		Lcd_Clear(BGCOLOR);									//清屏
		//加载静态页面
		Gui_DrawFont_GBK16(135,5,TEXTCOLOR,BGCOLOR,"mm");
		Gui_DrawFont_GBK16(127,23,TEXTCOLOR,BGCOLOR,"[");
		Gui_DrawFont_GBK16(135,23,RED,BGCOLOR,"NO");   		//红色的字体
		Gui_DrawFont_GBK16(151,23,TEXTCOLOR,BGCOLOR,"]");
		Gui_DrawFont_GBK16(20,50,TEXTCOLOR,BGCOLOR,"MAX:");
		Gui_DrawFont_GBK16(130,50,TEXTCOLOR,BGCOLOR,"mm");
		Gui_DrawFont_GBK16(20,75,TEXTCOLOR,BGCOLOR,"MIN:");
		Gui_DrawFont_GBK16(130,75,TEXTCOLOR,BGCOLOR,"mm");
			
		//预加载页面数据
		s = _Double2Char(0,8); 										//转字符串
		Gui_DrawFont_GBK24(15,12,TEXTCOLOR,BGCOLOR,s);
		free(s);
		s = _Double2Char(MeasureData.lMaxDeta,8);   				//转字符串
		Gui_DrawFont_GBK16(55,50,TEXTCOLOR,BGCOLOR,s);
		free(s);
		s = _Double2Char(MeasureData.lMinDeta,8);   				//转字符串
		Gui_DrawFont_GBK16(55,75,TEXTCOLOR,BGCOLOR,s);
		free(s);

		Gui_DrawFont_GBK16(0,112,TEXTCOLOR,RED,"         NO         ");//21个

		tempBackColor = RED;
		
		ScreenPageInfo.uPageReLoadFlag = 0;					//清除重新加载标志位
		MeasureData.lTempDeta = 0;
//		RGB_LED_Reset();
//		RGB_LED_Red(); //红色

		if(redflag)
		{
			redflag = 0;
			HAL_TIM_Base_Stop_IT(&htim2);  //关闭定时器中断
			RGB_LED_Reset();
			RGB_LED_Red(); //红色
			HAL_TIM_Base_Start_IT(&htim2);  //开启定时器中断
		}

//		RGB_LED_Reset();
//		HAL_TIM_Base_Stop_IT(&htim2);  //关闭定时器中断
//		RGB_LED_Red(); //红色
//		HAL_TIM_Base_Start_IT(&htim2);  //开启定时器中断
		
	}
	else	//页面后续加载，只需要加载实时更新的值
	{
		//RGB_LED_Write0();
	
		if(MeasureData.uMeasureFlag) //adc 采集时间到了
		{
		    lvoltage = getMeasureVoltage(); //转换为电压返回 ----->后续经过计算，转换为测量结果显示数据
			#ifndef TEST
			MeasureData.lDeta  = CalDetaValue(lvoltage);  	//LCD屏显示当前电压数据
			//printf("voltage = %d mv\r\n ",lvoltage);
			//printf("output mater = %f mm\r\n ",MeasureData.lDeta);
			#endif
			MeasureData.uMeasureFlag = 0;
			//choicelight(1,0,0);
		}
		//判断数值是否被更新，有更新则加载到页面
		if((MeasureData.lDeta != MeasureData.lTempDeta)&&(MeasureData.lDeta<=9999.9999))
		{
			printf("数据有更新!\r\n");
			printf("voltage = %d mv\r\n ",lvoltage);
			printf("output mater = %f mm\r\n ",MeasureData.lDeta);
			
			stemp= _Double2Char(MeasureData.lTempDeta,8); 	//转字符串
			s = _Double2Char(MeasureData.lDeta,8); 			//转字符串
			//大数字显示优化
			s1 = s;
			s2 = stemp;
			for(ui=0;ui<9;ui++)
			{
				if(ui!=4) //跳过小数点.
				{
					if(*s1 != *s2)
					{
						str[0] = *s1;
						Gui_DrawFont_GBK24(15+ui*12,12,TEXTCOLOR,BGCOLOR,str);
					}
				}
				s1++;
				s2++;
			}
			free(s);
			free(stemp);
			free(s1);
			free(s2);


			//测头没有输入气压的时候 还是会
			if((MeasureData.lDeta>SettingData.lNominalSize - MAX_DEVIATION_VALUE) //最大的偏移值
				&&(MeasureData.lDeta<SettingData.lNominalSize + MAX_DEVIATION_VALUE))	
			{	
				printf("检测到测头插入!\r\n");
				//printf("voltage = %d mv\r\n ",lvoltage);
				//printf("output mater = %f mm\r\n ",MeasureData.lDeta);
				MeasureData.lMaxDeta = MeasureData.lDeta > MeasureData.lMaxDeta?MeasureData.lDeta:MeasureData.lMaxDeta;
				MeasureData.lMinDeta = MeasureData.lDeta > MeasureData.lMinDeta?MeasureData.lMinDeta:MeasureData.lDeta;
							
				if(MeasureData.lMaxDeta != MeasureData.lTempMaxDeta)//显示最大值
				{
					s = _Double2Char(MeasureData.lMaxDeta,8); 			//转字符串
					Gui_DrawFont_GBK16(55,50,TEXTCOLOR,BGCOLOR,s);
					free(s);
				}
				if(MeasureData.lMinDeta != MeasureData.lTempMinDeta)//显示最小值
				{
					s = _Double2Char(MeasureData.lMinDeta,8); 			//转字符串
					Gui_DrawFont_GBK16(55,75,TEXTCOLOR,BGCOLOR,s);
					free(s);
				}
				if(SettingData.uToleranceUpperLimitSymbol == 1) //如果发现超过 超差上限  
				{
					lToleranceUpperLimit = -SettingData.lToleranceUpperLimit*0.001;// -
					lTUpperLimitWarning = -SettingData.lTUpperLimitWarning*0.001;
				}
				else  
				{
					lToleranceUpperLimit = SettingData.lToleranceUpperLimit*0.001;
					lTUpperLimitWarning = SettingData.lTUpperLimitWarning*0.001;
				}
				if(SettingData.uToleranceLowerLimitSymbol == 1)  
				{
					lToleranceLowerLimit = -SettingData.lToleranceLowerLimit*0.001;// -
					lTLowerLimitWarning = -SettingData.lTLowerLimitWarning*0.001;
				}
				else  
				{
					lToleranceLowerLimit = SettingData.lToleranceLowerLimit*0.001;
					lTLowerLimitWarning = SettingData.lTLowerLimitWarning*0.001;
				}

				//当前测量值 小于等于 名义公差值+超差上限 并且 当前测量值 大于等于 名义公差值+超差下限
				if((MeasureData.lDeta <= SettingData.lNominalSize + lToleranceUpperLimit)&&(MeasureData.lDeta >= SettingData.lNominalSize + lToleranceLowerLimit))
				{
					printf("工件合格!\r\n");

					backColor = GRAY0;//进度条变灰吗？
					//计算进度条的百分比
					percent = (MeasureData.lDeta -(SettingData.lNominalSize + lToleranceLowerLimit) ) / ((SettingData.lNominalSize + lToleranceUpperLimit) - (SettingData.lNominalSize + lToleranceLowerLimit));

				#if 0
					//预警橙色功能
					if((MeasureData.lDeta>SettingData.lNominalSize+lTUpperLimitWarning)||(MeasureData.lDeta<SettingData.lNominalSize+lTLowerLimitWarning))
					{
						percentColor = ORAGNE; //进度条颜色 预警
					}
					else
					{
						percentColor = GREEN;//进度条颜色 正常
					}
				#else
					//三挡显示
					if(percent <= 0.33) percentColor = GREEN;
					else if(0.33<percent<=0.66) percentColor = BLUE;
					else if(0.66<percent<=1) percentColor = ORAGNE;					
				#endif

					//设置右上角ok 根据perent显示不同颜色
					Gui_DrawFont_GBK16(135,23,percentColor,BGCOLOR,"OK");
					//进度条1显示优化
//					if(backColor == tempBackColor)
//					{
//						if(percentColor == tempPercentColor)
//						{
//							if(percent > tempPercent)
//							{
//								for(ui=tempPercent*21;ui<percent*21;ui++)
//								Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,percentColor," ");//个数
//							}
//							if(percent < tempPercent)
//							{
//								for(ui=percent*21;ui<tempPercent*21;ui++)
//								Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,backColor," ");//个数
//							}
//						}
//						else
//						{
//							for(ui=0;ui<percent*21;ui++)
//							Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,percentColor," ");//个数
//							for(ui=percent*21;ui<21;ui++)
//							Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,backColor," ");//个数
//						}
//					}
//					else
//					{					
//						for(ui=0;ui<percent*21;ui++)
//						Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,percentColor," ");//个数
//						for(ui=percent*21;ui<21;ui++)
//						Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,backColor," ");//个数
//					}

					nPercent = percent*160;	 //小光标的位置
					nPercentWS2821 = percent*20;//灯条的位置

					HAL_TIM_Base_Stop_IT(&htim2);  //关闭定时器中断
					//RGB_LED_Reset();
					//RGB_LED_Red(); //红色
					choicelight(nPercentWS2821,0,0); //执行小光标的颜色变化
					HAL_TIM_Base_Start_IT(&htim2);  //开启定时器中断
					redflag = 1;//红灯拉起来

					//进度条2显示优化
					if(backColor == tempBackColor) //之前的颜色没有变 
					{
						Gui_DrawFont_GBK16(ntempPercent,112,TEXTCOLOR,backColor," ");//只变1个
						Gui_DrawFont_GBK16(nPercent,112,TEXTCOLOR,percentColor," ");//第几个格子变色
					}
					else //颜色变了   发生颜色变化
					{
						Gui_DrawFont_GBK16(0,112,TEXTCOLOR,backColor,"                     ");//全变色
						Gui_DrawFont_GBK16(nPercent,112,TEXTCOLOR,percentColor," ");//第几个格子变色
					}

					tempPercentColor = percentColor;//暂存进度条颜色
					ntempPercent = nPercent; //暂存第几个格子变色
				}
				else //如果工件不合格
				{
					printf("工件不合格!\r\n");
					backColor = RED; //背景色是红色 
					Gui_DrawFont_GBK16(135,23,backColor,BGCOLOR,"NO"); //这里设置背景色
					Gui_DrawFont_GBK16(0,112,TEXTCOLOR,backColor,"         NO          ");//21 这里写字      112

//					RGB_LED_Reset();
//					HAL_TIM_Base_Stop_IT(&htim2);  //关闭定时器中断
//					RGB_LED_Red(); //红色
//					HAL_TIM_Base_Start_IT(&htim2);  //开启定时器中断
//
//					if(backColor != tempBackColor) //如果颜色发生变化
//					{
//						HAL_TIM_Base_Stop_IT(&htim2);  //关闭定时器中断
//						RGB_LED_Reset();
//						RGB_LED_Red(); //红色
//						HAL_TIM_Base_Start_IT(&htim2);  //开启定时器中断
//					}
					if(redflag)
					{
						redflag = 0;
						HAL_TIM_Base_Stop_IT(&htim2);  //关闭定时器中断
						RGB_LED_Reset();
						RGB_LED_Red(); //红色
						HAL_TIM_Base_Start_IT(&htim2);  //开启定时器中断
					}

				}
				tempBackColor = backColor;//缓存上一次的背景颜色	
				MeasureData.lTempMaxDeta = MeasureData.lMaxDeta;
				MeasureData.lTempMinDeta = MeasureData.lMinDeta;
				MeasureData.lTempDeta = MeasureData.lDeta;	//缓存上一次的数据	
			}
			else 
			{
				printf("检测到测头拔出!\r\n");
				//RGB_LED_Red(); //红色
				backColor = RED;
				Gui_DrawFont_GBK16(0,112,TEXTCOLOR,backColor,"         ERR         ");//21个
			}
			//这里有两种  一种是测头拔出来 一种是测头没有接气压
		}

	}
}

//---------------------------------------------------------
// screen展示菜单页面
//---------------------------------------------------------
void ShowMenuPage(void)
{
	//页面第一次加载，需要清屏，加载无需实时更新的静态文字
	if(ScreenPageInfo.uPageReLoadFlag == MENU_PAGE)			
	{
		//加载静态文字
		Lcd_Clear(BGCOLOR);										//清屏
		Gui_DrawFont_GBK24(2,25,TEXTCOLOR,BGCOLOR,"测量");
		Gui_DrawFont_GBK24(2,85,TEXTCOLOR,BGCOLOR,"设置");
		Gui_DrawFont_GBK24(108,25,TEXTCOLOR,BGCOLOR,"校正");
		Gui_DrawFont_GBK24(108,85,TEXTCOLOR,BGCOLOR,"查询");
		ScreenPageInfo.uPageReLoadFlag = 0;						//清除重新加载标志位
	}
	else
	{
		//根据鼠标位置，加载和释放鼠标位置背景色
		if(PageCursor.uMenuPage_CurCursor != PageCursor.uMenuPage_LastCursor)
		{
			switch(PageCursor.uMenuPage_CurCursor)
			{
				case MEASURE_POINTER:
					Gui_DrawFont_GBK24(2,25,TEXTCOLOR,TEXTBGCOLOR,"测量");
				    Gui_DrawFont_GBK24(50,25,TEXTCOLOR,BGCOLOR,"←");
					break;
				case SETTING_POINTER:
					Gui_DrawFont_GBK24(2,85,TEXTCOLOR,TEXTBGCOLOR,"设置");
					Gui_DrawFont_GBK24(50,85,TEXTCOLOR,BGCOLOR,"←");
					break;

				case CORRECT_POINTER:
					Gui_DrawFont_GBK24(108,25,TEXTCOLOR,TEXTBGCOLOR,"校正");
					Gui_DrawFont_GBK24(82,25,TEXTCOLOR,BGCOLOR,"→");
					break;
				case SEARCH_POINTER:
					Gui_DrawFont_GBK24(108,85,TEXTCOLOR,TEXTBGCOLOR,"查询");
					Gui_DrawFont_GBK24(82,85,TEXTCOLOR,BGCOLOR,"→");	
					break;
				default:
					break;
			}
			switch(PageCursor.uMenuPage_LastCursor)
			{
				case MEASURE_POINTER:
					Gui_DrawFont_GBK24(2,25,TEXTCOLOR,BGCOLOR,"测量");
					Gui_DrawFont_GBK24(50,25,BGCOLOR,BGCOLOR,"←");
					break;
				case SETTING_POINTER:
					Gui_DrawFont_GBK24(2,85,TEXTCOLOR,BGCOLOR,"设置");
					Gui_DrawFont_GBK24(50,85,BGCOLOR,BGCOLOR,"←");
					break;
				case CORRECT_POINTER:
					Gui_DrawFont_GBK24(108,25,TEXTCOLOR,BGCOLOR,"校正");
					Gui_DrawFont_GBK24(82,25,BGCOLOR,BGCOLOR,"→");
					break;
				case SEARCH_POINTER:
					Gui_DrawFont_GBK24(108,85,TEXTCOLOR,BGCOLOR,"查询");
					Gui_DrawFont_GBK24(82,85,BGCOLOR,BGCOLOR,"→");
					break;
				default:
					break;
			}
			PageCursor.uMenuPage_LastCursor = PageCursor.uMenuPage_CurCursor;   //记录鼠标当前位置
		}
	}
	

}



//---------------------------------------------------------
// 功能：screen展示设置页面1
//---------------------------------------------------------
void ShowSettingPage1(void)
{
	uint8_t *s;
	
	//页面第一次加载，需要清屏，加载无需实时更新静态文字
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_1)  //第一个设置界面
	{	
		//加载静态页面
		Lcd_Clear(BGCOLOR);									 //清屏					
		Gui_DrawFont_GBK16(35,50,TEXTCOLOR,BGCOLOR,"站号选择:"); 
		
		//数据预处理
		SettingData.lTempProgramNo = SettingData.lProgramNo; //拷贝设置值 用于临时修改
		SettingData.lTempStationNo = SettingData.lStationNo; //拷贝设置值 用于临时修改
		PageCursor.uSelected = 1;							 //选择设置为第一项
		ScreenPageInfo.uPageReLoadFlag = 0;					 //清除重新加载标志位
	}
	else //页面后续加载，只需要加载实时更新的值
	{
		//根据选择设置项 加载闪烁效果
		if(PageCursor.uSelected == 1)
		{	
			s = Int2Char(SettingData.lTempStationNo);		 //站号转字符串
			Gui_BlinkShow(115,50,s); 						 //闪烁显示站号
			free(s);
		}
		else if(PageCursor.uSelected == 2)					 //新的确认键按下后
		{
			if(PageCursor.uNewSlected)						 //固定新的静态页面
			{
				s = Int2Char(SettingData.lStationNo);		 //固定站号的显示
				Gui_DrawFont_GBK16(115,50,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;					 //清除标志
				MeasureData.lMaxDeta = 0;
				MeasureData.lMinDeta = 9999.9999;
			}
		}

	}
}

//---------------------------------------------------------
// screen展示设置页面2
//---------------------------------------------------------
void ShowSettingPage2(void)
{
	uint8_t *s;
	
	//页面第一次加载，需要清屏，加载无需实时更新静态文字
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_2)
	{
		//加载静态页面
		Lcd_Clear(BGCOLOR);											//清屏		
		Gui_DrawFont_GBK16(30,40,TEXTCOLOR,BGCOLOR,"公称尺寸");
		Gui_DrawFont_GBK16(120,40,TEXTCOLOR,BGCOLOR,"mm");
		//数据预处理
		Double2StrArr(SettingData.lNominalSize,SettingData.nNominalSizeCharArr,8); //将公称尺寸设置值每一数字位转为字符串数组
		SettingData.lTempNominalSize = SettingData.lNominalSize; 	//拷贝设置值 用于临时修改
		PageCursor.uSelected = 1;									//选择设置为第一项
		PageCursor.uEditPos = 0;									//数字编辑位置默认为最低位
		ScreenPageInfo.uPageReLoadFlag = 0;							//清除重新加载标志位
		PageCursor.uNewSlected = 1;
	}
	else //页面后续加载，只需要加载实时更新的值
	{
		//根据选择设置项 加载闪烁效果
		if(PageCursor.uSelected == 1) 							//新的确认键按下后
		{
			if(PageCursor.uNewSlected)								//固定新的静态页面
			{				
				s = _Double2Char(SettingData.lTempNominalSize,8);	//显示临时的公称尺寸
				Gui_DrawFont_GBK16(50,70,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;
			}
			if(PageCursor.uEditPosChangeFlag)						//左右按键按下 编辑位置改变 则正常显示上一次编辑位置的数字
			{
				if(PageCursor.uLastEditPos == 0)Gui_DrawFont_GBK16(114,70,TEXTCOLOR,BGCOLOR,SettingData.nNominalSizeCharArr[0]);
				else if(PageCursor.uLastEditPos == 1)Gui_DrawFont_GBK16(106,70,TEXTCOLOR,BGCOLOR,SettingData.nNominalSizeCharArr[1]);
				else if(PageCursor.uLastEditPos == 2)Gui_DrawFont_GBK16(98,70,TEXTCOLOR,BGCOLOR,SettingData.nNominalSizeCharArr[2]);
				else if(PageCursor.uLastEditPos == 3)Gui_DrawFont_GBK16(90,70,TEXTCOLOR,BGCOLOR,SettingData.nNominalSizeCharArr[3]);
				else if(PageCursor.uLastEditPos == 4)Gui_DrawFont_GBK16(74,70,TEXTCOLOR,BGCOLOR,SettingData.nNominalSizeCharArr[4]);
				else if(PageCursor.uLastEditPos == 5)Gui_DrawFont_GBK16(66,70,TEXTCOLOR,BGCOLOR,SettingData.nNominalSizeCharArr[5]);
				else if(PageCursor.uLastEditPos == 6)Gui_DrawFont_GBK16(58,70,TEXTCOLOR,BGCOLOR,SettingData.nNominalSizeCharArr[6]);
				else if(PageCursor.uLastEditPos == 7)Gui_DrawFont_GBK16(50,70,TEXTCOLOR,BGCOLOR,SettingData.nNominalSizeCharArr[7]);
				PageCursor.uEditPosChangeFlag = 0;
			}
			//根据编辑位置 闪烁显示当前位置的数字
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(114,70,SettingData.nNominalSizeCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(106,70,SettingData.nNominalSizeCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(98,70,SettingData.nNominalSizeCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(90,70,SettingData.nNominalSizeCharArr[3]);
			else if(PageCursor.uEditPos == 4)Gui_BlinkShow(74,70,SettingData.nNominalSizeCharArr[4]);
			else if(PageCursor.uEditPos == 5)Gui_BlinkShow(66,70,SettingData.nNominalSizeCharArr[5]);
			else if(PageCursor.uEditPos == 6)Gui_BlinkShow(58,70,SettingData.nNominalSizeCharArr[6]);
			else if(PageCursor.uEditPos == 7)Gui_BlinkShow(50,70,SettingData.nNominalSizeCharArr[7]);
		}
		else if(PageCursor.uSelected == 2)							//新的确认键按下后
		{
			if(PageCursor.uNewSlected)								//固定新的静态页面
			{
				s = _Double2Char(SettingData.lNominalSize,8);		//将公称尺寸转为字符串
				Gui_DrawFont_GBK16(50,70,TEXTCOLOR,BGCOLOR,s);		//固定公称尺寸的显示
				PageCursor.uNewSlected = 0;
				free(s);
			}
		}
	}
}

//---------------------------------------------------------
// screen展示设置页面3
//---------------------------------------------------------
void ShowSettingPage3(void)
{
	uint8_t *s;
	
	//页面第一次加载，需要清屏，加载无需实时更新静态文字
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_3)
	{
		//加载静态文字
		Lcd_Clear(BGCOLOR);															//清屏
		Gui_DrawFont_GBK16(30,0,TEXTCOLOR,BGCOLOR,"超差上限 μm");		
		if(SettingData.uToleranceUpperLimitSymbol)Gui_DrawFont_GBK16(55,16,TEXTCOLOR,BGCOLOR,"-");
		else Gui_DrawFont_GBK16(55,16,TEXTCOLOR,BGCOLOR,"+");
		s = _Double2Char(SettingData.lToleranceUpperLimit,4);						//将超差上限设置值转为字符串
		Gui_DrawFont_GBK16(65,16,TEXTCOLOR,BGCOLOR,s);
		free(s);
		
		//将当前设置的超差上下限每一数字位拆分为字符串加载到Arr中
		Double2StrArr(SettingData.lToleranceUpperLimit,SettingData.uToleranceUpperLimitCharArr,4);
		Double2StrArr(SettingData.lToleranceLowerLimit,SettingData.uToleranceLowerLimitCharArr,4);
		Double2StrArr(SettingData.lTUpperLimitWarning,SettingData.uTUpperLimitWarningCharArr,4);
		Double2StrArr(SettingData.lTLowerLimitWarning,SettingData.uTLowerLimitWarningCharArr,4);
		
		//数据预处理
		SettingData.lTempToleranceUpperLimit = SettingData.lToleranceUpperLimit;	//拷贝设置值 用于临时修改
		SettingData.lTempToleranceLowerLimit = SettingData.lToleranceLowerLimit;    //拷贝设置值 用于临时修改
		SettingData.lTempTUpperLimitWarning = SettingData.lTUpperLimitWarning;	//拷贝设置值 用于临时修改
		SettingData.lTempTLowerLimitWarning = SettingData.lTLowerLimitWarning;    //拷贝设置值 用于临时修改
		ScreenPageInfo.uPageReLoadFlag = 0;											//清除重新加载标志位						
		PageCursor.uEditPos = 0;													//默认编辑位置为最低位
		PageCursor.uEditPosChangeFlag = 0 ;											//清除标记位置改变标志
		PageCursor.uSelected = 1;													//设置编辑第一项
	}
	else //页面后续加载，只需要加载实时更新的值
	{
		
		//根据选择设置项 加载闪烁效果
		if(PageCursor.uSelected == 1)  
		{	
			if(PageCursor.uEditPosChangeFlag)						//左右按键按下 编辑位置改变 则正常显示上一次编辑位置的数字
			{
				if(PageCursor.uLastEditPos == 0)Gui_DrawFont_GBK16(97,16,TEXTCOLOR,BGCOLOR,SettingData.uToleranceUpperLimitCharArr[0]);
				else if(PageCursor.uLastEditPos == 1)Gui_DrawFont_GBK16(81,16,TEXTCOLOR,BGCOLOR,SettingData.uToleranceUpperLimitCharArr[1]);
				else if(PageCursor.uLastEditPos == 2)Gui_DrawFont_GBK16(73,16,TEXTCOLOR,BGCOLOR,SettingData.uToleranceUpperLimitCharArr[2]);
				else if(PageCursor.uLastEditPos == 3)Gui_DrawFont_GBK16(65,16,TEXTCOLOR,BGCOLOR,SettingData.uToleranceUpperLimitCharArr[3]);
				else if(PageCursor.uLastEditPos == 4)
				{
					if(SettingData.uToleranceUpperLimitSymbol)Gui_DrawFont_GBK16(55,16,TEXTCOLOR,BGCOLOR,"-");
					else Gui_DrawFont_GBK16(55,16,TEXTCOLOR,BGCOLOR,"+");
				}
				PageCursor.uEditPosChangeFlag = 0;
			}
			//根据编辑位置 闪烁显示当前位置的数字
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(97,16,SettingData.uToleranceUpperLimitCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(81,16,SettingData.uToleranceUpperLimitCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(73,16,SettingData.uToleranceUpperLimitCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(65,16,SettingData.uToleranceUpperLimitCharArr[3]);
			else if(PageCursor.uEditPos == 4)
			{
				if(SettingData.uToleranceUpperLimitSymbol)Gui_BlinkShow(55,16,"-");
				else Gui_BlinkShow(55,16,"+");
			}
		}
		else if(PageCursor.uSelected == 2) 											//新的确认键按下后
		{
			if(PageCursor.uNewSlected)												//固定新的静态页面
			{	
				s = _Double2Char(SettingData.lTempToleranceUpperLimit,4);
				Gui_DrawFont_GBK16(65,16,TEXTCOLOR,BGCOLOR,s);						//固定超差上限设置值的显示					
				Gui_DrawFont_GBK16(30,32,TEXTCOLOR,BGCOLOR,"超差下限 μm");
				if(SettingData.uToleranceLowerLimitSymbol)Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"-");
				else Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"+");
				free(s);
				s = _Double2Char(SettingData.lTempToleranceLowerLimit,4);				//预显示超差下限设置值
				Gui_DrawFont_GBK16(65,48,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;											//标志清零
				PageCursor.uEditPos = 0;											//默认编辑位置为最低位		
			}
			if(PageCursor.uEditPosChangeFlag)						//左右按键按下 编辑位置改变 则正常显示上一次编辑位置的数字
			{
				if(PageCursor.uLastEditPos == 0)Gui_DrawFont_GBK16(97,48,TEXTCOLOR,BGCOLOR,SettingData.uToleranceLowerLimitCharArr[0]);
				else if(PageCursor.uLastEditPos == 1)Gui_DrawFont_GBK16(81,48,TEXTCOLOR,BGCOLOR,SettingData.uToleranceLowerLimitCharArr[1]);
				else if(PageCursor.uLastEditPos == 2)Gui_DrawFont_GBK16(73,48,TEXTCOLOR,BGCOLOR,SettingData.uToleranceLowerLimitCharArr[2]);
				else if(PageCursor.uLastEditPos == 3)Gui_DrawFont_GBK16(65,48,TEXTCOLOR,BGCOLOR,SettingData.uToleranceLowerLimitCharArr[3]);
				else if(PageCursor.uLastEditPos == 4)
				{
					if(SettingData.uToleranceLowerLimitSymbol)Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"-");
					else Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"+");
				}
				PageCursor.uEditPosChangeFlag = 0;
			}
			//根据编辑位置 闪烁显示当前位置的数字
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(97,48,SettingData.uToleranceLowerLimitCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(81,48,SettingData.uToleranceLowerLimitCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(73,48,SettingData.uToleranceLowerLimitCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(65,48,SettingData.uToleranceLowerLimitCharArr[3]);
			else if(PageCursor.uEditPos == 4)
			{
				if(SettingData.uToleranceLowerLimitSymbol)Gui_BlinkShow(55,48,"-");
				else Gui_BlinkShow(55,48,"+");
			}
		}
		else if(PageCursor.uSelected == 3)											//新的确认键按下后
		{
			if(PageCursor.uNewSlected)												//固定新的静态页面
			{
				s = _Double2Char(SettingData.lTempToleranceLowerLimit,4);				//固定超差下限设置值的显示	
				Gui_DrawFont_GBK16(65,48,TEXTCOLOR,BGCOLOR,s);	
				if(SettingData.uToleranceLowerLimitSymbol)Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"-");
				else Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"+");				
				free(s);
				Gui_DrawFont_GBK16(30,64,TEXTCOLOR,BGCOLOR,"预警上限 μm");
				if(SettingData.uToleranceUpperLimitSymbol)Gui_DrawFont_GBK16(55,80,TEXTCOLOR,BGCOLOR,"-");
				else Gui_DrawFont_GBK16(55,80,TEXTCOLOR,BGCOLOR,"+");
				free(s);
				s = _Double2Char(SettingData.lTempTUpperLimitWarning,4);				//预显示超差下限设置值
				Gui_DrawFont_GBK16(65,80,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;											//标志清零
				PageCursor.uEditPos = 0;											//默认编辑位置为最低位	
				PageCursor.uEditPosChangeFlag = 0 ;	
			}
			if(PageCursor.uEditPosChangeFlag)						//左右按键按下 编辑位置改变 则正常显示上一次编辑位置的数字
			{
				if(PageCursor.uLastEditPos == 0)Gui_DrawFont_GBK16(97,80,TEXTCOLOR,BGCOLOR,SettingData.uTUpperLimitWarningCharArr[0]);
				else if(PageCursor.uLastEditPos == 1)Gui_DrawFont_GBK16(81,80,TEXTCOLOR,BGCOLOR,SettingData.uTUpperLimitWarningCharArr[1]);
				else if(PageCursor.uLastEditPos == 2)Gui_DrawFont_GBK16(73,80,TEXTCOLOR,BGCOLOR,SettingData.uTUpperLimitWarningCharArr[2]);
				else if(PageCursor.uLastEditPos == 3)Gui_DrawFont_GBK16(65,80,TEXTCOLOR,BGCOLOR,SettingData.uTUpperLimitWarningCharArr[3]);
				PageCursor.uEditPosChangeFlag = 0;
			}
			//根据编辑位置 闪烁显示当前位置的数字
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(97,80,SettingData.uTUpperLimitWarningCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(81,80,SettingData.uTUpperLimitWarningCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(73,80,SettingData.uTUpperLimitWarningCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(65,80,SettingData.uTUpperLimitWarningCharArr[3]);
		}
		else if(PageCursor.uSelected == 4)											//新的确认键按下后
		{
			if(PageCursor.uNewSlected)												//固定新的静态页面
			{
				s = _Double2Char(SettingData.lTempTUpperLimitWarning,4);				//固定超差下限设置值的显示	
				Gui_DrawFont_GBK16(65,80,TEXTCOLOR,BGCOLOR,s);	
				Gui_DrawFont_GBK16(30,96,TEXTCOLOR,BGCOLOR,"预警下限 μm");
				if(SettingData.uToleranceLowerLimitSymbol)Gui_DrawFont_GBK16(55,112,TEXTCOLOR,BGCOLOR,"-");
				else Gui_DrawFont_GBK16(55,112,TEXTCOLOR,BGCOLOR,"+");
				free(s);
				s = _Double2Char(SettingData.lTempTLowerLimitWarning,4);				//预显示超差下限设置值
				Gui_DrawFont_GBK16(65,112,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;											//标志清零
				PageCursor.uEditPos = 0;											//默认编辑位置为最低位	
				PageCursor.uEditPosChangeFlag = 0 ;	
			}
			if(PageCursor.uEditPosChangeFlag)						//左右按键按下 编辑位置改变 则正常显示上一次编辑位置的数字
			{
				if(PageCursor.uLastEditPos == 0)Gui_DrawFont_GBK16(97,112,TEXTCOLOR,BGCOLOR,SettingData.uTLowerLimitWarningCharArr[0]);
				else if(PageCursor.uLastEditPos == 1)Gui_DrawFont_GBK16(81,112,TEXTCOLOR,BGCOLOR,SettingData.uTLowerLimitWarningCharArr[1]);
				else if(PageCursor.uLastEditPos == 2)Gui_DrawFont_GBK16(73,112,TEXTCOLOR,BGCOLOR,SettingData.uTLowerLimitWarningCharArr[2]);
				else if(PageCursor.uLastEditPos == 3)Gui_DrawFont_GBK16(65,112,TEXTCOLOR,BGCOLOR,SettingData.uTLowerLimitWarningCharArr[3]);
				PageCursor.uEditPosChangeFlag = 0;
			}
			//根据编辑位置 闪烁显示当前位置的数字
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(97,112,SettingData.uTLowerLimitWarningCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(81,112,SettingData.uTLowerLimitWarningCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(73,112,SettingData.uTLowerLimitWarningCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(65,112,SettingData.uTLowerLimitWarningCharArr[3]);
		}
		else if(PageCursor.uSelected == 5)											//新的确认键按下后
		{
			if(PageCursor.uNewSlected)												//固定新的静态页面
			{
				s = _Double2Char(SettingData.lTempTLowerLimitWarning,4);				//固定显示超差下限设置值
				Gui_DrawFont_GBK16(65,112,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;											//标志清零
				PageCursor.uEditPos = 0;											//默认编辑位置为最低位	
				PageCursor.uEditPosChangeFlag = 0 ;	
			}
		}
	}
}
//---------------------------------------------------------
// screen展示设置页面4
//---------------------------------------------------------
void ShowSettingPage4(void)
{
	uint8_t *s;
	//页面第一次加载，需要清屏，加载无需实时更新静态文字
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_4)
	{
		//加载静态页面
		Lcd_Clear(BGCOLOR);
		Gui_DrawFont_GBK16(20,20,TEXTCOLOR,BGCOLOR,"上限标准件值 mm");
		s = _Double2Char(SettingData.lStandardUpperLimit,8);								//标准件上限设置值转为字符串
		Gui_DrawFont_GBK16(50,45,TEXTCOLOR,BGCOLOR,s);
		free(s);
		
		//将当前设置的超差上下限每一位拆分为字符串加载到Arr中
		Double2StrArr(SettingData.lStandardUpperLimit,SettingData.uStandardUpperLimitCharArr,8);
		Double2StrArr(SettingData.lStandardLowerLimit,SettingData.uStandardLowerLimitCharArr,8);
		
		//数据预处理
		SettingData.lTempStandardUpperLimit = SettingData.lStandardUpperLimit;				//拷贝设置值 用于临时修改
		SettingData.lTempStandardLowerLimit = SettingData.lStandardLowerLimit;				//拷贝设置值 用于临时修改
		ScreenPageInfo.uPageReLoadFlag = 0;													//清除重新加载标志位
		PageCursor.uEditPos = 0;															//默认编辑位置为最低位
		PageCursor.uEditPosChangeFlag = 0 ;													//清除标记位置改变标志
		PageCursor.uSelected = 1;															//设置编辑第一项
	}
	else //页面后续加载，只需要加载实时更新的值
	{
		//根据选择设置项 加载闪烁效果
		if(PageCursor.uSelected == 1)  					
		{	
			if(PageCursor.uEditPosChangeFlag)			//左右按键按下 编辑位置改变 则正常显示上一次编辑位置的数字
			{
				if(PageCursor.uLastEditPos == 0)Gui_DrawFont_GBK16(114,45,TEXTCOLOR,BGCOLOR,SettingData.uStandardUpperLimitCharArr[0]);
				else if(PageCursor.uLastEditPos == 1)Gui_DrawFont_GBK16(106,45,TEXTCOLOR,BGCOLOR,SettingData.uStandardUpperLimitCharArr[1]);
				else if(PageCursor.uLastEditPos == 2)Gui_DrawFont_GBK16(98,45,TEXTCOLOR,BGCOLOR,SettingData.uStandardUpperLimitCharArr[2]);
				else if(PageCursor.uLastEditPos == 3)Gui_DrawFont_GBK16(90,45,TEXTCOLOR,BGCOLOR,SettingData.uStandardUpperLimitCharArr[3]);
				else if(PageCursor.uLastEditPos == 4)Gui_DrawFont_GBK16(74,45,TEXTCOLOR,BGCOLOR,SettingData.uStandardUpperLimitCharArr[4]);
				else if(PageCursor.uLastEditPos == 5)Gui_DrawFont_GBK16(66,45,TEXTCOLOR,BGCOLOR,SettingData.uStandardUpperLimitCharArr[5]);
				else if(PageCursor.uLastEditPos == 6)Gui_DrawFont_GBK16(58,45,TEXTCOLOR,BGCOLOR,SettingData.uStandardUpperLimitCharArr[6]);
				else if(PageCursor.uLastEditPos == 7)Gui_DrawFont_GBK16(50,45,TEXTCOLOR,BGCOLOR,SettingData.uStandardUpperLimitCharArr[7]);
				PageCursor.uEditPosChangeFlag = 0;
			}
			//根据编辑位置 闪烁显示当前位置的数字
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(114,45,SettingData.uStandardUpperLimitCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(106,45,SettingData.uStandardUpperLimitCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(98,45,SettingData.uStandardUpperLimitCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(90,45,SettingData.uStandardUpperLimitCharArr[3]);
			else if(PageCursor.uEditPos == 4)Gui_BlinkShow(74,45,SettingData.uStandardUpperLimitCharArr[4]);
			else if(PageCursor.uEditPos == 5)Gui_BlinkShow(66,45,SettingData.uStandardUpperLimitCharArr[5]);
			else if(PageCursor.uEditPos == 6)Gui_BlinkShow(58,45,SettingData.uStandardUpperLimitCharArr[6]);
			else if(PageCursor.uEditPos == 7)Gui_BlinkShow(50,45,SettingData.uStandardUpperLimitCharArr[7]);
		}
		else if(PageCursor.uSelected == 2) 												//新的确认键按下后
		{
			if(PageCursor.uNewSlected)													//固定新的静态页面
			{
				s = _Double2Char(SettingData.lTempStandardUpperLimit,8);					//标准件上限转字符串
				Gui_DrawFont_GBK16(50,45,TEXTCOLOR,BGCOLOR,s);									
				Gui_DrawFont_GBK16(20,70,TEXTCOLOR,BGCOLOR,"下限标准件值 mm");			
				free(s);
				s = _Double2Char(SettingData.lTempStandardLowerLimit,8);				//预显示标准件下限设置值
				Gui_DrawFont_GBK16(50,95,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;
				PageCursor.uEditPos = 0;
			}
			if(PageCursor.uEditPosChangeFlag)		 //左右按键按下 编辑位置改变 则正常显示上一次编辑位置的数字
			{
				if(PageCursor.uLastEditPos == 0)Gui_DrawFont_GBK16(114,95,TEXTCOLOR,BGCOLOR,SettingData.uStandardLowerLimitCharArr[0]);
				else if(PageCursor.uLastEditPos == 1)Gui_DrawFont_GBK16(106,95,TEXTCOLOR,BGCOLOR,SettingData.uStandardLowerLimitCharArr[1]);
				else if(PageCursor.uLastEditPos == 2)Gui_DrawFont_GBK16(98,95,TEXTCOLOR,BGCOLOR,SettingData.uStandardLowerLimitCharArr[2]);
				else if(PageCursor.uLastEditPos == 3)Gui_DrawFont_GBK16(90,95,TEXTCOLOR,BGCOLOR,SettingData.uStandardLowerLimitCharArr[3]);
				else if(PageCursor.uLastEditPos == 4)Gui_DrawFont_GBK16(74,95,TEXTCOLOR,BGCOLOR,SettingData.uStandardLowerLimitCharArr[4]);
				else if(PageCursor.uLastEditPos == 5)Gui_DrawFont_GBK16(66,95,TEXTCOLOR,BGCOLOR,SettingData.uStandardLowerLimitCharArr[5]);
				else if(PageCursor.uLastEditPos == 6)Gui_DrawFont_GBK16(58,95,TEXTCOLOR,BGCOLOR,SettingData.uStandardLowerLimitCharArr[6]);
				else if(PageCursor.uLastEditPos == 7)Gui_DrawFont_GBK16(50,95,TEXTCOLOR,BGCOLOR,SettingData.uStandardLowerLimitCharArr[7]);
				PageCursor.uEditPosChangeFlag = 0;
			}
			//根据编辑位置 闪烁显示当前位置的数字
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(114,95,SettingData.uStandardLowerLimitCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(106,95,SettingData.uStandardLowerLimitCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(98,95,SettingData.uStandardLowerLimitCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(90,95,SettingData.uStandardLowerLimitCharArr[3]);
			else if(PageCursor.uEditPos == 4)Gui_BlinkShow(74,95,SettingData.uStandardLowerLimitCharArr[4]);
			else if(PageCursor.uEditPos == 5)Gui_BlinkShow(66,95,SettingData.uStandardLowerLimitCharArr[5]);
			else if(PageCursor.uEditPos == 6)Gui_BlinkShow(58,95,SettingData.uStandardLowerLimitCharArr[6]);
			else if(PageCursor.uEditPos == 7)Gui_BlinkShow(50,95,SettingData.uStandardLowerLimitCharArr[7]);
		}
		else if(PageCursor.uSelected == 3)											//新的确认键按下后
		{
			if(PageCursor.uNewSlected)												//固定新的静态页面
			{
				s = _Double2Char(SettingData.lTempStandardLowerLimit,8);
				Gui_DrawFont_GBK16(50,95,TEXTCOLOR,BGCOLOR,s);						//固定标准件下限设置值的显示	
				PageCursor.uNewSlected = 0;
				free(s);
			}
		}
	}
}

//---------------------------------------------------------
// screen展示设置页面5
//---------------------------------------------------------
void ShowSettingPage5(void)
{
	
	//页面第一次加载，需要清屏，加载无需实时更新静态文字
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_5)
	{
		//加载静态页面
		Lcd_Clear(BGCOLOR);											//清屏
		Gui_DrawFont_GBK16(20,50,TEXTCOLOR,BGCOLOR,"测量模式:");
		//数据预处理
		ScreenPageInfo.uPageReLoadFlag = 0;
		SettingData.nTempMeasureMode = SettingData.lMeasureMode;
		if(SettingData.nTempMeasureMode == INNER_MEMODE)Gui_DrawFont_GBK16(95,50,TEXTCOLOR,BGCOLOR,"内尺寸");
		else if(SettingData.nTempMeasureMode == OUTER_MEMODE)Gui_DrawFont_GBK16(95,50,TEXTCOLOR,BGCOLOR,"外尺寸");
		PageCursor.uEditPosChangeFlag = 1;
	}
	else //页面后续加载，只需要加载实时更新的值
	{
		if(PageCursor.uEditPosChangeFlag)
		{
			//编辑响应，显示相应内容
			PageCursor.uEditPosChangeFlag = 0;
			if(SettingData.nTempMeasureMode == INNER_MEMODE)Gui_DrawFont_GBK16(95,50,TEXTCOLOR,BGCOLOR,"内尺寸");
			else if(SettingData.nTempMeasureMode == OUTER_MEMODE)Gui_DrawFont_GBK16(95,50,TEXTCOLOR,BGCOLOR,"外尺寸");
			
		}
	}
}





//---------------------------------------------------------
// screen展示校准页面
//---------------------------------------------------------
void ShowCorrectPage(void)
{	
	uint8_t *s;
	if(ScreenPageInfo.uPageReLoadFlag == CORRECT_PAGE)
	{
		Lcd_Clear(BGCOLOR);
		Gui_DrawFont_GBK16(45,20,TEXTCOLOR,BGCOLOR,"标准件校正");
		//TODO 
		//执行上限校正函数
		Gui_DrawFont_GBK16(10,90,TEXTCOLOR,BGCOLOR,"上限校正");
		ScreenPageInfo.uPageReLoadFlag = 0;
		PageCursor.uSelected = 0;
	}
	else
	{
		if(PageCursor.uSelected == 0)
		{
			//获取上限校正值
			if(MeasureData.uMeasureFlag)
			{
				CorrectValue.lCorrectUpperLimitValue = getMeasureVoltage();
				s = Double2Char(CorrectValue.lCorrectUpperLimitValue,8);
				Gui_DrawFont_GBK16(60,55,TEXTCOLOR,BGCOLOR,s);   //填入校正值
				free(s);
				MeasureData.uMeasureFlag = 0;
			}
		}
		else if(PageCursor.uSelected == 1)
		{
			if(PageCursor.uNewSlected == 1)
			{
				Gui_DrawFont_GBK16(130,90,TEXTCOLOR,BGCOLOR,"ok");
				PageCursor.uNewSlected = 0;
			}
		}
		else if(PageCursor.uSelected == 2)
		{
			//获取下限校正值
			if(MeasureData.uMeasureFlag)
			{
				CorrectValue.lCorrectLowerLimitValue = getMeasureVoltage();
				s = Double2Char(CorrectValue.lCorrectLowerLimitValue,8);
				Gui_DrawFont_GBK16(60,55,TEXTCOLOR,BGCOLOR,s);   //填入校正值
				free(s);
				MeasureData.uMeasureFlag = 0;
			}
			if(PageCursor.uNewSlected == 1)
			{
				Gui_DrawFont_GBK16(10,90,TEXTCOLOR,BGCOLOR,"下限校正");
				Gui_DrawFont_GBK16(130,90,BGCOLOR,BGCOLOR,"ok");       //隐藏OK
				PageCursor.uNewSlected = 0;
			}
		}
		else if(PageCursor.uSelected == 3)
		{
			if(PageCursor.uNewSlected == 1)
			{
				//TODO 
				//获取
				//执行计算K值函数
				uint8_t corrected = ValueCorrect(CorrectValue.lCorrectLowerLimitValue,SettingData.lStandardLowerLimit,CorrectValue.lCorrectUpperLimitValue,SettingData.lStandardUpperLimit);
				if(corrected)
				{
					s = Double2Char(CorrectValue.lKValue,6);
					if(SettingData.lMeasureMode == OUTER_MEMODE)
					{
						Gui_DrawFont_GBK16(82,90,TEXTCOLOR,BGCOLOR,"k=-"); //填入k值
						Gui_DrawFont_GBK16(110,90,TEXTCOLOR,BGCOLOR,s); //填入k值
					}
					else
					{
						Gui_DrawFont_GBK16(82,90,TEXTCOLOR,BGCOLOR,"k="); //填入k值
						Gui_DrawFont_GBK16(102,90,TEXTCOLOR,BGCOLOR,s); //填入k值	
					}
					free(s);
					PageCursor.uNewSlected = 0;
				}
				else
				{
					set_ScreenPageID(ERROR_PAGE);
					g_uErrorNo = ERROR_LIMIT;
				}
			}
		}
	}
}

//---------------------------------------------------------
// screen展示查询页面
//---------------------------------------------------------
void ShowSearchPage(void)
{
	
	if(ScreenPageInfo.uPageReLoadFlag == SEARCH_PAGE)
	{
		Lcd_Clear(BGCOLOR);
		Gui_DrawFont_GBK16(55,20,TEXTCOLOR,BGCOLOR,"数据查询");
		Gui_DrawFont_GBK16(20,55,TEXTCOLOR,BGCOLOR,"序号:");
		Gui_DrawFont_GBK16(60,55,TEXTCOLOR,BGCOLOR,"0001");
		Gui_DrawFont_GBK16(20,85,TEXTCOLOR,BGCOLOR,"负超");
		Gui_DrawFont_GBK16(60,85,TEXTCOLOR,BGCOLOR,"23.0317");
		Gui_DrawFont_GBK16(130,85,TEXTCOLOR,BGCOLOR,"mm");
		ScreenPageInfo.uPageReLoadFlag = 0;
	}
	else
	{

	}
}





//---------------------------------------------------------
// screen展示查询页面
//---------------------------------------------------------
void ShowErrorPage(void)
{
	
	if(ScreenPageInfo.uPageReLoadFlag == ERROR_PAGE)
	{
		if(g_uErrorNo == ERROR_LIMIT)
		{
			Lcd_Clear(BGCOLOR);
			Gui_DrawFont_GBK16(55,20,TEXTERRORCOLOR,BGCOLOR,"错误提示");
			Gui_DrawFont_GBK16(35,55,TEXTCOLOR,BGCOLOR,"注意上下限值");
		}
		else if(g_uErrorNo == ERROR_PARAM_1_NO_SET) //公差未设置
		{
			Lcd_Clear(BGCOLOR);
			Gui_DrawFont_GBK16(55,20,TEXTERRORCOLOR,BGCOLOR,"错误提示");
			Gui_DrawFont_GBK16(35,55,TEXTCOLOR,BGCOLOR,"设置公差尺寸");
		}
		
		ScreenPageInfo.uPageReLoadFlag = 0;
	}
	else
	{

	}
}

//---------------------------------------------------------
// 菜单显示页面处理函数
//---------------------------------------------------------
void MenuPage_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:										
			if((PageCursor.uMenuPage_CurCursor > MEASURE_POINTER) && (PageCursor.uMenuPage_CurCursor <= SEARCH_POINTER))PageCursor.uMenuPage_CurCursor--;//鼠标操作
			else if(PageCursor.uMenuPage_CurCursor == MEASURE_POINTER)PageCursor.uMenuPage_CurCursor = SEARCH_POINTER;
			break;
		case DOWN_PRESS:
			if((PageCursor.uMenuPage_CurCursor >= MEASURE_POINTER) && (PageCursor.uMenuPage_CurCursor < SEARCH_POINTER ))PageCursor.uMenuPage_CurCursor++;
			else if(PageCursor.uMenuPage_CurCursor == SEARCH_POINTER)PageCursor.uMenuPage_CurCursor = MEASURE_POINTER;
			break;
		case RIGHT_PRESS:
			if((PageCursor.uMenuPage_CurCursor >= MEASURE_POINTER) && (PageCursor.uMenuPage_CurCursor <= SEARCH_POINTER - 2))PageCursor.uMenuPage_CurCursor+=2;
			break;
		case LEFT_PRESS:
			if((PageCursor.uMenuPage_CurCursor >= MEASURE_POINTER + 2) && (PageCursor.uMenuPage_CurCursor <= SEARCH_POINTER ))PageCursor.uMenuPage_CurCursor-=2;
			break;
		case ENTER_PRESS:
			set_ScreenPageID(PageCursor.uMenuPage_CurCursor);	//根据鼠标指针所指位置进入下一个页面
			break;
		case ESC_PRESS:
			set_ScreenPageID(MEASURE_PAGE);    					//退出到测量显示页面
			PageCursor.uMenuPage_CurCursor = MEASURE_POINTER;  	//初始化鼠标位置
			break;
		default:
			break;
	}
}

//---------------------------------------------------------
// 设置页面1按键处理
//---------------------------------------------------------
void SettingPage1_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			if((PageCursor.uSelected == 1)&&(SettingData.lTempStationNo < 9))SettingData.lTempStationNo++;		//编辑第一行站号加一

			break;
		case DOWN_PRESS:
			if((PageCursor.uSelected == 1)&&(SettingData.lTempStationNo > 0) && (SettingData.lTempStationNo <= 9))SettingData.lTempStationNo--;		//编辑第一行站号减一
			break;
		case RIGHT_PRESS:
			break;
		case LEFT_PRESS:
			break;
		case ENTER_PRESS:
			if(PageCursor.uSelected == 1)
			{
				PageCursor.uSelected = 2 ;					//确认键按下，进入下一行编辑
				SettingData.lStationNo = SettingData.lTempStationNo; 
				InitSettingData(SettingData.lStationNo);	//根据站号 初始化所有设置参数
				FlashWrite_CurStationNo(SettingData.lStationNo); //2021.0924新增 更新Flash中保存的当前站号
				PageCursor.uNewSlected = 1;
			}
			else if(PageCursor.uSelected == 2)
			{
				//PageCursor.uNewSlected = 1;
				set_ScreenPageID(SETTING_PAGE_2);	//进入下一个编辑页面
			}

			break;
		case ESC_PRESS:
		    //2021.09.24新增
			if(SettingData.lNominalSize == 0) //没有设置
			{
				set_ScreenPageID(ERROR_PAGE);    		//退出到Error页面
				g_uErrorNo = ERROR_PARAM_1_NO_SET;
			}
			else
			{
				set_ScreenPageID(MENU_PAGE);    		//退出到测量显示页面
			}
			break;
		default:
			break;
	}
}


//---------------------------------------------------------
// 设置页面2按键处理
//---------------------------------------------------------
void SettingPage2_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			//数据减一
			if(PageCursor.uSelected == 1)
			{
				if((SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] = '0';
			}
			break;
		case DOWN_PRESS:
			//数据减一
			if(PageCursor.uSelected == 1)
			{
				if((SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0]--; 
				else SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] = '9'; 
			}
			break;
		case RIGHT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 7))//第二行编辑位置右移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case LEFT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos < 7)) //第二行编辑位置左移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case ENTER_PRESS:
			if(PageCursor.uSelected == 1)
			{
				PageCursor.uSelected = 2;
				SettingData.lNominalSize = StrArr2Double(SettingData.nNominalSizeCharArr,8); //保存编辑好的数据
				PageCursor.uNewSlected = 1;
				FlashUpdateSettingData(SettingData.lStationNo);
			}
			else if(PageCursor.uSelected == 2)
			{
				set_ScreenPageID(SETTING_PAGE_3);	//进入下一个编辑页面
			}
			break;
		case ESC_PRESS:
			set_ScreenPageID(SETTING_PAGE_1);    		//退出到设置页面1
			break;
		default:
			break;
	}
}


//---------------------------------------------------------
// 设置页面3按键处理
//---------------------------------------------------------
void SettingPage3_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			if(PageCursor.uSelected == 1 )				 //编辑第一行 数字加一
			{
				if(PageCursor.uEditPos==4)
				{
					if(SettingData.uToleranceUpperLimitSymbol)SettingData.uToleranceUpperLimitSymbol=0;
					else SettingData.uToleranceUpperLimitSymbol = 1;
				}
				else
				{
					if((SettingData.uToleranceUpperLimitCharArr[PageCursor.uEditPos][0] >= '0')
					&&(SettingData.uToleranceUpperLimitCharArr[PageCursor.uEditPos][0] < '9'))
					SettingData.uToleranceUpperLimitCharArr[PageCursor.uEditPos][0]++; 
					else SettingData.uToleranceUpperLimitCharArr[PageCursor.uEditPos][0] = '0'; 
				}
				
			}		//鼠标操作
			else if(PageCursor.uSelected == 2)			  //编辑第二行 数字加一
			{
				if(PageCursor.uEditPos==4)
				{
					if(SettingData.uToleranceLowerLimitSymbol)SettingData.uToleranceLowerLimitSymbol=0;
					else SettingData.uToleranceLowerLimitSymbol = 1;
				}
				else
				{
					if((SettingData.uToleranceLowerLimitCharArr[PageCursor.uEditPos][0] >= '0')
					&&(SettingData.uToleranceLowerLimitCharArr[PageCursor.uEditPos][0] < '9'))
					SettingData.uToleranceLowerLimitCharArr[PageCursor.uEditPos][0]++; 
					else SettingData.uToleranceLowerLimitCharArr[PageCursor.uEditPos][0]='0'; 
				}
			}
			else if(PageCursor.uSelected == 3)			  //编辑第3行 数字加一
			{
				if((SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0]='0'; 		
			}
			else if(PageCursor.uSelected == 4)			  //编辑第4行 数字加一
			{
				if((SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0]='0'; 		
			}
			break;
		case DOWN_PRESS:
			if(PageCursor.uSelected == 1)					//编辑第一行 数字减一
			{
				if(PageCursor.uEditPos==4)
				{
					if(SettingData.uToleranceUpperLimitSymbol)SettingData.uToleranceUpperLimitSymbol=0;
					else SettingData.uToleranceUpperLimitSymbol = 1;
				}
				else
				{
					if((SettingData.uToleranceUpperLimitCharArr[PageCursor.uEditPos][0] > '0')
					&&(SettingData.uToleranceUpperLimitCharArr[PageCursor.uEditPos][0] <= '9'))
					SettingData.uToleranceUpperLimitCharArr[PageCursor.uEditPos][0]--;
					else SettingData.uToleranceUpperLimitCharArr[PageCursor.uEditPos][0]='9';
				}
				
			}
			else if(PageCursor.uSelected == 2)				//编辑第二行 数字减一
			{
				if(PageCursor.uEditPos==4)
				{
					if(SettingData.uToleranceLowerLimitSymbol)SettingData.uToleranceLowerLimitSymbol=0;
					else SettingData.uToleranceLowerLimitSymbol = 1;
				}
				else
				{
					if((SettingData.uToleranceLowerLimitCharArr[PageCursor.uEditPos][0] > '0')
					&&(SettingData.uToleranceLowerLimitCharArr[PageCursor.uEditPos][0] <= '9'))
					SettingData.uToleranceLowerLimitCharArr[PageCursor.uEditPos][0]--; 
					else SettingData.uToleranceLowerLimitCharArr[PageCursor.uEditPos][0]='9'; 
				}
			}
			else if(PageCursor.uSelected == 3)				//编辑第3行 数字减一
			{

				if((SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0]--; 
				else SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0]='9'; 
			}
			else if(PageCursor.uSelected == 4)				//编辑第4行 数字减一
			{

				if((SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0]--; 
				else SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0]='9'; 
			}
			break;
		case RIGHT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 4))	//第一行编辑位置右移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 2)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 4))//第二行编辑位置右移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 3)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 3))//第二行编辑位置右移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 4)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 3))//第二行编辑位置右移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case LEFT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos < 4)) //第一行编辑位置左移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 2)&&(PageCursor.uEditPos < 4))//第二行编辑位置左移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 3)&&(PageCursor.uEditPos < 3))//第二行编辑位置左移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 4)&&(PageCursor.uEditPos < 3))//第二行编辑位置左移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case ENTER_PRESS:
			if(PageCursor.uSelected == 1)
			{
				PageCursor.uSelected = 2 ;				//确认键按下，进入下一行编辑
				SettingData.lTempToleranceUpperLimit = StrArr2Double(SettingData.uToleranceUpperLimitCharArr,4); //保存编辑好的数据
				SettingData.lToleranceUpperLimit = SettingData.lTempToleranceUpperLimit;
				FlashUpdateSettingData(SettingData.lStationNo);      //Flash更新存储值
				PageCursor.uNewSlected = 1;	
			}
			else if(PageCursor.uSelected == 2)
			{
				SettingData.lTempToleranceLowerLimit = StrArr2Double(SettingData.uToleranceLowerLimitCharArr,4); //保存编辑好的数据
				SettingData.lToleranceLowerLimit = SettingData.lTempToleranceLowerLimit;
				FlashUpdateSettingData(SettingData.lStationNo);
				PageCursor.uSelected = 3;
				PageCursor.uNewSlected = 1;
			}
			else if(PageCursor.uSelected == 3)
			{
				SettingData.lTempTUpperLimitWarning = StrArr2Double(SettingData.uTUpperLimitWarningCharArr,4); //保存编辑好的数据	
				SettingData.lTUpperLimitWarning = SettingData.lTempTUpperLimitWarning;
				FlashUpdateSettingData(SettingData.lStationNo);
				PageCursor.uSelected = 4;
				PageCursor.uNewSlected = 1;
			}
			else if(PageCursor.uSelected == 4)
			{
				SettingData.lTempTLowerLimitWarning = StrArr2Double(SettingData.uTLowerLimitWarningCharArr,4); //保存编辑好的数据	
				SettingData.lTLowerLimitWarning = SettingData.lTempTLowerLimitWarning;
				FlashUpdateSettingData(SettingData.lStationNo);
				PageCursor.uSelected = 5;
				PageCursor.uNewSlected = 1;
			}
			else if(PageCursor.uSelected == 5)
			{
				set_ScreenPageID(SETTING_PAGE_4);	//进入下一个设置页面
			}
			break;
		case ESC_PRESS:

//			else 
//			{
//				SettingData.lToleranceUpperLimit = SettingData.lTempToleranceUpperLimit;
//				SettingData.lToleranceLowerLimit = SettingData.lTempToleranceLowerLimit;
//				FlashUpdateSettingData(SettingData.lStationNo);
				set_ScreenPageID(SETTING_PAGE_2);    									//退出到测量显示页面
//			}
			break;
		default:
			break;
	}
}




//---------------------------------------------------------
// 设置页面4按键处理
//---------------------------------------------------------
void SettingPage4_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			if(PageCursor.uSelected == 1)			//编辑第一行 数字加一
			{
				if((SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0]='0'; 
			}		
			else if(PageCursor.uSelected == 2) 	//编辑第二行 数字加一
			{
				if((SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0]='0'; 
			}
			break;
		case DOWN_PRESS:
			if(PageCursor.uSelected == 1) 			//编辑第一行 数字减一
			{
				if((SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0]--;
				else SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0]='9';
			}
			else if(PageCursor.uSelected == 2)		//编辑第二行 数字减一
			{
				if((SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0]--; 
				else SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0]='9'; 
			}
			break;
		case RIGHT_PRESS:	
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 7))		//第一行编辑位置右移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 2)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 7))//第二行编辑位置右移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case LEFT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos < 7))			//第一行编辑位置左移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 2)&&(PageCursor.uEditPos < 7))		//第二行编辑位置左移
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case ENTER_PRESS:
			if(PageCursor.uSelected == 1)										//第一行编辑确认相应
			{
				PageCursor.uSelected = 2 ;										//进入下一行编辑
				SettingData.lTempStandardUpperLimit = StrArr2Double(SettingData.uStandardUpperLimitCharArr,8); //保存编辑好的数据
				PageCursor.uNewSlected = 1;
				//FlashUpdateSettingData(SettingData.lStationNo);					//更新Flash存储设置内容
			}
			else if(PageCursor.uSelected == 2)
			{
				SettingData.lTempStandardLowerLimit = StrArr2Double(SettingData.uStandardLowerLimitCharArr,8); //保存编辑好的数据
				if(SettingData.lTempStandardLowerLimit>=SettingData.lTempStandardUpperLimit)
				{
					set_ScreenPageID(ERROR_PAGE);
					g_uErrorNo = ERROR_LIMIT;
				}
				else 
				{
					PageCursor.uSelected = 3;
					PageCursor.uNewSlected = 1;
				}
			}
			else if(PageCursor.uSelected == 3)
			{
				SettingData.lStandardUpperLimit = SettingData.lTempStandardUpperLimit;
				SettingData.lStandardLowerLimit = SettingData.lTempStandardLowerLimit;
				FlashUpdateSettingData(SettingData.lStationNo);					//更新Flash存储设置内容
				set_ScreenPageID(SETTING_PAGE_5);	//根据鼠标指针所指位置进入下一个页面
			}
			break;
		case ESC_PRESS:
//			if(SettingData.lTempStandardLowerLimit>=SettingData.lTempStandardUpperLimit)set_ScreenPageID(ERROR_PAGE);
//			else 
//			{
				SettingData.lStandardUpperLimit = SettingData.lTempStandardUpperLimit;
				SettingData.lStandardLowerLimit = SettingData.lTempStandardLowerLimit;
				//退出到测量显示页面
				FlashUpdateSettingData(SettingData.lStationNo);					//更新Flash存储设置内容
				set_ScreenPageID(SETTING_PAGE_3);  
//			}
			break;
		default:
			break;
	}
}
//---------------------------------------------------------
// 设置页面5按键处理
//---------------------------------------------------------
void SettingPage5_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			//模式循环
			if(SettingData.nTempMeasureMode ==  INNER_MEMODE)
			{
				SettingData.nTempMeasureMode = OUTER_MEMODE;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if(SettingData.nTempMeasureMode == OUTER_MEMODE)
			{
				SettingData.nTempMeasureMode = INNER_MEMODE;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case DOWN_PRESS:
			//模式循环
			if(SettingData.nTempMeasureMode ==  INNER_MEMODE)
			{
				SettingData.nTempMeasureMode = OUTER_MEMODE;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if(SettingData.nTempMeasureMode == OUTER_MEMODE)
			{
				SettingData.nTempMeasureMode = INNER_MEMODE;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case RIGHT_PRESS:

			break;
		case LEFT_PRESS:

			break;
		case ENTER_PRESS:
			SettingData.lMeasureMode = SettingData.nTempMeasureMode;			//保存设置
			FlashUpdateSettingData(SettingData.lStationNo);						//更新Flash内容
			set_ScreenPageID(CORRECT_PAGE);    									//退出到矫正显示页面
			break;
		case ESC_PRESS:
			set_ScreenPageID(SETTING_PAGE_4);    									//退出到菜单显示页面
			break;
		default:
			break;
	}
}

//---------------------------------------------------------
// 校准页面按键处理
//---------------------------------------------------------
void CorrectPage_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	

			break;
		case DOWN_PRESS:

			break;
		case RIGHT_PRESS:

			break;
		case LEFT_PRESS:

			break;
		case ENTER_PRESS:
			
			if(PageCursor.uSelected < 3)
			{
				PageCursor.uSelected++;
				PageCursor.uNewSlected = 1;
			}
			else if(PageCursor.uSelected == 3)
			{
				FlashUpdateSettingData(SettingData.lStationNo);
				set_ScreenPageID(MEASURE_PAGE); 
				MeasureData.lMaxDeta = 0;
				MeasureData.lMinDeta = 9999.9999;
			}
			break;
		case ESC_PRESS:
			if(PageCursor.uSelected == 0 )set_ScreenPageID(MENU_PAGE);    									//退出到菜单显示页面
			break;
		default:
			break;
	}
}



//---------------------------------------------------------
// 设置屏幕ID
//---------------------------------------------------------
void set_ScreenPageID(uint8_t pageId)
{
	ScreenPageInfo.uPageReLoadFlag = pageId;
	ScreenPageInfo.uPageID = pageId;
	PageCursor.uMenuPage_LastCursor = 0;
}

//---------------------------------------------------------
// 获取屏幕ID
//---------------------------------------------------------
uint8_t get_ScreenPageID(void)
{
	return ScreenPageInfo.uPageID ; //返回屏幕ID
}


//---------------------------------------------------------
// 屏幕显示处理函数
//---------------------------------------------------------
void ScreenProcess(void)
{
	uint8_t uScreenPageID = get_ScreenPageID();		//获取页面ID
	uint8_t uPressKey = KeyMonitor();  				//按键监听
	//根据页面ID显示相应的页面，进行不同的按键处理响应
	switch(uScreenPageID)
	{
		//测量页面
		case MEASURE_PAGE:	 //默认是这个界面 1				
		{
			if(uPressKey == ESC_PRESS) 			
			{
				set_ScreenPageID(MENU_PAGE);	//进入菜单界面
			}
			else if(uPressKey == LEFT_PRESS)  //左键保存
			{
				//保存数据  100行 3列  一行就是一个数据点  TODO
				_SAVE_DATA[nCur_Save_No][SAVE_INDEX_REAL] = MeasureData.lDeta;
				_SAVE_DATA[nCur_Save_No][SAVE_INDEX_MAX] = MeasureData.lMaxDeta;
				_SAVE_DATA[nCur_Save_No][SAVE_INDEX_MIN] = MeasureData.lMinDeta;
				nCur_Save_No++; //开始下一个数据点的保存
			}
			else ShowMeasurePage();						//显示测量页面
		}	break;
		//菜单页面
		case MENU_PAGE:						
		{
			if(uPressKey != NULL_PRESS)
			{			
				MenuPage_KeyResponse(uPressKey);		//菜单页面按键响应处理
			}
			else ShowMenuPage();						//显示菜单页面
		}	break;	

		//设置页面1 站号和程序设置		
		case SETTING_PAGE_1:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage1_KeyResponse(uPressKey);	//设置页面1按键响应处理
			}
			else ShowSettingPage1();					//显示设置页面1
			
		}	break;	
		//设置页面2 量程和工程尺寸设置
		case SETTING_PAGE_2:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage2_KeyResponse(uPressKey);	//设置页面2按键响应处理
			}
			else ShowSettingPage2();					//显示设置页面2
			
		}	break;	
		//设置页面3 超差上下限设置
		case SETTING_PAGE_3:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;//先记下上一次的id号
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage3_KeyResponse(uPressKey);	//设置页面3按键响应处理
			}
			else ShowSettingPage3();					//显示设置页面3
			
		}	break;	
		//设置页面4 标准件上下限设置
		case SETTING_PAGE_4:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage4_KeyResponse(uPressKey);	//设置页面4按键响应处理
			}
			else ShowSettingPage4();					//显示设置页面4
			
		}	break;	
		//设置页面5 测量模式选择
		case SETTING_PAGE_5:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage5_KeyResponse(uPressKey);	//设置页面5按键响应处理
			}
			else ShowSettingPage5();					//显示设置页面5
			
		}	break;	
		//设置页面6 预警值选择
		case SETTING_PAGE_6:			
		{
//			ScreenPageInfo.uLastPageID = uScreenPageID;
//			if(uPressKey != NULL_PRESS)
//			{			
//				SettingPage5_KeyResponse(uPressKey);	//设置页面5按键响应处理
//			}
//			else ShowSettingPage5();					//显示设置页面5
			
		}	break;	
		//矫正页面
		case CORRECT_PAGE:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{
				CorrectPage_KeyResponse(uPressKey);		//矫正页面按键响应处理
			}
			else ShowCorrectPage();						//显示校正页面
			
		}	break;
		//查询页面		
		case SEARCH_PAGE:			
		{
			if(uPressKey == ESC_PRESS)
			{
				set_ScreenPageID(MENU_PAGE);			//查询页面按键响应处理
			}
			else ShowSearchPage();						//显示查询页面
			
		}	break;
		case ERROR_PAGE:
			if((uPressKey == ESC_PRESS)||(uPressKey == ENTER_PRESS))
			{
				set_ScreenPageID(ScreenPageInfo.uLastPageID);			//error页面按键响应处理
				g_uErrorNo = ERROR_RELEASE;
			}
			else ShowErrorPage();					//显示error页面
		default:
			break;
	}
}




