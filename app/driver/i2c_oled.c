/*
* 0.96 OLED  锟斤拷示锟斤拷锟斤拷
*/
//include锟侥硷拷锟斤拷锟斤拷锟斤拷锟饺伙拷锟斤拷头锟侥硷拷锟斤拷呒锟斤拷锟酵凤拷募锟斤拷锟斤拷苑锟斤拷锟绞撅拷锟斤拷直锟斤拷锟街拷锟侥讹拷锟斤拷未锟斤拷锟斤拷锟斤拷锟斤拷锟�

#include "i2c_oled.h"

#include "i2c_master.h"
#include "i2c_oled_fonts.h"

#define MAX_SMALL_FONTS 21 //小锟斤拷锟斤拷
#define MAX_BIG_FONTS 16 //锟斤拷锟斤拷锟斤拷

static bool oledstat = false;//oled状态锟斤拷始锟斤拷识为锟斤拷


//锟斤拷某锟侥达拷锟斤拷锟斤拷写锟斤拷锟斤拷锟筋（锟斤拷锟捷ｏ拷锟斤拷锟斤拷位锟侥达拷锟斤拷实锟斤拷锟截讹拷锟斤拷锟斤拷
bool ICACHE_FLASH_ATTR
OLED_writeReg(uint8_t reg_addr,uint8_t val)
{
  //开始i2c通信
  i2c_master_start();
  //写OLED从机地址
  i2c_master_writeByte(OLED_ADDRESS);
  //执行从机应答检测函数，如果从机发送了非应答信号，那么就退出数据发送函数
  if(!i2c_master_checkAck()) {
	  i2c_master_stop();
    return 0;
  }
  //写入指令地址
  i2c_master_writeByte(reg_addr);
  //执行从机应答检测函数，如果从机发送了非应答信号，那么就退出数据发送函数
  if(!i2c_master_checkAck()) {
	  i2c_master_stop();
    return 0;
  }
  //向地址写入数据
  i2c_master_writeByte(val&0xff);
  //执行从机应答检测函数，如果从机发送了非应答信号，那么就退出数据发送函数
  if(!i2c_master_checkAck()) {
	  i2c_master_stop();
    return 0;
  }
  //停止iic通信
  i2c_master_stop();

  //锟斤拷锟斤拷oled状态为锟芥，锟斤拷锟斤拷始锟斤拷锟斤拷希锟斤拷锟斤拷锟斤拷锟�
  if (reg_addr==0x00)
    oledstat=true;

  return 1;
}

//锟斤拷锟斤拷示锟斤拷锟狡寄达拷锟斤拷锟斤拷写锟斤拷锟筋，锟斤拷锟斤拷锟斤拷值锟斤拷锟斤拷锟斤拷锟斤拷锟�0.96OLED锟斤拷示锟斤拷_锟斤拷锟斤拷芯片锟街册”锟侥碉拷锟斤拷28页
void ICACHE_FLASH_ATTR
OLED_writeCmd(unsigned char I2C_Command)
{
  OLED_writeReg(0x00,I2C_Command);
}

//写锟斤拷锟斤拷示锟斤拷锟斤拷锟捷ｏ拷也锟斤拷锟角碉拷锟斤拷要锟斤拷示锟侥分憋拷锟绞碉拷
void ICACHE_FLASH_ATTR
OLED_writeDat(unsigned char I2C_Data)
{
	OLED_writeReg(0x40,I2C_Data);
}

//设置起始点坐标
void ICACHE_FLASH_ATTR
OLED_SetPos(unsigned char x, unsigned char y)
{
	OLED_writeCmd(0xb0+y);
	OLED_writeCmd(((x&0xf0)>>4)|0x10);
	OLED_writeCmd((x&0x0f)|0x01);
}

//全屏填充
void ICACHE_FLASH_ATTR
OLED_Fill(unsigned char fill_Data)
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		OLED_writeCmd(0xb0+m);		//page0-page1
		OLED_writeCmd(0x00);		//low column start address
		OLED_writeCmd(0x10);		//high column start address
		for(n=0;n<132;n++)
			{
				OLED_writeDat(fill_Data);
			}
	}
}

