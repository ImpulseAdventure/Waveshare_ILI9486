#include "DEV_Config.h"
#include "LCD_Driver.h"
#include "LCD_GUI.h"
#include "LCD_Bmp.h"

void setup()
{
  System_Init();

  Serial.println("3.5inch TFT Touch Shield ShowBMP Demo");
  Serial.println("SD_Init ");
  SD_Init();

  Serial.println("LCD Init ");
  LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;
  LCD_Init(Lcd_ScanDir, 200);

  Serial.println("LCD_Clear ");
  LCD_Clear(LCD_BACKGROUND);

  LCD_ShowBMP();
}

void loop()
{

}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

