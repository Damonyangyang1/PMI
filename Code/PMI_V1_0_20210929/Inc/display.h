#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "main.h"
 /******************************************************************************
//������������STM32F103C8
//              GND   ��Դ��
//              VCC   ��5V��3.3v��Դ
//              SCL   ��PA7��SCL��
//              SDA   ��PA6��SDA��
//              RES   ��PA5
//              DC    ��PA4
//              CS    ��PA3 
//				BL	  ��PA2
*******************************************************************************/


//�˵�ҳ��ָ��
#define		MEASURE_POINTER     1	    //����
#define		SETTING_POINTER     2	    //����
#define		CORRECT_POINTER     3	    //У��
#define		SEARCH_POINTER      4		//��ѯ


//ҳ��ID���
#define   	MENU_PAGE          10       //�˵�ҳ��
#define		MEASURE_PAGE        1       //����ҳ�� Ĭ��Ϊ����ҳ��
#define		CORRECT_PAGE        3	    //У��ҳ��
#define		SEARCH_PAGE         4		//��ѯҳ��

#define     ERROR_PAGE          7
//����ҳ��
#define		SETTING_PAGE_1  	2	    //������ҳ��1
#define		SETTING_PAGE_2  	22	    //������ҳ��2
#define		SETTING_PAGE_3  	23	    //������ҳ��3
#define		SETTING_PAGE_4  	24	    //������ҳ��4
#define		SETTING_PAGE_5  	25	    //������ҳ��5
#define		SETTING_PAGE_6  	26	    //������ҳ��6

//#define     MEASURE_MODE_0		0
//#define     MEASURE_MODE_1		1	
//#define     MEASURE_MODE_2		2

//��ɫ����
#define BGCOLOR  		 BLACK			//����ɫ     
#define TEXTBGCOLOR   	 BLUE           //�ı�����ɫ
#define TEXTCOLOR   	 WHITE     		//�ı���ɫ
#define TEXTERRORCOLOR   RED

#define MAX_DEVIATION_VALUE    1  		//�����λmm

//������ʹ��spea015dʱ������3.3V��ѹ������ѹʱ��ĵ�ѹ���
#define NoPressurelvoltageUp 283 ; //����ѹʱ��ѹ���� 
#define NoPressurelvoltageDown 280 ; //����ѹʱ��ѹ���� 


#define MAX_PARAM_NUM      		    12    //���Ƴߴ�+����������+ 2������λ+�����ޱ�׼��ֵ+����ģʽ+����У׼ֵ+Ԥ��������   
#define MAX_STATION_NUM    			10    //10��

#define MAX_SAVE_PARAM_NUM          3    //3��
#define MAX_SAVE_NUM				100  // 100��
#define SAVE_INDEX_REAL      		0    //datareal
#define SAVE_INDEX_MAX       		1    //maxdata
#define SAVE_INDEX_MIN       		2    //mindata


#define NOMISIZE_INDEX  			0    //����
#define TOUPLIMIT_INDEX   			1    //��������
#define TOLOLIMIT_INDEX   			2    //��������
#define TOUPLIMITSYB_INDEX   		3    //�������޷��� 0����+ 1����-
#define TOLOLIMITSYB_INDEX   		4    //�������޷���

#define STUPLIMIT_INDEX   			5    //��׼������
#define STLOLIMIT_INDEX   			6    //��׼������
#define MEMODE_INDEX   				7    //����ģʽ
#define CORRECTSTUPLIMIT_INDEX   	8    //����У׼ֵ
#define CORRECTSTLOLIMIT_INDEX   	9    //����У׼ֵ
#define MEASUREUPWARNING_INDEX   	10   //����Ԥ������
#define MEASURELOWARNING_INDEX		11	 //����Ԥ������

#define INNER_MEMODE   			0		 //����ģʽ�ڴ�ߴ�
#define OUTER_MEMODE  			1 		 //����ģʽ��ߴ�

//ҳ�������Ϣ
typedef struct 
{
	uint8_t uMenuPage_CurCursor;		//�˵�ҳ��ĵ�ǰ���λ��
	uint8_t uPage_Cursor;
	uint8_t uMenuPage_LastCursor;		//�˵�ҳ��������һ�ε�λ��
	uint8_t uSelected;					//ѡ���޸�
	uint8_t uNewSlected;				//ѡ��ı�
	uint8_t uBlinkFlag;					//��˸��־
	uint8_t uEditPos;					//�༭λ��
	uint8_t uLastEditPos;			    //��һ�α༭λ��
	uint8_t uEditPosChangeFlag;		    //λ�øı��־λ
}CURSORINFO;

