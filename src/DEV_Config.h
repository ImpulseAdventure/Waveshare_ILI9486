/******************************************************************************
**************************Hardware interface layer*****************************
* | file          :   DEV_Config.h
* | version     :   V1.0
* | date        :   2017-12-11
* | function    :
    Provide the hardware underlying interface
******************************************************************************/
#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include <SPI.h>
#include <Wire.h>

#define USE_SPI_4W 1
#define USE_IIC    0

//GPIO config
//LCD
#define LCD_CS 10
#define LCD_CS_0        digitalWrite(LCD_CS, LOW)
#define LCD_CS_1        digitalWrite(LCD_CS, HIGH)

#define LCD_BL 9

#define LCD_RST 8
#define LCD_RST_0       digitalWrite(LCD_RST, LOW)
#define LCD_RST_1       digitalWrite(LCD_RST, HIGH)

#define LCD_DC 7
#define LCD_DC_0        digitalWrite(LCD_DC, LOW)
#define LCD_DC_1        digitalWrite(LCD_DC, HIGH)

//Touch
#define TP_CS 4
#define TP_CS_0    digitalWrite(TP_CS, LOW)
#define TP_CS_1    digitalWrite(TP_CS, HIGH)

#define TP_IRQ 3
#define GET_TP_IRQ    digitalRead(TP_IRQ)

#define TP_BUSY 6
#define GET_TP_BUSY    digitalRead(TP_BUSY)

#define SPI4W_Write_Byte(__DATA) SPI.transfer(__DATA)
#define SPI4W_Read_Byte(__DATA) SPI.transfer(__DATA)
/*------------------------------------------------------------------------------------------------------*/
uint8_t System_Init(void);
void PWM_SetValue(uint16_t value);

void Driver_Delay_ms(unsigned long xms);
void Driver_Delay_us(int xus);

#endif

