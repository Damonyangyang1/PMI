#include "display.h"
#include "stdlib.h"
#include "usart.h"

typedef enum
{
	ERROR_RELEASE,
	ERROR_LIMIT,  		  //�����޴���
	ERROR_PARAM_1_NO_SET, //����ߴ����δ����
}ENUM_ERROR_INFO;

CURSORINFO 		 PageCursor = {1,0,0};      					  	//ҳ�����
PAGEINFO 		 ScreenPageInfo = {0,MEASURE_PAGE,MEASURE_PAGE};	//��ʾҳ��
MEASUREDATAINFO  MeasureData = {0,0,0,0,0,0};			    	    //����ҳ��������Ϣ
SETTINGDATAINFO  SettingData = {0,0,0,0};//1507635       	  	    //����ҳ����Ϣ
CORRECTVALUEINFO CorrectValue;										//����������

uint32_t _SETTING_VALUE[MAX_STATION_NUM][MAX_PARAM_NUM];   	//10��12�� �������վ�������ֵ
uint32_t _SAVE_DATA[MAX_SAVE_NUM][MAX_SAVE_PARAM_NUM];     //100��3�� ������б���ֵ
uint16_t nCur_Save_No; //��ǰ��������ݵ�  ��Ӧ�����ÿһ��data 

uint8_t g_uErrorNo = ERROR_RELEASE;



//---------------------------------------------------------
// ���ܣ�Screen��ʼ����ӭҳ��
//---------------------------------------------------------
void ScreenInit(void)
{
	Lcd_Init();						//LCD��ʼ��
	FlashData_Init();				//Flash�洢���ݳ�ʼ��
	InitSettingData(SettingData.lStationNo);
	//�ϵ�У׼
	ValueCorrect(CorrectValue.lCorrectLowerLimitValue,SettingData.lStandardLowerLimit,CorrectValue.lCorrectUpperLimitValue,SettingData.lStandardUpperLimit); 					
	Lcd_Clear(BLACK);				//����
	Gui_DrawFont_GBK32(22,15,TEXTCOLOR,BGCOLOR, "ZONCETA");//��ʾ��ӭҳ��
	Gui_DrawFont_GBK36(15,50,TEXTCOLOR,BGCOLOR, "��������");
	Gui_DrawFont_GBK32(45,85,TEXTCOLOR,BGCOLOR, "V1.0");
	LCD_LED_SET;					//����LCD������
    delayms(2000);					//�ȴ�
	set_ScreenPageID(MEASURE_PAGE);	//������ʾҳ��IDΪ����ҳ��
	MeasureData.lMaxDeta = 0;
	MeasureData.lMinDeta = 9999.9999;
	Lcd_Clear(BLACK);				//����
	delayms(10);					//�ȴ�
	#ifdef TEST
	MeasureData.lDeta = 22.9100;
	#endif
}

//---------------------------------------------------------
// ���ܣ�Screen��˸��ʾ�ַ���s
//---------------------------------------------------------
void Gui_BlinkShow(uint8_t ux,uint8_t uy,uint8_t *s)
{ 
	//ͨ����ʱ��100ms�ı��־λʵ��
	if(PageCursor.uBlinkFlag) 
	{
		Gui_DrawFont_GBK16(ux,uy,BGCOLOR,BGCOLOR,s);		//����������ɫ�ͱ���ɫһ�£���������ʧ
	}
	else
	{
		Gui_DrawFont_GBK16(ux,uy,TEXTCOLOR,BGCOLOR,s);		//����������ɫΪ����������������
	}
}

//---------------------------------------------------------
// ���ܣ�screenչʾ����ҳ��
//---------------------------------------------------------
uint16_t tempPercentColor;
double tempPercent;
uint16_t ntempPercent;
uint16_t tempBackColor =0;
double temp;

