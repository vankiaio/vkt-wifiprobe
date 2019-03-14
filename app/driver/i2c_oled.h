
#ifndef __I2C_OLED_H
#define	__I2C_OLED_H

#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

//����OLED��ʾ����Ϊ���豸��ַ������ʾ��ģ�鱳����躸�ӵ���һ�ߣ���ַ����ʲô
#define OLED_ADDRESS	0x78  // D/C->GND
//#define OLED_ADDRESS	0x7a // D/C->Vcc

//����ʾ���ƼĴ�����д����
void OLED_writeCmd(unsigned char I2C_Command);
//д����
void OLED_writeDat(unsigned char I2C_Data);
//Oled��ʼ����ز�����main�ļ��п�ʼ�����ʼ��һ��
bool oled_init(void);
//������Ļ�ϵ���ʾλ��
void OLED_SetPos(unsigned char x, unsigned char y);
//��Ļ���
void OLED_Fill(unsigned char fill_Data);
//����
void OLED_CLS(void);
//����Ļ
void OLED_ON(void);
//�ر���Ļ
void OLED_OFF(void);
//����Ļ�ϵ�ָ��λ����ʾ��ָ�����ַ���������Ϊ��λ�ã���0�𣩡���λ�ã���0�𣩣�����ʾ�ַ����������С��1��С����2�Ǵ����壩
//С����ʱ�������ʾ21���ַ���128/6��,�����8�У�0-7����������ʱ�������ʾ16���ַ���128/8���������4�У�ռ��С��������У�Ҳ����˵���������к�����ŵ���һ����ʾҪ��һ�У���1����ʾ�˴����壬��3�в�����ʾ����ַ���2����ʾ���ռ��1�д�������°벿�֣�
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);

//��ʾλͼ����ȡģ���ߵ�λͼ���ݣ�x0Ϊ��ʼ�еĵ㣨0-127����y0Ϊ��ʼ�еĵĵ㣨0-7����x1/y1�ֱ�Ϊ��ֹ�еĵ㣬BMPΪλͼ����
//���Խ��������Ϊ��С���ʵ�λͼ���ں��ʵ�λ����ʾ
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,const unsigned char BMP[]);//

#endif
