#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "main.h"
 /******************************************************************************
//本程序适用与STM32F103C8
//              GND   电源地
//              VCC   接5V或3.3v电源
//              SCL   接PA7（SCL）
//              SDA   接PA6（SDA）
//              RES   接PA5
//              DC    接PA4
//              CS    接PA3 
//				BL	  接PA2
*******************************************************************************/


//菜单页面指针
#define		MEASURE_POINTER     1	    //测量
#define		SETTING_POINTER     2	    //设置
#define		CORRECT_POINTER     3	    //校正
#define		SEARCH_POINTER      4		//查询


//页面ID编号
#define   	MENU_PAGE          10       //菜单页面
#define		MEASURE_PAGE        1       //测量页面 默认为测量页面
#define		CORRECT_PAGE        3	    //校正页面
#define		SEARCH_PAGE         4		//查询页面

#define     ERROR_PAGE          7
//设置页面
#define		SETTING_PAGE_1  	2	    //设置子页面1
#define		SETTING_PAGE_2  	22	    //设置子页面2
#define		SETTING_PAGE_3  	23	    //设置子页面3
#define		SETTING_PAGE_4  	24	    //设置子页面4
#define		SETTING_PAGE_5  	25	    //设置子页面5
#define		SETTING_PAGE_6  	26	    //设置子页面6

//#define     MEASURE_MODE_0		0
//#define     MEASURE_MODE_1		1	
//#define     MEASURE_MODE_2		2

//颜色定义
#define BGCOLOR  		 BLACK			//背景色     
#define TEXTBGCOLOR   	 BLUE           //文本背景色
#define TEXTCOLOR   	 WHITE     		//文本颜色
#define TEXTERRORCOLOR   RED

#define MAX_DEVIATION_VALUE    1  		//最大误差单位mm

//下面是使用spea015d时，接入3.3V电压，无气压时候的电压输出
#define NoPressurelvoltageUp 283 ; //无气压时电压上限 
#define NoPressurelvoltageDown 280 ; //无气压时电压上限 


#define MAX_PARAM_NUM      		    12    //公称尺寸+超差上下限+ 2个符号位+上下限标准件值+测量模式+上下校准值+预警上下限   
#define MAX_STATION_NUM    			10    //10种

#define MAX_SAVE_PARAM_NUM          3    //3列
#define MAX_SAVE_NUM				100  // 100行
#define SAVE_INDEX_REAL      		0    //datareal
#define SAVE_INDEX_MAX       		1    //maxdata
#define SAVE_INDEX_MIN       		2    //mindata


#define NOMISIZE_INDEX  			0    //公称
#define TOUPLIMIT_INDEX   			1    //超差上限
#define TOLOLIMIT_INDEX   			2    //超差下限
#define TOUPLIMITSYB_INDEX   		3    //超差上限符号 0代表+ 1代表-
#define TOLOLIMITSYB_INDEX   		4    //超差下限符号

#define STUPLIMIT_INDEX   			5    //标准件上限
#define STLOLIMIT_INDEX   			6    //标准件下限
#define MEMODE_INDEX   				7    //测量模式
#define CORRECTSTUPLIMIT_INDEX   	8    //上限校准值
#define CORRECTSTLOLIMIT_INDEX   	9    //下限校准值
#define MEASUREUPWARNING_INDEX   	10   //测量预警上限
#define MEASURELOWARNING_INDEX		11	 //测量预警下限

#define INNER_MEMODE   			0		 //测量模式内存尺寸
#define OUTER_MEMODE  			1 		 //测量模式外尺寸

//页面鼠标信息
typedef struct 
{
	uint8_t uMenuPage_CurCursor;		//菜单页面的当前鼠标位置
	uint8_t uPage_Cursor;
	uint8_t uMenuPage_LastCursor;		//菜单页面的鼠标上一次的位置
	uint8_t uSelected;					//选择修改
	uint8_t uNewSlected;				//选择改变
	uint8_t uBlinkFlag;					//闪烁标志
	uint8_t uEditPos;					//编辑位置
	uint8_t uLastEditPos;			    //上一次编辑位置
	uint8_t uEditPosChangeFlag;		    //位置改变标志位
}CURSORINFO;

//页面信息
typedef struct
{
	uint8_t uPageState;					//页面状态
	uint8_t uPageReLoadFlag;			//重新加载标志为  	  	 		是不是第一次加载
	uint8_t uPageID;					//页面ID
	uint8_t uLastPageID;				//上一次页面
}PAGEINFO;