uint8_t redflag =1;//��ƿ��Ʊ�־
void ShowMeasurePage(void)
{
	uint8_t *s,*stemp,str[1]="0",*s1,*s2;
	uint32_t lvoltage; //�⵽�ĵ�ѹֵ	
	uint8_t ui;
	double percent ;       //�������ٷֱ�
	uint16_t percentColor;//�ζ�������ɫ ���������ǿ�
	uint16_t backColor;   //������ɫ
	uint16_t nPercent;    //�ζ�����x��λ��

	uint8_t nPercentWS2821 = 0;
	double lToleranceUpperLimit,lToleranceLowerLimit,lTUpperLimitWarning,lTLowerLimitWarning,ltemp1,ltemp2;
	
	//ҳ���һ�μ��أ���Ҫ��������������ʵʱ���µľ�̬����
	if(ScreenPageInfo.uPageReLoadFlag == MEASURE_PAGE)
	{
		Lcd_Clear(BGCOLOR);									//����
		//���ؾ�̬ҳ��
		Gui_DrawFont_GBK16(135,5,TEXTCOLOR,BGCOLOR,"mm");
		Gui_DrawFont_GBK16(127,23,TEXTCOLOR,BGCOLOR,"[");
		Gui_DrawFont_GBK16(135,23,RED,BGCOLOR,"NO");   		//��ɫ������
		Gui_DrawFont_GBK16(151,23,TEXTCOLOR,BGCOLOR,"]");
		Gui_DrawFont_GBK16(20,50,TEXTCOLOR,BGCOLOR,"MAX:");
		Gui_DrawFont_GBK16(130,50,TEXTCOLOR,BGCOLOR,"mm");
		Gui_DrawFont_GBK16(20,75,TEXTCOLOR,BGCOLOR,"MIN:");
		Gui_DrawFont_GBK16(130,75,TEXTCOLOR,BGCOLOR,"mm");
			
		//Ԥ����ҳ������
		s = _Double2Char(0,8); 										//ת�ַ���
		Gui_DrawFont_GBK24(15,12,TEXTCOLOR,BGCOLOR,s);
		free(s);
		s = _Double2Char(MeasureData.lMaxDeta,8);   				//ת�ַ���
		Gui_DrawFont_GBK16(55,50,TEXTCOLOR,BGCOLOR,s);
		free(s);
		s = _Double2Char(MeasureData.lMinDeta,8);   				//ת�ַ���
		Gui_DrawFont_GBK16(55,75,TEXTCOLOR,BGCOLOR,s);
		free(s);

		Gui_DrawFont_GBK16(0,112,TEXTCOLOR,RED,"         NO         ");//21��

		tempBackColor = RED;
		
		ScreenPageInfo.uPageReLoadFlag = 0;					//������¼��ر�־λ
		MeasureData.lTempDeta = 0;
//		RGB_LED_Reset();
//		RGB_LED_Red(); //��ɫ

		if(redflag)
		{
			redflag = 0;
			HAL_TIM_Base_Stop_IT(&htim2);  //�رն�ʱ���ж�
			RGB_LED_Reset();
			RGB_LED_Red(); //��ɫ
			HAL_TIM_Base_Start_IT(&htim2);  //������ʱ���ж�
		}

//		RGB_LED_Reset();
//		HAL_TIM_Base_Stop_IT(&htim2);  //�رն�ʱ���ж�
//		RGB_LED_Red(); //��ɫ
//		HAL_TIM_Base_Start_IT(&htim2);  //������ʱ���ж�
		
	}
	else	//ҳ��������أ�ֻ��Ҫ����ʵʱ���µ�ֵ
	{
		//RGB_LED_Write0();
	
		if(MeasureData.uMeasureFlag) //adc �ɼ�ʱ�䵽��
		{
		    lvoltage = getMeasureVoltage(); //ת��Ϊ��ѹ���� ----->�����������㣬ת��Ϊ���������ʾ����
			#ifndef TEST
			MeasureData.lDeta  = CalDetaValue(lvoltage);  	//LCD����ʾ��ǰ��ѹ����
			//printf("voltage = %d mv\r\n ",lvoltage);
			//printf("output mater = %f mm\r\n ",MeasureData.lDeta);
			#endif
			MeasureData.uMeasureFlag = 0;
			//choicelight(1,0,0);
		}
		//�ж���ֵ�Ƿ񱻸��£��и�������ص�ҳ��
		if((MeasureData.lDeta != MeasureData.lTempDeta)&&(MeasureData.lDeta<=9999.9999))
		{
			printf("�����и���!\r\n");
			printf("voltage = %d mv\r\n ",lvoltage);
			printf("output mater = %f mm\r\n ",MeasureData.lDeta);
			
			stemp= _Double2Char(MeasureData.lTempDeta,8); 	//ת�ַ���
			s = _Double2Char(MeasureData.lDeta,8); 			//ת�ַ���
			//��������ʾ�Ż�
			s1 = s;
			s2 = stemp;
			for(ui=0;ui<9;ui++)
			{
				if(ui!=4) //����С����.
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


			//��ͷû��������ѹ��ʱ�� ���ǻ�
			if((MeasureData.lDeta>SettingData.lNominalSize - MAX_DEVIATION_VALUE) //����ƫ��ֵ
				&&(MeasureData.lDeta<SettingData.lNominalSize + MAX_DEVIATION_VALUE))	
			{	
				printf("��⵽��ͷ����!\r\n");
				//printf("voltage = %d mv\r\n ",lvoltage);
				//printf("output mater = %f mm\r\n ",MeasureData.lDeta);
				MeasureData.lMaxDeta = MeasureData.lDeta > MeasureData.lMaxDeta?MeasureData.lDeta:MeasureData.lMaxDeta;
				MeasureData.lMinDeta = MeasureData.lDeta > MeasureData.lMinDeta?MeasureData.lMinDeta:MeasureData.lDeta;
							
				if(MeasureData.lMaxDeta != MeasureData.lTempMaxDeta)//��ʾ���ֵ
				{
					s = _Double2Char(MeasureData.lMaxDeta,8); 			//ת�ַ���
					Gui_DrawFont_GBK16(55,50,TEXTCOLOR,BGCOLOR,s);
					free(s);
				}
				if(MeasureData.lMinDeta != MeasureData.lTempMinDeta)//��ʾ��Сֵ
				{
					s = _Double2Char(MeasureData.lMinDeta,8); 			//ת�ַ���
					Gui_DrawFont_GBK16(55,75,TEXTCOLOR,BGCOLOR,s);
					free(s);
				}
				if(SettingData.uToleranceUpperLimitSymbol == 1) //������ֳ��� ��������  
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

				//��ǰ����ֵ С�ڵ��� ���幫��ֵ+�������� ���� ��ǰ����ֵ ���ڵ��� ���幫��ֵ+��������
				if((MeasureData.lDeta <= SettingData.lNominalSize + lToleranceUpperLimit)&&(MeasureData.lDeta >= SettingData.lNominalSize + lToleranceLowerLimit))
				{
					printf("�����ϸ�!\r\n");

					backColor = GRAY0;//�����������
					//����������İٷֱ�
					percent = (MeasureData.lDeta -(SettingData.lNominalSize + lToleranceLowerLimit) ) / ((SettingData.lNominalSize + lToleranceUpperLimit) - (SettingData.lNominalSize + lToleranceLowerLimit));

				#if 0
					//Ԥ����ɫ����
					if((MeasureData.lDeta>SettingData.lNominalSize+lTUpperLimitWarning)||(MeasureData.lDeta<SettingData.lNominalSize+lTLowerLimitWarning))
					{
						percentColor = ORAGNE; //��������ɫ Ԥ��
					}
					else
					{
						percentColor = GREEN;//��������ɫ ����
					}
				#else
					//������ʾ
					if(percent <= 0.33) percentColor = GREEN;
					else if(0.33<percent<=0.66) percentColor = BLUE;
					else if(0.66<percent<=1) percentColor = ORAGNE;					
				#endif

					//�������Ͻ�ok ����perent��ʾ��ͬ��ɫ
					Gui_DrawFont_GBK16(135,23,percentColor,BGCOLOR,"OK");
					//������1��ʾ�Ż�
//					if(backColor == tempBackColor)
//					{
//						if(percentColor == tempPercentColor)
//						{
//							if(percent > tempPercent)
//							{
//								for(ui=tempPercent*21;ui<percent*21;ui++)
//								Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,percentColor," ");//����
//							}
//							if(percent < tempPercent)
//							{
//								for(ui=percent*21;ui<tempPercent*21;ui++)
//								Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,backColor," ");//����
//							}
//						}
//						else
//						{
//							for(ui=0;ui<percent*21;ui++)
//							Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,percentColor," ");//����
//							for(ui=percent*21;ui<21;ui++)
//							Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,backColor," ");//����
//						}
//					}
//					else
//					{					
//						for(ui=0;ui<percent*21;ui++)
//						Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,percentColor," ");//����
//						for(ui=percent*21;ui<21;ui++)
//						Gui_DrawFont_GBK16(ui*8,112,TEXTCOLOR,backColor," ");//����
//					}

					nPercent = percent*160;	 //С����λ��
					nPercentWS2821 = percent*20;//������λ��

					HAL_TIM_Base_Stop_IT(&htim2);  //�رն�ʱ���ж�
					//RGB_LED_Reset();
					//RGB_LED_Red(); //��ɫ
					choicelight(nPercentWS2821,0,0); //ִ��С������ɫ�仯
					HAL_TIM_Base_Start_IT(&htim2);  //������ʱ���ж�
					redflag = 1;//���������

					//������2��ʾ�Ż�
					if(backColor == tempBackColor) //֮ǰ����ɫû�б� 
					{
						Gui_DrawFont_GBK16(ntempPercent,112,TEXTCOLOR,backColor," ");//ֻ��1��
						Gui_DrawFont_GBK16(nPercent,112,TEXTCOLOR,percentColor," ");//�ڼ������ӱ�ɫ
					}
					else //��ɫ����   ������ɫ�仯
					{
						Gui_DrawFont_GBK16(0,112,TEXTCOLOR,backColor,"                     ");//ȫ��ɫ
						Gui_DrawFont_GBK16(nPercent,112,TEXTCOLOR,percentColor," ");//�ڼ������ӱ�ɫ
					}

					tempPercentColor = percentColor;//�ݴ��������ɫ
					ntempPercent = nPercent; //�ݴ�ڼ������ӱ�ɫ
				}
				else //����������ϸ�
				{
					printf("�������ϸ�!\r\n");
					backColor = RED; //����ɫ�Ǻ�ɫ 
					Gui_DrawFont_GBK16(135,23,backColor,BGCOLOR,"NO"); //�������ñ���ɫ
					Gui_DrawFont_GBK16(0,112,TEXTCOLOR,backColor,"         NO          ");//21 ����д��      112

//					RGB_LED_Reset();
//					HAL_TIM_Base_Stop_IT(&htim2);  //�رն�ʱ���ж�
//					RGB_LED_Red(); //��ɫ
//					HAL_TIM_Base_Start_IT(&htim2);  //������ʱ���ж�
//
//					if(backColor != tempBackColor) //�����ɫ�����仯
//					{
//						HAL_TIM_Base_Stop_IT(&htim2);  //�رն�ʱ���ж�
//						RGB_LED_Reset();
//						RGB_LED_Red(); //��ɫ
//						HAL_TIM_Base_Start_IT(&htim2);  //������ʱ���ж�
//					}
					if(redflag)
					{
						redflag = 0;
						HAL_TIM_Base_Stop_IT(&htim2);  //�رն�ʱ���ж�
						RGB_LED_Reset();
						RGB_LED_Red(); //��ɫ
						HAL_TIM_Base_Start_IT(&htim2);  //������ʱ���ж�
					}

				}
				tempBackColor = backColor;//������һ�εı�����ɫ	
				MeasureData.lTempMaxDeta = MeasureData.lMaxDeta;
				MeasureData.lTempMinDeta = MeasureData.lMinDeta;
				MeasureData.lTempDeta = MeasureData.lDeta;	//������һ�ε�����	
			}
			else 
			{
				printf("��⵽��ͷ�γ�!\r\n");
				//RGB_LED_Red(); //��ɫ
				backColor = RED;
				Gui_DrawFont_GBK16(0,112,TEXTCOLOR,backColor,"         ERR         ");//21��
			}
			//����������  һ���ǲ�ͷ�γ��� һ���ǲ�ͷû�н���ѹ
		}

	}
}

//---------------------------------------------------------
// screenչʾ�˵�ҳ��
//---------------------------------------------------------
void ShowMenuPage(void)
{
	//ҳ���һ�μ��أ���Ҫ��������������ʵʱ���µľ�̬����
	if(ScreenPageInfo.uPageReLoadFlag == MENU_PAGE)			
	{
		//���ؾ�̬����
		Lcd_Clear(BGCOLOR);										//����
		Gui_DrawFont_GBK24(2,25,TEXTCOLOR,BGCOLOR,"����");
		Gui_DrawFont_GBK24(2,85,TEXTCOLOR,BGCOLOR,"����");
		Gui_DrawFont_GBK24(108,25,TEXTCOLOR,BGCOLOR,"У��");
		Gui_DrawFont_GBK24(108,85,TEXTCOLOR,BGCOLOR,"��ѯ");
		ScreenPageInfo.uPageReLoadFlag = 0;						//������¼��ر�־λ
	}
	else
	{
		//�������λ�ã����غ��ͷ����λ�ñ���ɫ
		if(PageCursor.uMenuPage_CurCursor != PageCursor.uMenuPage_LastCursor)
		{
			switch(PageCursor.uMenuPage_CurCursor)
			{
				case MEASURE_POINTER:
					Gui_DrawFont_GBK24(2,25,TEXTCOLOR,TEXTBGCOLOR,"����");
				    Gui_DrawFont_GBK24(50,25,TEXTCOLOR,BGCOLOR,"��");
					break;
				case SETTING_POINTER:
					Gui_DrawFont_GBK24(2,85,TEXTCOLOR,TEXTBGCOLOR,"����");
					Gui_DrawFont_GBK24(50,85,TEXTCOLOR,BGCOLOR,"��");
					break;

				case CORRECT_POINTER:
					Gui_DrawFont_GBK24(108,25,TEXTCOLOR,TEXTBGCOLOR,"У��");
					Gui_DrawFont_GBK24(82,25,TEXTCOLOR,BGCOLOR,"��");
					break;
				case SEARCH_POINTER:
					Gui_DrawFont_GBK24(108,85,TEXTCOLOR,TEXTBGCOLOR,"��ѯ");
					Gui_DrawFont_GBK24(82,85,TEXTCOLOR,BGCOLOR,"��");	
					break;
				default:
					break;
			}
			switch(PageCursor.uMenuPage_LastCursor)
			{
				case MEASURE_POINTER:
					Gui_DrawFont_GBK24(2,25,TEXTCOLOR,BGCOLOR,"����");
					Gui_DrawFont_GBK24(50,25,BGCOLOR,BGCOLOR,"��");
					break;
				case SETTING_POINTER:
					Gui_DrawFont_GBK24(2,85,TEXTCOLOR,BGCOLOR,"����");
					Gui_DrawFont_GBK24(50,85,BGCOLOR,BGCOLOR,"��");
					break;
				case CORRECT_POINTER:
					Gui_DrawFont_GBK24(108,25,TEXTCOLOR,BGCOLOR,"У��");
					Gui_DrawFont_GBK24(82,25,BGCOLOR,BGCOLOR,"��");
					break;
				case SEARCH_POINTER:
					Gui_DrawFont_GBK24(108,85,TEXTCOLOR,BGCOLOR,"��ѯ");
					Gui_DrawFont_GBK24(82,85,BGCOLOR,BGCOLOR,"��");
					break;
				default:
					break;
			}
			PageCursor.uMenuPage_LastCursor = PageCursor.uMenuPage_CurCursor;   //��¼��굱ǰλ��
		}
	}
	

}



//---------------------------------------------------------
// ���ܣ�screenչʾ����ҳ��1
//---------------------------------------------------------
void ShowSettingPage1(void)
{
	uint8_t *s;
	
	//ҳ���һ�μ��أ���Ҫ��������������ʵʱ���¾�̬����
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_1)  //��һ�����ý���
	{	
		//���ؾ�̬ҳ��
		Lcd_Clear(BGCOLOR);									 //����					
		Gui_DrawFont_GBK16(35,50,TEXTCOLOR,BGCOLOR,"վ��ѡ��:"); 
		
		//����Ԥ����
		SettingData.lTempProgramNo = SettingData.lProgramNo; //��������ֵ ������ʱ�޸�
		SettingData.lTempStationNo = SettingData.lStationNo; //��������ֵ ������ʱ�޸�
		PageCursor.uSelected = 1;							 //ѡ������Ϊ��һ��
		ScreenPageInfo.uPageReLoadFlag = 0;					 //������¼��ر�־λ
	}
	else //ҳ��������أ�ֻ��Ҫ����ʵʱ���µ�ֵ
	{
		//����ѡ�������� ������˸Ч��
		if(PageCursor.uSelected == 1)
		{	
			s = Int2Char(SettingData.lTempStationNo);		 //վ��ת�ַ���
			Gui_BlinkShow(115,50,s); 						 //��˸��ʾվ��
			free(s);
		}
		else if(PageCursor.uSelected == 2)					 //�µ�ȷ�ϼ����º�
		{
			if(PageCursor.uNewSlected)						 //�̶��µľ�̬ҳ��
			{
				s = Int2Char(SettingData.lStationNo);		 //�̶�վ�ŵ���ʾ
				Gui_DrawFont_GBK16(115,50,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;					 //�����־
				MeasureData.lMaxDeta = 0;
				MeasureData.lMinDeta = 9999.9999;
			}
		}

	}
}

//---------------------------------------------------------
// screenչʾ����ҳ��2
//---------------------------------------------------------
void ShowSettingPage2(void)
{
	uint8_t *s;
	
	//ҳ���һ�μ��أ���Ҫ��������������ʵʱ���¾�̬����
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_2)
	{
		//���ؾ�̬ҳ��
		Lcd_Clear(BGCOLOR);											//����		
		Gui_DrawFont_GBK16(30,40,TEXTCOLOR,BGCOLOR,"���Ƴߴ�");
		Gui_DrawFont_GBK16(120,40,TEXTCOLOR,BGCOLOR,"mm");
		//����Ԥ����
		Double2StrArr(SettingData.lNominalSize,SettingData.nNominalSizeCharArr,8); //�����Ƴߴ�����ֵÿһ����λתΪ�ַ�������
		SettingData.lTempNominalSize = SettingData.lNominalSize; 	//��������ֵ ������ʱ�޸�
		PageCursor.uSelected = 1;									//ѡ������Ϊ��һ��
		PageCursor.uEditPos = 0;									//���ֱ༭λ��Ĭ��Ϊ���λ
		ScreenPageInfo.uPageReLoadFlag = 0;							//������¼��ر�־λ
		PageCursor.uNewSlected = 1;
	}
	else //ҳ��������أ�ֻ��Ҫ����ʵʱ���µ�ֵ
	{
		//����ѡ�������� ������˸Ч��
		if(PageCursor.uSelected == 1) 							//�µ�ȷ�ϼ����º�
		{
			if(PageCursor.uNewSlected)								//�̶��µľ�̬ҳ��
			{				
				s = _Double2Char(SettingData.lTempNominalSize,8);	//��ʾ��ʱ�Ĺ��Ƴߴ�
				Gui_DrawFont_GBK16(50,70,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;
			}
			if(PageCursor.uEditPosChangeFlag)						//���Ұ������� �༭λ�øı� ��������ʾ��һ�α༭λ�õ�����
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
			//���ݱ༭λ�� ��˸��ʾ��ǰλ�õ�����
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(114,70,SettingData.nNominalSizeCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(106,70,SettingData.nNominalSizeCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(98,70,SettingData.nNominalSizeCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(90,70,SettingData.nNominalSizeCharArr[3]);
			else if(PageCursor.uEditPos == 4)Gui_BlinkShow(74,70,SettingData.nNominalSizeCharArr[4]);
			else if(PageCursor.uEditPos == 5)Gui_BlinkShow(66,70,SettingData.nNominalSizeCharArr[5]);
			else if(PageCursor.uEditPos == 6)Gui_BlinkShow(58,70,SettingData.nNominalSizeCharArr[6]);
			else if(PageCursor.uEditPos == 7)Gui_BlinkShow(50,70,SettingData.nNominalSizeCharArr[7]);
		}
		else if(PageCursor.uSelected == 2)							//�µ�ȷ�ϼ����º�
		{
			if(PageCursor.uNewSlected)								//�̶��µľ�̬ҳ��
			{
				s = _Double2Char(SettingData.lNominalSize,8);		//�����Ƴߴ�תΪ�ַ���
				Gui_DrawFont_GBK16(50,70,TEXTCOLOR,BGCOLOR,s);		//�̶����Ƴߴ����ʾ
				PageCursor.uNewSlected = 0;
				free(s);
			}
		}
	}
}

//---------------------------------------------------------
// screenչʾ����ҳ��3
//---------------------------------------------------------
void ShowSettingPage3(void)
{
	uint8_t *s;
	
	//ҳ���һ�μ��أ���Ҫ��������������ʵʱ���¾�̬����
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_3)
	{
		//���ؾ�̬����
		Lcd_Clear(BGCOLOR);															//����
		Gui_DrawFont_GBK16(30,0,TEXTCOLOR,BGCOLOR,"�������� ��m");		
		if(SettingData.uToleranceUpperLimitSymbol)Gui_DrawFont_GBK16(55,16,TEXTCOLOR,BGCOLOR,"-");
		else Gui_DrawFont_GBK16(55,16,TEXTCOLOR,BGCOLOR,"+");
		s = _Double2Char(SettingData.lToleranceUpperLimit,4);						//��������������ֵתΪ�ַ���
		Gui_DrawFont_GBK16(65,16,TEXTCOLOR,BGCOLOR,s);
		free(s);
		
		//����ǰ���õĳ���������ÿһ����λ���Ϊ�ַ������ص�Arr��
		Double2StrArr(SettingData.lToleranceUpperLimit,SettingData.uToleranceUpperLimitCharArr,4);
		Double2StrArr(SettingData.lToleranceLowerLimit,SettingData.uToleranceLowerLimitCharArr,4);
		Double2StrArr(SettingData.lTUpperLimitWarning,SettingData.uTUpperLimitWarningCharArr,4);
		Double2StrArr(SettingData.lTLowerLimitWarning,SettingData.uTLowerLimitWarningCharArr,4);
		
		//����Ԥ����
		SettingData.lTempToleranceUpperLimit = SettingData.lToleranceUpperLimit;	//��������ֵ ������ʱ�޸�
		SettingData.lTempToleranceLowerLimit = SettingData.lToleranceLowerLimit;    //��������ֵ ������ʱ�޸�
		SettingData.lTempTUpperLimitWarning = SettingData.lTUpperLimitWarning;	//��������ֵ ������ʱ�޸�
		SettingData.lTempTLowerLimitWarning = SettingData.lTLowerLimitWarning;    //��������ֵ ������ʱ�޸�
		ScreenPageInfo.uPageReLoadFlag = 0;											//������¼��ر�־λ						
		PageCursor.uEditPos = 0;													//Ĭ�ϱ༭λ��Ϊ���λ
		PageCursor.uEditPosChangeFlag = 0 ;											//������λ�øı��־
		PageCursor.uSelected = 1;													//���ñ༭��һ��
	}
	else //ҳ��������أ�ֻ��Ҫ����ʵʱ���µ�ֵ
	{
		
		//����ѡ�������� ������˸Ч��
		if(PageCursor.uSelected == 1)  
		{	
			if(PageCursor.uEditPosChangeFlag)						//���Ұ������� �༭λ�øı� ��������ʾ��һ�α༭λ�õ�����
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
			//���ݱ༭λ�� ��˸��ʾ��ǰλ�õ�����
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
		else if(PageCursor.uSelected == 2) 											//�µ�ȷ�ϼ����º�
		{
			if(PageCursor.uNewSlected)												//�̶��µľ�̬ҳ��
			{	
				s = _Double2Char(SettingData.lTempToleranceUpperLimit,4);
				Gui_DrawFont_GBK16(65,16,TEXTCOLOR,BGCOLOR,s);						//�̶�������������ֵ����ʾ					
				Gui_DrawFont_GBK16(30,32,TEXTCOLOR,BGCOLOR,"�������� ��m");
				if(SettingData.uToleranceLowerLimitSymbol)Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"-");
				else Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"+");
				free(s);
				s = _Double2Char(SettingData.lTempToleranceLowerLimit,4);				//Ԥ��ʾ������������ֵ
				Gui_DrawFont_GBK16(65,48,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;											//��־����
				PageCursor.uEditPos = 0;											//Ĭ�ϱ༭λ��Ϊ���λ		
			}
			if(PageCursor.uEditPosChangeFlag)						//���Ұ������� �༭λ�øı� ��������ʾ��һ�α༭λ�õ�����
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
			//���ݱ༭λ�� ��˸��ʾ��ǰλ�õ�����
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
		else if(PageCursor.uSelected == 3)											//�µ�ȷ�ϼ����º�
		{
			if(PageCursor.uNewSlected)												//�̶��µľ�̬ҳ��
			{
				s = _Double2Char(SettingData.lTempToleranceLowerLimit,4);				//�̶�������������ֵ����ʾ	
				Gui_DrawFont_GBK16(65,48,TEXTCOLOR,BGCOLOR,s);	
				if(SettingData.uToleranceLowerLimitSymbol)Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"-");
				else Gui_DrawFont_GBK16(55,48,TEXTCOLOR,BGCOLOR,"+");				
				free(s);
				Gui_DrawFont_GBK16(30,64,TEXTCOLOR,BGCOLOR,"Ԥ������ ��m");
				if(SettingData.uToleranceUpperLimitSymbol)Gui_DrawFont_GBK16(55,80,TEXTCOLOR,BGCOLOR,"-");
				else Gui_DrawFont_GBK16(55,80,TEXTCOLOR,BGCOLOR,"+");
				free(s);
				s = _Double2Char(SettingData.lTempTUpperLimitWarning,4);				//Ԥ��ʾ������������ֵ
				Gui_DrawFont_GBK16(65,80,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;											//��־����
				PageCursor.uEditPos = 0;											//Ĭ�ϱ༭λ��Ϊ���λ	
				PageCursor.uEditPosChangeFlag = 0 ;	
			}
			if(PageCursor.uEditPosChangeFlag)						//���Ұ������� �༭λ�øı� ��������ʾ��һ�α༭λ�õ�����
			{
				if(PageCursor.uLastEditPos == 0)Gui_DrawFont_GBK16(97,80,TEXTCOLOR,BGCOLOR,SettingData.uTUpperLimitWarningCharArr[0]);
				else if(PageCursor.uLastEditPos == 1)Gui_DrawFont_GBK16(81,80,TEXTCOLOR,BGCOLOR,SettingData.uTUpperLimitWarningCharArr[1]);
				else if(PageCursor.uLastEditPos == 2)Gui_DrawFont_GBK16(73,80,TEXTCOLOR,BGCOLOR,SettingData.uTUpperLimitWarningCharArr[2]);
				else if(PageCursor.uLastEditPos == 3)Gui_DrawFont_GBK16(65,80,TEXTCOLOR,BGCOLOR,SettingData.uTUpperLimitWarningCharArr[3]);
				PageCursor.uEditPosChangeFlag = 0;
			}
			//���ݱ༭λ�� ��˸��ʾ��ǰλ�õ�����
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(97,80,SettingData.uTUpperLimitWarningCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(81,80,SettingData.uTUpperLimitWarningCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(73,80,SettingData.uTUpperLimitWarningCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(65,80,SettingData.uTUpperLimitWarningCharArr[3]);
		}
		else if(PageCursor.uSelected == 4)											//�µ�ȷ�ϼ����º�
		{
			if(PageCursor.uNewSlected)												//�̶��µľ�̬ҳ��
			{
				s = _Double2Char(SettingData.lTempTUpperLimitWarning,4);				//�̶�������������ֵ����ʾ	
				Gui_DrawFont_GBK16(65,80,TEXTCOLOR,BGCOLOR,s);	
				Gui_DrawFont_GBK16(30,96,TEXTCOLOR,BGCOLOR,"Ԥ������ ��m");
				if(SettingData.uToleranceLowerLimitSymbol)Gui_DrawFont_GBK16(55,112,TEXTCOLOR,BGCOLOR,"-");
				else Gui_DrawFont_GBK16(55,112,TEXTCOLOR,BGCOLOR,"+");
				free(s);
				s = _Double2Char(SettingData.lTempTLowerLimitWarning,4);				//Ԥ��ʾ������������ֵ
				Gui_DrawFont_GBK16(65,112,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;											//��־����
				PageCursor.uEditPos = 0;											//Ĭ�ϱ༭λ��Ϊ���λ	
				PageCursor.uEditPosChangeFlag = 0 ;	
			}
			if(PageCursor.uEditPosChangeFlag)						//���Ұ������� �༭λ�øı� ��������ʾ��һ�α༭λ�õ�����
			{
				if(PageCursor.uLastEditPos == 0)Gui_DrawFont_GBK16(97,112,TEXTCOLOR,BGCOLOR,SettingData.uTLowerLimitWarningCharArr[0]);
				else if(PageCursor.uLastEditPos == 1)Gui_DrawFont_GBK16(81,112,TEXTCOLOR,BGCOLOR,SettingData.uTLowerLimitWarningCharArr[1]);
				else if(PageCursor.uLastEditPos == 2)Gui_DrawFont_GBK16(73,112,TEXTCOLOR,BGCOLOR,SettingData.uTLowerLimitWarningCharArr[2]);
				else if(PageCursor.uLastEditPos == 3)Gui_DrawFont_GBK16(65,112,TEXTCOLOR,BGCOLOR,SettingData.uTLowerLimitWarningCharArr[3]);
				PageCursor.uEditPosChangeFlag = 0;
			}
			//���ݱ༭λ�� ��˸��ʾ��ǰλ�õ�����
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(97,112,SettingData.uTLowerLimitWarningCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(81,112,SettingData.uTLowerLimitWarningCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(73,112,SettingData.uTLowerLimitWarningCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(65,112,SettingData.uTLowerLimitWarningCharArr[3]);
		}
		else if(PageCursor.uSelected == 5)											//�µ�ȷ�ϼ����º�
		{
			if(PageCursor.uNewSlected)												//�̶��µľ�̬ҳ��
			{
				s = _Double2Char(SettingData.lTempTLowerLimitWarning,4);				//�̶���ʾ������������ֵ
				Gui_DrawFont_GBK16(65,112,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;											//��־����
				PageCursor.uEditPos = 0;											//Ĭ�ϱ༭λ��Ϊ���λ	
				PageCursor.uEditPosChangeFlag = 0 ;	
			}
		}
	}
}
//---------------------------------------------------------
// screenչʾ����ҳ��4
//---------------------------------------------------------
void ShowSettingPage4(void)
{
	uint8_t *s;
	//ҳ���һ�μ��أ���Ҫ��������������ʵʱ���¾�̬����
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_4)
	{
		//���ؾ�̬ҳ��
		Lcd_Clear(BGCOLOR);
		Gui_DrawFont_GBK16(20,20,TEXTCOLOR,BGCOLOR,"���ޱ�׼��ֵ mm");
		s = _Double2Char(SettingData.lStandardUpperLimit,8);								//��׼����������ֵתΪ�ַ���
		Gui_DrawFont_GBK16(50,45,TEXTCOLOR,BGCOLOR,s);
		free(s);
		
		//����ǰ���õĳ���������ÿһλ���Ϊ�ַ������ص�Arr��
		Double2StrArr(SettingData.lStandardUpperLimit,SettingData.uStandardUpperLimitCharArr,8);
		Double2StrArr(SettingData.lStandardLowerLimit,SettingData.uStandardLowerLimitCharArr,8);
		
		//����Ԥ����
		SettingData.lTempStandardUpperLimit = SettingData.lStandardUpperLimit;				//��������ֵ ������ʱ�޸�
		SettingData.lTempStandardLowerLimit = SettingData.lStandardLowerLimit;				//��������ֵ ������ʱ�޸�
		ScreenPageInfo.uPageReLoadFlag = 0;													//������¼��ر�־λ
		PageCursor.uEditPos = 0;															//Ĭ�ϱ༭λ��Ϊ���λ
		PageCursor.uEditPosChangeFlag = 0 ;													//������λ�øı��־
		PageCursor.uSelected = 1;															//���ñ༭��һ��
	}
	else //ҳ��������أ�ֻ��Ҫ����ʵʱ���µ�ֵ
	{
		//����ѡ�������� ������˸Ч��
		if(PageCursor.uSelected == 1)  					
		{	
			if(PageCursor.uEditPosChangeFlag)			//���Ұ������� �༭λ�øı� ��������ʾ��һ�α༭λ�õ�����
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
			//���ݱ༭λ�� ��˸��ʾ��ǰλ�õ�����
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(114,45,SettingData.uStandardUpperLimitCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(106,45,SettingData.uStandardUpperLimitCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(98,45,SettingData.uStandardUpperLimitCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(90,45,SettingData.uStandardUpperLimitCharArr[3]);
			else if(PageCursor.uEditPos == 4)Gui_BlinkShow(74,45,SettingData.uStandardUpperLimitCharArr[4]);
			else if(PageCursor.uEditPos == 5)Gui_BlinkShow(66,45,SettingData.uStandardUpperLimitCharArr[5]);
			else if(PageCursor.uEditPos == 6)Gui_BlinkShow(58,45,SettingData.uStandardUpperLimitCharArr[6]);
			else if(PageCursor.uEditPos == 7)Gui_BlinkShow(50,45,SettingData.uStandardUpperLimitCharArr[7]);
		}
		else if(PageCursor.uSelected == 2) 												//�µ�ȷ�ϼ����º�
		{
			if(PageCursor.uNewSlected)													//�̶��µľ�̬ҳ��
			{
				s = _Double2Char(SettingData.lTempStandardUpperLimit,8);					//��׼������ת�ַ���
				Gui_DrawFont_GBK16(50,45,TEXTCOLOR,BGCOLOR,s);									
				Gui_DrawFont_GBK16(20,70,TEXTCOLOR,BGCOLOR,"���ޱ�׼��ֵ mm");			
				free(s);
				s = _Double2Char(SettingData.lTempStandardLowerLimit,8);				//Ԥ��ʾ��׼����������ֵ
				Gui_DrawFont_GBK16(50,95,TEXTCOLOR,BGCOLOR,s);
				free(s);
				PageCursor.uNewSlected = 0;
				PageCursor.uEditPos = 0;
			}
			if(PageCursor.uEditPosChangeFlag)		 //���Ұ������� �༭λ�øı� ��������ʾ��һ�α༭λ�õ�����
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
			//���ݱ༭λ�� ��˸��ʾ��ǰλ�õ�����
			if(PageCursor.uEditPos == 0)Gui_BlinkShow(114,95,SettingData.uStandardLowerLimitCharArr[0]);
			else if(PageCursor.uEditPos == 1)Gui_BlinkShow(106,95,SettingData.uStandardLowerLimitCharArr[1]);
			else if(PageCursor.uEditPos == 2)Gui_BlinkShow(98,95,SettingData.uStandardLowerLimitCharArr[2]);
			else if(PageCursor.uEditPos == 3)Gui_BlinkShow(90,95,SettingData.uStandardLowerLimitCharArr[3]);
			else if(PageCursor.uEditPos == 4)Gui_BlinkShow(74,95,SettingData.uStandardLowerLimitCharArr[4]);
			else if(PageCursor.uEditPos == 5)Gui_BlinkShow(66,95,SettingData.uStandardLowerLimitCharArr[5]);
			else if(PageCursor.uEditPos == 6)Gui_BlinkShow(58,95,SettingData.uStandardLowerLimitCharArr[6]);
			else if(PageCursor.uEditPos == 7)Gui_BlinkShow(50,95,SettingData.uStandardLowerLimitCharArr[7]);
		}
		else if(PageCursor.uSelected == 3)											//�µ�ȷ�ϼ����º�
		{
			if(PageCursor.uNewSlected)												//�̶��µľ�̬ҳ��
			{
				s = _Double2Char(SettingData.lTempStandardLowerLimit,8);
				Gui_DrawFont_GBK16(50,95,TEXTCOLOR,BGCOLOR,s);						//�̶���׼����������ֵ����ʾ	
				PageCursor.uNewSlected = 0;
				free(s);
			}
		}
	}
}

//---------------------------------------------------------
// screenչʾ����ҳ��5
//---------------------------------------------------------
void ShowSettingPage5(void)
{
	
	//ҳ���һ�μ��أ���Ҫ��������������ʵʱ���¾�̬����
	if(ScreenPageInfo.uPageReLoadFlag == SETTING_PAGE_5)
	{
		//���ؾ�̬ҳ��
		Lcd_Clear(BGCOLOR);											//����
		Gui_DrawFont_GBK16(20,50,TEXTCOLOR,BGCOLOR,"����ģʽ:");
		//����Ԥ����
		ScreenPageInfo.uPageReLoadFlag = 0;
		SettingData.nTempMeasureMode = SettingData.lMeasureMode;
		if(SettingData.nTempMeasureMode == INNER_MEMODE)Gui_DrawFont_GBK16(95,50,TEXTCOLOR,BGCOLOR,"�ڳߴ�");
		else if(SettingData.nTempMeasureMode == OUTER_MEMODE)Gui_DrawFont_GBK16(95,50,TEXTCOLOR,BGCOLOR,"��ߴ�");
		PageCursor.uEditPosChangeFlag = 1;
	}
	else //ҳ��������أ�ֻ��Ҫ����ʵʱ���µ�ֵ
	{
		if(PageCursor.uEditPosChangeFlag)
		{
			//�༭��Ӧ����ʾ��Ӧ����
			PageCursor.uEditPosChangeFlag = 0;
			if(SettingData.nTempMeasureMode == INNER_MEMODE)Gui_DrawFont_GBK16(95,50,TEXTCOLOR,BGCOLOR,"�ڳߴ�");
			else if(SettingData.nTempMeasureMode == OUTER_MEMODE)Gui_DrawFont_GBK16(95,50,TEXTCOLOR,BGCOLOR,"��ߴ�");
			
		}
	}
}





//---------------------------------------------------------
// screenչʾУ׼ҳ��
//---------------------------------------------------------
void ShowCorrectPage(void)
{	
	uint8_t *s;
	if(ScreenPageInfo.uPageReLoadFlag == CORRECT_PAGE)
	{
		Lcd_Clear(BGCOLOR);
		Gui_DrawFont_GBK16(45,20,TEXTCOLOR,BGCOLOR,"��׼��У��");
		//TODO 
		//ִ������У������
		Gui_DrawFont_GBK16(10,90,TEXTCOLOR,BGCOLOR,"����У��");
		ScreenPageInfo.uPageReLoadFlag = 0;
		PageCursor.uSelected = 0;
	}
	else
	{
		if(PageCursor.uSelected == 0)
		{
			//��ȡ����У��ֵ
			if(MeasureData.uMeasureFlag)
			{
				CorrectValue.lCorrectUpperLimitValue = getMeasureVoltage();
				s = Double2Char(CorrectValue.lCorrectUpperLimitValue,8);
				Gui_DrawFont_GBK16(60,55,TEXTCOLOR,BGCOLOR,s);   //����У��ֵ
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
			//��ȡ����У��ֵ
			if(MeasureData.uMeasureFlag)
			{
				CorrectValue.lCorrectLowerLimitValue = getMeasureVoltage();
				s = Double2Char(CorrectValue.lCorrectLowerLimitValue,8);
				Gui_DrawFont_GBK16(60,55,TEXTCOLOR,BGCOLOR,s);   //����У��ֵ
				free(s);
				MeasureData.uMeasureFlag = 0;
			}
			if(PageCursor.uNewSlected == 1)
			{
				Gui_DrawFont_GBK16(10,90,TEXTCOLOR,BGCOLOR,"����У��");
				Gui_DrawFont_GBK16(130,90,BGCOLOR,BGCOLOR,"ok");       //����OK
				PageCursor.uNewSlected = 0;
			}
		}
		else if(PageCursor.uSelected == 3)
		{
			if(PageCursor.uNewSlected == 1)
			{
				//TODO 
				//��ȡ
				//ִ�м���Kֵ����
				uint8_t corrected = ValueCorrect(CorrectValue.lCorrectLowerLimitValue,SettingData.lStandardLowerLimit,CorrectValue.lCorrectUpperLimitValue,SettingData.lStandardUpperLimit);
				if(corrected)
				{
					s = Double2Char(CorrectValue.lKValue,6);
					if(SettingData.lMeasureMode == OUTER_MEMODE)
					{
						Gui_DrawFont_GBK16(82,90,TEXTCOLOR,BGCOLOR,"k=-"); //����kֵ
						Gui_DrawFont_GBK16(110,90,TEXTCOLOR,BGCOLOR,s); //����kֵ
					}
					else
					{
						Gui_DrawFont_GBK16(82,90,TEXTCOLOR,BGCOLOR,"k="); //����kֵ
						Gui_DrawFont_GBK16(102,90,TEXTCOLOR,BGCOLOR,s); //����kֵ	
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
// screenչʾ��ѯҳ��
//---------------------------------------------------------
void ShowSearchPage(void)
{
	
	if(ScreenPageInfo.uPageReLoadFlag == SEARCH_PAGE)
	{
		Lcd_Clear(BGCOLOR);
		Gui_DrawFont_GBK16(55,20,TEXTCOLOR,BGCOLOR,"���ݲ�ѯ");
		Gui_DrawFont_GBK16(20,55,TEXTCOLOR,BGCOLOR,"���:");
		Gui_DrawFont_GBK16(60,55,TEXTCOLOR,BGCOLOR,"0001");
		Gui_DrawFont_GBK16(20,85,TEXTCOLOR,BGCOLOR,"����");
		Gui_DrawFont_GBK16(60,85,TEXTCOLOR,BGCOLOR,"23.0317");
		Gui_DrawFont_GBK16(130,85,TEXTCOLOR,BGCOLOR,"mm");
		ScreenPageInfo.uPageReLoadFlag = 0;
	}
	else
	{

	}
}





//---------------------------------------------------------
// screenչʾ��ѯҳ��
//---------------------------------------------------------
void ShowErrorPage(void)
{
	
	if(ScreenPageInfo.uPageReLoadFlag == ERROR_PAGE)
	{
		if(g_uErrorNo == ERROR_LIMIT)
		{
			Lcd_Clear(BGCOLOR);
			Gui_DrawFont_GBK16(55,20,TEXTERRORCOLOR,BGCOLOR,"������ʾ");
			Gui_DrawFont_GBK16(35,55,TEXTCOLOR,BGCOLOR,"ע��������ֵ");
		}
		else if(g_uErrorNo == ERROR_PARAM_1_NO_SET) //����δ����
		{
			Lcd_Clear(BGCOLOR);
			Gui_DrawFont_GBK16(55,20,TEXTERRORCOLOR,BGCOLOR,"������ʾ");
			Gui_DrawFont_GBK16(35,55,TEXTCOLOR,BGCOLOR,"���ù���ߴ�");
		}
		
		ScreenPageInfo.uPageReLoadFlag = 0;
	}
	else
	{

	}
}

//---------------------------------------------------------
// �˵���ʾҳ�洦����
//---------------------------------------------------------
void MenuPage_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:										
			if((PageCursor.uMenuPage_CurCursor > MEASURE_POINTER) && (PageCursor.uMenuPage_CurCursor <= SEARCH_POINTER))PageCursor.uMenuPage_CurCursor--;//������
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
			set_ScreenPageID(PageCursor.uMenuPage_CurCursor);	//�������ָ����ָλ�ý�����һ��ҳ��
			break;
		case ESC_PRESS:
			set_ScreenPageID(MEASURE_PAGE);    					//�˳���������ʾҳ��
			PageCursor.uMenuPage_CurCursor = MEASURE_POINTER;  	//��ʼ�����λ��
			break;
		default:
			break;
	}
}

//---------------------------------------------------------
// ����ҳ��1��������
//---------------------------------------------------------
void SettingPage1_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			if((PageCursor.uSelected == 1)&&(SettingData.lTempStationNo < 9))SettingData.lTempStationNo++;		//�༭��һ��վ�ż�һ

			break;
		case DOWN_PRESS:
			if((PageCursor.uSelected == 1)&&(SettingData.lTempStationNo > 0) && (SettingData.lTempStationNo <= 9))SettingData.lTempStationNo--;		//�༭��һ��վ�ż�һ
			break;
		case RIGHT_PRESS:
			break;
		case LEFT_PRESS:
			break;
		case ENTER_PRESS:
			if(PageCursor.uSelected == 1)
			{
				PageCursor.uSelected = 2 ;					//ȷ�ϼ����£�������һ�б༭
				SettingData.lStationNo = SettingData.lTempStationNo; 
				InitSettingData(SettingData.lStationNo);	//����վ�� ��ʼ���������ò���
				FlashWrite_CurStationNo(SettingData.lStationNo); //2021.0924���� ����Flash�б���ĵ�ǰվ��
				PageCursor.uNewSlected = 1;
			}
			else if(PageCursor.uSelected == 2)
			{
				//PageCursor.uNewSlected = 1;
				set_ScreenPageID(SETTING_PAGE_2);	//������һ���༭ҳ��
			}

			break;
		case ESC_PRESS:
		    //2021.09.24����
			if(SettingData.lNominalSize == 0) //û������
			{
				set_ScreenPageID(ERROR_PAGE);    		//�˳���Errorҳ��
				g_uErrorNo = ERROR_PARAM_1_NO_SET;
			}
			else
			{
				set_ScreenPageID(MENU_PAGE);    		//�˳���������ʾҳ��
			}
			break;
		default:
			break;
	}
}


//---------------------------------------------------------
// ����ҳ��2��������
//---------------------------------------------------------
void SettingPage2_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			//���ݼ�һ
			if(PageCursor.uSelected == 1)
			{
				if((SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] = '0';
			}
			break;
		case DOWN_PRESS:
			//���ݼ�һ
			if(PageCursor.uSelected == 1)
			{
				if((SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0]--; 
				else SettingData.nNominalSizeCharArr[PageCursor.uEditPos][0] = '9'; 
			}
			break;
		case RIGHT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 7))//�ڶ��б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case LEFT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos < 7)) //�ڶ��б༭λ������
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
				SettingData.lNominalSize = StrArr2Double(SettingData.nNominalSizeCharArr,8); //����༭�õ�����
				PageCursor.uNewSlected = 1;
				FlashUpdateSettingData(SettingData.lStationNo);
			}
			else if(PageCursor.uSelected == 2)
			{
				set_ScreenPageID(SETTING_PAGE_3);	//������һ���༭ҳ��
			}
			break;
		case ESC_PRESS:
			set_ScreenPageID(SETTING_PAGE_1);    		//�˳�������ҳ��1
			break;
		default:
			break;
	}
}


//---------------------------------------------------------
// ����ҳ��3��������
//---------------------------------------------------------
void SettingPage3_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			if(PageCursor.uSelected == 1 )				 //�༭��һ�� ���ּ�һ
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
				
			}		//������
			else if(PageCursor.uSelected == 2)			  //�༭�ڶ��� ���ּ�һ
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
			else if(PageCursor.uSelected == 3)			  //�༭��3�� ���ּ�һ
			{
				if((SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0]='0'; 		
			}
			else if(PageCursor.uSelected == 4)			  //�༭��4�� ���ּ�һ
			{
				if((SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0]='0'; 		
			}
			break;
		case DOWN_PRESS:
			if(PageCursor.uSelected == 1)					//�༭��һ�� ���ּ�һ
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
			else if(PageCursor.uSelected == 2)				//�༭�ڶ��� ���ּ�һ
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
			else if(PageCursor.uSelected == 3)				//�༭��3�� ���ּ�һ
			{

				if((SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0]--; 
				else SettingData.uTUpperLimitWarningCharArr[PageCursor.uEditPos][0]='9'; 
			}
			else if(PageCursor.uSelected == 4)				//�༭��4�� ���ּ�һ
			{

				if((SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0]--; 
				else SettingData.uTLowerLimitWarningCharArr[PageCursor.uEditPos][0]='9'; 
			}
			break;
		case RIGHT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 4))	//��һ�б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 2)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 4))//�ڶ��б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 3)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 3))//�ڶ��б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 4)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 3))//�ڶ��б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case LEFT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos < 4)) //��һ�б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 2)&&(PageCursor.uEditPos < 4))//�ڶ��б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 3)&&(PageCursor.uEditPos < 3))//�ڶ��б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 4)&&(PageCursor.uEditPos < 3))//�ڶ��б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case ENTER_PRESS:
			if(PageCursor.uSelected == 1)
			{
				PageCursor.uSelected = 2 ;				//ȷ�ϼ����£�������һ�б༭
				SettingData.lTempToleranceUpperLimit = StrArr2Double(SettingData.uToleranceUpperLimitCharArr,4); //����༭�õ�����
				SettingData.lToleranceUpperLimit = SettingData.lTempToleranceUpperLimit;
				FlashUpdateSettingData(SettingData.lStationNo);      //Flash���´洢ֵ
				PageCursor.uNewSlected = 1;	
			}
			else if(PageCursor.uSelected == 2)
			{
				SettingData.lTempToleranceLowerLimit = StrArr2Double(SettingData.uToleranceLowerLimitCharArr,4); //����༭�õ�����
				SettingData.lToleranceLowerLimit = SettingData.lTempToleranceLowerLimit;
				FlashUpdateSettingData(SettingData.lStationNo);
				PageCursor.uSelected = 3;
				PageCursor.uNewSlected = 1;
			}
			else if(PageCursor.uSelected == 3)
			{
				SettingData.lTempTUpperLimitWarning = StrArr2Double(SettingData.uTUpperLimitWarningCharArr,4); //����༭�õ�����	
				SettingData.lTUpperLimitWarning = SettingData.lTempTUpperLimitWarning;
				FlashUpdateSettingData(SettingData.lStationNo);
				PageCursor.uSelected = 4;
				PageCursor.uNewSlected = 1;
			}
			else if(PageCursor.uSelected == 4)
			{
				SettingData.lTempTLowerLimitWarning = StrArr2Double(SettingData.uTLowerLimitWarningCharArr,4); //����༭�õ�����	
				SettingData.lTLowerLimitWarning = SettingData.lTempTLowerLimitWarning;
				FlashUpdateSettingData(SettingData.lStationNo);
				PageCursor.uSelected = 5;
				PageCursor.uNewSlected = 1;
			}
			else if(PageCursor.uSelected == 5)
			{
				set_ScreenPageID(SETTING_PAGE_4);	//������һ������ҳ��
			}
			break;
		case ESC_PRESS:

//			else 
//			{
//				SettingData.lToleranceUpperLimit = SettingData.lTempToleranceUpperLimit;
//				SettingData.lToleranceLowerLimit = SettingData.lTempToleranceLowerLimit;
//				FlashUpdateSettingData(SettingData.lStationNo);
				set_ScreenPageID(SETTING_PAGE_2);    									//�˳���������ʾҳ��
//			}
			break;
		default:
			break;
	}
}




//---------------------------------------------------------
// ����ҳ��4��������
//---------------------------------------------------------
void SettingPage4_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			if(PageCursor.uSelected == 1)			//�༭��һ�� ���ּ�һ
			{
				if((SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0]='0'; 
			}		
			else if(PageCursor.uSelected == 2) 	//�༭�ڶ��� ���ּ�һ
			{
				if((SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0] >= '0')
				&&(SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0] < '9'))
				SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0]++; 
				else SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0]='0'; 
			}
			break;
		case DOWN_PRESS:
			if(PageCursor.uSelected == 1) 			//�༭��һ�� ���ּ�һ
			{
				if((SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0]--;
				else SettingData.uStandardUpperLimitCharArr[PageCursor.uEditPos][0]='9';
			}
			else if(PageCursor.uSelected == 2)		//�༭�ڶ��� ���ּ�һ
			{
				if((SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0] > '0')
				&&(SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0] <= '9'))
				SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0]--; 
				else SettingData.uStandardLowerLimitCharArr[PageCursor.uEditPos][0]='9'; 
			}
			break;
		case RIGHT_PRESS:	
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 7))		//��һ�б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 2)&&(PageCursor.uEditPos > 0) &&(PageCursor.uEditPos <= 7))//�ڶ��б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos--;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case LEFT_PRESS:
			if((PageCursor.uSelected == 1)&&(PageCursor.uEditPos < 7))			//��һ�б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			else if((PageCursor.uSelected == 2)&&(PageCursor.uEditPos < 7))		//�ڶ��б༭λ������
			{
				PageCursor.uLastEditPos = PageCursor.uEditPos;
				PageCursor.uEditPos++;
				PageCursor.uEditPosChangeFlag = 1;
			}
			break;
		case ENTER_PRESS:
			if(PageCursor.uSelected == 1)										//��һ�б༭ȷ����Ӧ
			{
				PageCursor.uSelected = 2 ;										//������һ�б༭
				SettingData.lTempStandardUpperLimit = StrArr2Double(SettingData.uStandardUpperLimitCharArr,8); //����༭�õ�����
				PageCursor.uNewSlected = 1;
				//FlashUpdateSettingData(SettingData.lStationNo);					//����Flash�洢��������
			}
			else if(PageCursor.uSelected == 2)
			{
				SettingData.lTempStandardLowerLimit = StrArr2Double(SettingData.uStandardLowerLimitCharArr,8); //����༭�õ�����
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
				FlashUpdateSettingData(SettingData.lStationNo);					//����Flash�洢��������
				set_ScreenPageID(SETTING_PAGE_5);	//�������ָ����ָλ�ý�����һ��ҳ��
			}
			break;
		case ESC_PRESS:
//			if(SettingData.lTempStandardLowerLimit>=SettingData.lTempStandardUpperLimit)set_ScreenPageID(ERROR_PAGE);
//			else 
//			{
				SettingData.lStandardUpperLimit = SettingData.lTempStandardUpperLimit;
				SettingData.lStandardLowerLimit = SettingData.lTempStandardLowerLimit;
				//�˳���������ʾҳ��
				FlashUpdateSettingData(SettingData.lStationNo);					//����Flash�洢��������
				set_ScreenPageID(SETTING_PAGE_3);  
//			}
			break;
		default:
			break;
	}
}
//---------------------------------------------------------
// ����ҳ��5��������
//---------------------------------------------------------
void SettingPage5_KeyResponse(uint8_t uPressKey)
{
	switch(uPressKey)
	{
		case UP_PRESS:	
			//ģʽѭ��
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
			//ģʽѭ��
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
			SettingData.lMeasureMode = SettingData.nTempMeasureMode;			//��������
			FlashUpdateSettingData(SettingData.lStationNo);						//����Flash����
			set_ScreenPageID(CORRECT_PAGE);    									//�˳���������ʾҳ��
			break;
		case ESC_PRESS:
			set_ScreenPageID(SETTING_PAGE_4);    									//�˳����˵���ʾҳ��
			break;
		default:
			break;
	}
}

//---------------------------------------------------------
// У׼ҳ�水������
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
			if(PageCursor.uSelected == 0 )set_ScreenPageID(MENU_PAGE);    									//�˳����˵���ʾҳ��
			break;
		default:
			break;
	}
}



//---------------------------------------------------------
// ������ĻID
//---------------------------------------------------------
void set_ScreenPageID(uint8_t pageId)
{
	ScreenPageInfo.uPageReLoadFlag = pageId;
	ScreenPageInfo.uPageID = pageId;
	PageCursor.uMenuPage_LastCursor = 0;
}

//---------------------------------------------------------
// ��ȡ��ĻID
//---------------------------------------------------------
uint8_t get_ScreenPageID(void)
{
	return ScreenPageInfo.uPageID ; //������ĻID
}


//---------------------------------------------------------
// ��Ļ��ʾ������
//---------------------------------------------------------
void ScreenProcess(void)
{
	uint8_t uScreenPageID = get_ScreenPageID();		//��ȡҳ��ID
	uint8_t uPressKey = KeyMonitor();  				//��������
	//����ҳ��ID��ʾ��Ӧ��ҳ�棬���в�ͬ�İ���������Ӧ
	switch(uScreenPageID)
	{
		//����ҳ��
		case MEASURE_PAGE:	 //Ĭ����������� 1				
		{
			if(uPressKey == ESC_PRESS) 			
			{
				set_ScreenPageID(MENU_PAGE);	//����˵�����
			}
			else if(uPressKey == LEFT_PRESS)  //�������
			{
				//��������  100�� 3��  һ�о���һ�����ݵ�  TODO
				_SAVE_DATA[nCur_Save_No][SAVE_INDEX_REAL] = MeasureData.lDeta;
				_SAVE_DATA[nCur_Save_No][SAVE_INDEX_MAX] = MeasureData.lMaxDeta;
				_SAVE_DATA[nCur_Save_No][SAVE_INDEX_MIN] = MeasureData.lMinDeta;
				nCur_Save_No++; //��ʼ��һ�����ݵ�ı���
			}
			else ShowMeasurePage();						//��ʾ����ҳ��
		}	break;
		//�˵�ҳ��
		case MENU_PAGE:						
		{
			if(uPressKey != NULL_PRESS)
			{			
				MenuPage_KeyResponse(uPressKey);		//�˵�ҳ�水����Ӧ����
			}
			else ShowMenuPage();						//��ʾ�˵�ҳ��
		}	break;	

		//����ҳ��1 վ�źͳ�������		
		case SETTING_PAGE_1:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage1_KeyResponse(uPressKey);	//����ҳ��1������Ӧ����
			}
			else ShowSettingPage1();					//��ʾ����ҳ��1
			
		}	break;	
		//����ҳ��2 ���̺͹��̳ߴ�����
		case SETTING_PAGE_2:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage2_KeyResponse(uPressKey);	//����ҳ��2������Ӧ����
			}
			else ShowSettingPage2();					//��ʾ����ҳ��2
			
		}	break;	
		//����ҳ��3 ��������������
		case SETTING_PAGE_3:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;//�ȼ�����һ�ε�id��
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage3_KeyResponse(uPressKey);	//����ҳ��3������Ӧ����
			}
			else ShowSettingPage3();					//��ʾ����ҳ��3
			
		}	break;	
		//����ҳ��4 ��׼������������
		case SETTING_PAGE_4:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage4_KeyResponse(uPressKey);	//����ҳ��4������Ӧ����
			}
			else ShowSettingPage4();					//��ʾ����ҳ��4
			
		}	break;	
		//����ҳ��5 ����ģʽѡ��
		case SETTING_PAGE_5:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{			
				SettingPage5_KeyResponse(uPressKey);	//����ҳ��5������Ӧ����
			}
			else ShowSettingPage5();					//��ʾ����ҳ��5
			
		}	break;	
		//����ҳ��6 Ԥ��ֵѡ��
		case SETTING_PAGE_6:			
		{
//			ScreenPageInfo.uLastPageID = uScreenPageID;
//			if(uPressKey != NULL_PRESS)
//			{			
//				SettingPage5_KeyResponse(uPressKey);	//����ҳ��5������Ӧ����
//			}
//			else ShowSettingPage5();					//��ʾ����ҳ��5
			
		}	break;	
		//����ҳ��
		case CORRECT_PAGE:			
		{
			ScreenPageInfo.uLastPageID = uScreenPageID;
			if(uPressKey != NULL_PRESS)
			{
				CorrectPage_KeyResponse(uPressKey);		//����ҳ�水����Ӧ����
			}
			else ShowCorrectPage();						//��ʾУ��ҳ��
			
		}	break;
		//��ѯҳ��		
		case SEARCH_PAGE:			
		{
			if(uPressKey == ESC_PRESS)
			{
				set_ScreenPageID(MENU_PAGE);			//��ѯҳ�水����Ӧ����
			}
			else ShowSearchPage();						//��ʾ��ѯҳ��
			
		}	break;
		case ERROR_PAGE:
			if((uPressKey == ESC_PRESS)||(uPressKey == ENTER_PRESS))
			{
				set_ScreenPageID(ScreenPageInfo.uLastPageID);			//errorҳ�水����Ӧ����
				g_uErrorNo = ERROR_RELEASE;
			}
			else ShowErrorPage();					//��ʾerrorҳ��
		default:
			break;
	}
}