//清屏
void ICACHE_FLASH_ATTR
OLED_CLS(void)
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          :
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void ICACHE_FLASH_ATTR
OLED_ON(void)
{
	OLED_writeCmd(0X8D);
	OLED_writeCmd(0X14);
	OLED_writeCmd(0XAF);
}


//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          :
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void ICACHE_FLASH_ATTR
OLED_OFF(void)
{
	OLED_writeCmd(0X8D);
	OLED_writeCmd(0X10);
	OLED_writeCmd(0XAE);
}


//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          :
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void ICACHE_FLASH_ATTR
OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					OLED_writeDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					OLED_writeDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					OLED_writeDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          :
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
void ICACHE_FLASH_ATTR
OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,const unsigned char BMP[])//
{
	unsigned int j=0;
	unsigned char x,y;

	if (x1>128)
		x1=128;
	if (y1>8)
		y1=8;

	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			OLED_writeDat(BMP[j++]);
		}
	}
}


//Oled锟斤拷始锟斤拷锟斤拷夭锟斤拷锟斤拷锟斤拷慰锟�51锟斤拷锟教猴拷锟叫撅拷园锟侥碉拷锟叫革拷锟斤拷锟侥诧拷锟借，锟斤拷锟斤拷为什么锟斤拷锟斤拷锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷锟剿ｏ拷也锟斤拷锟斤拷模锟斤拷锟斤拷锟斤拷娑拷锟�
bool ICACHE_FLASH_ATTR
oled_init(void)
{
  //延时100ms
  os_delay_us(60000);
  os_delay_us(40000);


  OLED_writeCmd(0xAE);   //display off
  OLED_writeCmd(0x20);        //Set Memory Addressing Mode
  OLED_writeCmd(0x10);        //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
  OLED_writeCmd(0xb0);        //Set Page Start Address for Page Addressing Mode,0-7
  OLED_writeCmd(0xc8);        //Set COM Output Scan Direction
  OLED_writeCmd(0x00);//---set low column address
  OLED_writeCmd(0x10);//---set high column address
  OLED_writeCmd(0x40);//--set start line address
  OLED_writeCmd(0x81);//--set contrast control register
  OLED_writeCmd(0x7f);//亮度调节 0x00~0xff
  OLED_writeCmd(0xa1);//--set segment re-map 0 to 127
  OLED_writeCmd(0xa6);//--set normal display
  OLED_writeCmd(0xa8);//--set multiplex ratio(1 to 64)
  OLED_writeCmd(0x3F);//
  OLED_writeCmd(0xa4);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
  OLED_writeCmd(0xd3);//-set display offset
  OLED_writeCmd(0x00);//-not offset
  OLED_writeCmd(0xd5);//--set display clock divide ratio/oscillator frequency
  OLED_writeCmd(0xf0);//--set divide ratio
  OLED_writeCmd(0xd9);//--set pre-charge period
  OLED_writeCmd(0x22); //
  OLED_writeCmd(0xda);//--set com pins hardware configuration
  OLED_writeCmd(0x12);
  OLED_writeCmd(0xdb);//--set vcomh
  OLED_writeCmd(0x20);//0x20,0.77xVcc
  OLED_writeCmd(0x8d);//--set DC-DC enable
  OLED_writeCmd(0x14);//
  OLED_writeCmd(0xaf);//--turn on oled panel

  OLED_Fill(0x00);  //清屏


  OLED_ShowStr(0, 3, "System Start", 2);

  //显示延时600ms
  os_delay_us(60000);
  os_delay_us(60000);
  os_delay_us(60000);
  os_delay_us(60000);
  os_delay_us(60000);
  os_delay_us(60000);
  os_delay_us(60000);
  os_delay_us(60000);
  os_delay_us(60000);
  os_delay_us(60000);
  //清屏
  OLED_CLS();
//    显示位图，自左上角0,0，位置分辨率点起，至第128个分辨率点、第8行的右下角，显示图片“esp8266 天气预报”的位图数据
  OLED_DrawBMP(0, 0, 128, 8, vankia_BMP);


  return oledstat;
}


