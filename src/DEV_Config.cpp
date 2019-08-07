/******************************************************************************
**************************Hardware interface layer*****************************
* | file          :   DEV_Config.c
* | version     :   V1.0
* | date        :   2017-12-11
* | function    :
    Provide the hardware underlying interface
******************************************************************************/
#include "DEV_Config.h"
#include <SPI.h>

/********************************************************************************
  function:    System Init and exit
  note:
    Initialize the communication method
********************************************************************************/
uint8_t System_Init(void)
{
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_DC, OUTPUT);
  pinMode(TP_CS, OUTPUT);
  pinMode(TP_IRQ, INPUT);
  digitalWrite(TP_IRQ, HIGH);

  // NOTE: Reduced serial port baud rate to match
  //       common default setting. Was 115200 bps.
  //Serial.begin(115200);
  Serial.begin(9600);
  
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();

  return 0;
}

void PWM_SetValue(uint16_t value)
{
    pinMode(LCD_BL, OUTPUT);
    analogWrite(LCD_BL, value);
}

/********************************************************************************
  function:    Delay function
  note:
    Driver_Delay_ms(xms) : Delay x ms
    Driver_Delay_us(xus) : Delay x us
********************************************************************************/
void Driver_Delay_ms(unsigned long xms)
{
  delay(xms);
}

void Driver_Delay_us(int xus)
{
  for (int j = xus; j > 0; j--);
}