//函数曲线
typedef struct
{
	uint32_t lCorrectUpperLimitValue;  //标准件上限校准值
	uint32_t lCorrectLowerLimitValue;  //标准件下限校准值
	double lKValue;				//K值 斜率
	double lBValue;         	//B值 截距
}CORRECTVALUEINFO;

//测量页面数据信息
typedef struct
{
	double lDeta; //工件的值
	double lMaxDeta;
	double lMinDeta;	
	uint32_t lNG;			
	uint32_t lRange;
	uint32_t lMeasureMode;	 //测量模式
	
	double lTempDeta;	
	double lTempMaxDeta;
	double lTempMinDeta;	
	uint32_t lTempNG;			
	uint32_t lTempRange; 
	uint32_t lTempMeasureMode;	

	uint8_t	 uMeasureFlag;
}MEASUREDATAINFO;

//设置页面信息
typedef struct
{
	uint32_t lStationNo;	 			//站号选择			 	
	uint32_t lProgramNo;	 			//程序选择		
	uint32_t lRangeNo;		 			//量程选择
	
	double lNominalSize;   				//公称尺寸				
	double lToleranceUpperLimit;		//超差上限			
	double lToleranceLowerLimit;  		//超差下限		
	
	double lStandardUpperLimit;			//标准件上限			
	double lMeUpWarning;   				//测量预警上限	
	
	double lStandardLowerLimit;   		//标准件下限	
	double lMeLoWarning;   				//测量预警下限

	uint32_t lMeasureMode;				//测量模式		
	
	uint32_t lTempStationNo;	 		//站号选择			 	
	uint32_t lTempProgramNo;	 		//程序选择		
	uint32_t lTempRangeNo;		 		//量程选择
	double lTempNominalSize;   		//公称尺寸				
	double lTempToleranceUpperLimit;	//超差上限			
	double lTempToleranceLowerLimit;  //超差下限		
	
	double lTUpperLimitWarning;		//超差上限预警			
	double lTLowerLimitWarning;  	//超差下限预警	
	
	double lTempTUpperLimitWarning;		//超差上限预警			
	double lTempTLowerLimitWarning;  	//超差下限预警	
	
	double lTempStandardUpperLimit;		//标准件上限			
	double lTempStandardLowerLimit;   //标准件下限	
	uint32_t nTempMeasureMode;			//测量模式	
	uint8_t  nNominalSizeCharArr[8][2];			//存放公称尺寸每一位char 最后一位为\0
	uint8_t  uToleranceUpperLimitCharArr[4][2];	//存放超差上限每一位char 最后一位为\0 
	uint8_t  uToleranceLowerLimitCharArr[4][2];	//存放超差下限每一位char 最后一位为\0
	uint8_t  uToleranceUpperLimitSymbol;		//正负符号
	uint8_t  uToleranceLowerLimitSymbol;		//正负符号
	
	uint8_t  uStandardUpperLimitCharArr[8][2];	//标准件上限值每一位char 最后一位为\0
	uint8_t  uStandardLowerLimitCharArr[8][2];	//标准件上限值每一位char 最后一位为\0
	
	uint8_t  uTUpperLimitWarningCharArr[4][2];	//超差上限预警值每一位char 最后一位为\0
	uint8_t  uTLowerLimitWarningCharArr[4][2];	//超差上限预警值每一位char 最后一位为\0
	uint8_t  uSettingChangeFlag;
}SETTINGDATAINFO;


void ShowErrorPage(void);
void ScreenInit(void);
void ScreenProcess(void);

void ShowMenuPage(void);
void ShowMeasurePage(void);
void ShowAdjustPage(void);

void ShowSettingPage1(void);
void ShowSettingPage2(void);
void ShowSettingPage3(void);
void ShowSettingPage4(void);
void ShowSettingPage5(void);

void ShowCorrectPage(void);
void ShowSearchPage(void);
void ShowExtrenNumPage(void);

void MenuPage_KeyResponse(uint8_t uPressKey);
void SettingPage1_KeyResponse(uint8_t uPressKey);
void SettingPage2_KeyResponse(uint8_t uPressKey);
void SettingPage3_KeyResponse(uint8_t uPressKey);
void SettingPage4_KeyResponse(uint8_t uPressKey);
void SettingPage5_KeyResponse(uint8_t uPressKey);

void Gui_BlinkShow(uint8_t ux,uint8_t uy,uint8_t *p);
void set_ScreenPageID(uint8_t pageId);
uint8_t get_ScreenPageID(void);


#endif