//ҳ����Ϣ
typedef struct
{
	uint8_t uPageState;					//ҳ��״̬
	uint8_t uPageReLoadFlag;			//���¼��ر�־Ϊ  	  	 		�ǲ��ǵ�һ�μ���
	uint8_t uPageID;					//ҳ��ID
	uint8_t uLastPageID;				//��һ��ҳ��
}PAGEINFO;

//��������
typedef struct
{
	uint32_t lCorrectUpperLimitValue;  //��׼������У׼ֵ
	uint32_t lCorrectLowerLimitValue;  //��׼������У׼ֵ
	double lKValue;				//Kֵ б��
	double lBValue;         	//Bֵ �ؾ�
}CORRECTVALUEINFO;

//����ҳ��������Ϣ
typedef struct
{
	double lDeta; //������ֵ
	double lMaxDeta;
	double lMinDeta;	
	uint32_t lNG;			
	uint32_t lRange;
	uint32_t lMeasureMode;	 //����ģʽ
	
	double lTempDeta;	
	double lTempMaxDeta;
	double lTempMinDeta;	
	uint32_t lTempNG;			
	uint32_t lTempRange; 
	uint32_t lTempMeasureMode;	

	uint8_t	 uMeasureFlag;
}MEASUREDATAINFO;

//����ҳ����Ϣ
typedef struct
{
	uint32_t lStationNo;	 			//վ��ѡ��			 	
	uint32_t lProgramNo;	 			//����ѡ��		
	uint32_t lRangeNo;		 			//����ѡ��
	
	double lNominalSize;   				//���Ƴߴ�				
	double lToleranceUpperLimit;		//��������			
	double lToleranceLowerLimit;  		//��������		
	
	double lStandardUpperLimit;			//��׼������			
	double lMeUpWarning;   				//����Ԥ������	
	
	double lStandardLowerLimit;   		//��׼������	
	double lMeLoWarning;   				//����Ԥ������

	uint32_t lMeasureMode;				//����ģʽ		
	
	uint32_t lTempStationNo;	 		//վ��ѡ��			 	
	uint32_t lTempProgramNo;	 		//����ѡ��		
	uint32_t lTempRangeNo;		 		//����ѡ��
	double lTempNominalSize;   		//���Ƴߴ�				
	double lTempToleranceUpperLimit;	//��������			
	double lTempToleranceLowerLimit;  //��������		
	
	double lTUpperLimitWarning;		//��������Ԥ��			
	double lTLowerLimitWarning;  	//��������Ԥ��	
	
	double lTempTUpperLimitWarning;		//��������Ԥ��			
	double lTempTLowerLimitWarning;  	//��������Ԥ��	
	
	double lTempStandardUpperLimit;		//��׼������			
	double lTempStandardLowerLimit;   //��׼������	
	uint32_t nTempMeasureMode;			//����ģʽ	
	uint8_t  nNominalSizeCharArr[8][2];			//��Ź��Ƴߴ�ÿһλchar ���һλΪ\0
	uint8_t  uToleranceUpperLimitCharArr[4][2];	//��ų�������ÿһλchar ���һλΪ\0 
	uint8_t  uToleranceLowerLimitCharArr[4][2];	//��ų�������ÿһλchar ���һλΪ\0
	uint8_t  uToleranceUpperLimitSymbol;		//��������
	uint8_t  uToleranceLowerLimitSymbol;		//��������
	
	uint8_t  uStandardUpperLimitCharArr[8][2];	//��׼������ֵÿһλchar ���һλΪ\0
	uint8_t  uStandardLowerLimitCharArr[8][2];	//��׼������ֵÿһλchar ���һλΪ\0
	
	uint8_t  uTUpperLimitWarningCharArr[4][2];	//��������Ԥ��ֵÿһλchar ���һλΪ\0
	uint8_t  uTLowerLimitWarningCharArr[4][2];	//��������Ԥ��ֵÿһλchar ���һλΪ\0
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

