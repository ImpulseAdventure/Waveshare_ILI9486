#include "DEV_Config.h"
#include "LCD_Driver.h"
#include "Waveshare_ILI9486.h"

void setup()
{
  System_Init();

  Serial.println("4inch　TFT　Touch Shiled LCD Show...");  
  Serial.println("LCD Init...");
  LCD_SCAN_DIR Lcd_ScanDir = SCAN_DIR_DFT;  
  LCD_Init( Lcd_ScanDir, 100);

  Serial.println("LCD_Clear...");
  LCD_Clear(LCD_BACKGROUND);

  Serial.println("LCD_Show...");
  GUI_Show();
}

void loop()
{
  DEV_TIME sDev_time;
  sDev_time.Hour = 23;
  sDev_time.Min = 38;
  sDev_time.Sec = 56;

  Serial.print("Show time\r\n");
  for (;;) {
    sDev_time.Sec++;
    GUI_Showtime(200, 150, 327, 197, &sDev_time, RED);
    Driver_Delay_ms(200);//Analog clock 1s
    if (sDev_time.Sec == 60)
      sDev_time.Sec = 0;
  }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/

