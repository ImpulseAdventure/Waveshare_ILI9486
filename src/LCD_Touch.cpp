/*****************************************************************************
  | File          :   LCD_Touch.c
  | Author      :   Waveshare team
  | Function    :   LCD Touch Pad Driver and Draw
  | Info        :
    Image scanning
       Please use progressive scanning to generate images or fonts
  ----------------
  | This version:   V1.0
  | Date        :   2017-08-16
  | Info        :   Basic version

******************************************************************************/
#include "LCD_Touch.h"
#include "Debug.h"
#include <stdlib.h>
#include <math.h>

extern LCD_DIS sLCD_DIS;
static TP_DEV sTP_DEV;
static TP_DRAW sTP_Draw;
/*******************************************************************************
  function:
        Read the ADC of the channel
  parameter:
    Channel_Cmd :   0x90: Read channel Y +, select the ADC resolution is 12 bits, set to differential mode
                    0xd0: Read channel x +, select the ADC resolution is 12 bits, set to differential mode
*******************************************************************************/
static unsigned int TP_Read_ADC(unsigned char CMD)
{
  unsigned int Data = 0;

  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.begin();

  TP_CS_0;

  SPI4W_Write_Byte(CMD);
  Driver_Delay_us(200);

  Data = SPI4W_Read_Byte(0XFF);
  Data <<= 8;//7bit
  Data |= SPI4W_Read_Byte(0XFF);
  Data >>= 3;//5bit
  TP_CS_1;

  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();

  return Data;
}

/*******************************************************************************
  function:
        Read the 5th channel value and exclude the maximum and minimum returns the average
  parameter:
    Channel_Cmd :   0x90 :Read channel Y +
                    0xd0 :Read channel x +
*******************************************************************************/
#define READ_TIMES  5   //Number of readings
#define LOST_NUM    1   //Discard value
static unsigned int TP_Read_ADC_Average(unsigned char Channel_Cmd)
{
  unsigned char i, j;
  unsigned int Read_Buff[READ_TIMES];
  unsigned int Read_Sum = 0, Read_Temp = 0;

  //Read and save multiple samples
  for (i = 0; i < READ_TIMES; i++) {
    Read_Buff[i] = TP_Read_ADC(Channel_Cmd);
    Driver_Delay_us(200);
  }

  //Sort from small to large
  for (i = 0; i < READ_TIMES  -  1; i ++) {
    for (j = i + 1; j < READ_TIMES; j ++) {
      if (Read_Buff[i] > Read_Buff[j]) {
        Read_Temp = Read_Buff[i];
        Read_Buff[i] = Read_Buff[j];
        Read_Buff[j] = Read_Temp;
      }
    }
  }

  //Exclude the largest and the smallest
  for (i = LOST_NUM; i < READ_TIMES - LOST_NUM; i ++)
    Read_Sum += Read_Buff[i];

  //Averaging
  Read_Temp = Read_Sum / (READ_TIMES - 2 * LOST_NUM);

  return Read_Temp;
}

/*******************************************************************************
  function:
        Read X channel and Y channel AD value
  parameter:
    Channel_Cmd :   0x90 :Read channel Y +
                    0xd0 :Read channel x +
*******************************************************************************/
static void TP_Read_ADC_XY(unsigned int *pXCh_Adc, unsigned int  *pYCh_Adc )
{
  *pXCh_Adc = TP_Read_ADC_Average(0xD0);
  *pYCh_Adc = TP_Read_ADC_Average(0x90);
}

/*******************************************************************************
  function:
        2 times to read the touch screen IC, and the two can not exceed the deviation,
        ERR_RANGE, meet the conditions, then that the correct reading, otherwise the reading error.
  parameter:
    Channel_Cmd :   pYCh_Adc = 0x90 :Read channel Y +
                    pXCh_Adc = 0xd0 :Read channel x +
*******************************************************************************/
#define ERR_RANGE 50    //tolerance scope
static bool TP_Read_TwiceADC(unsigned int *pXCh_Adc, unsigned int  *pYCh_Adc )
{
  unsigned int XCh_Adc1, YCh_Adc1, XCh_Adc2, YCh_Adc2;

  //Read the ADC values Read the ADC values twice
  TP_Read_ADC_XY(&XCh_Adc1, &YCh_Adc1);
  //  Driver_Delay_us(10);
  TP_Read_ADC_XY(&XCh_Adc2, &YCh_Adc2);
  //  Driver_Delay_us(10);

  //The ADC error used twice is greater than ERR_RANGE to take the average
  if ( ((XCh_Adc2 <= XCh_Adc1 && XCh_Adc1 < XCh_Adc2 + ERR_RANGE) ||
        (XCh_Adc1 <= XCh_Adc2 && XCh_Adc2 < XCh_Adc1 + ERR_RANGE))
       && ((YCh_Adc2 <= YCh_Adc1 && YCh_Adc1 < YCh_Adc2 + ERR_RANGE) ||
           (YCh_Adc1 <= YCh_Adc2 && YCh_Adc2 < YCh_Adc1 + ERR_RANGE))) {
    *pXCh_Adc = (XCh_Adc1 + XCh_Adc2) / 2;
    *pYCh_Adc = (YCh_Adc1 + YCh_Adc2) / 2;
    return true;
  }

  //The ADC error used twice is less than ERR_RANGE returns failed
  return false;
}

/*******************************************************************************
  function:
        Calculation
  parameter:
        chCoordType:
                    1 : calibration
                    0 : relative position
*******************************************************************************/
static unsigned char TP_Scan(unsigned char chCoordType)
{
  //In X, Y coordinate measurement, IRQ is disabled and output is low
  if (!GET_TP_IRQ) {//Press the button to press
    //Read the physical coordinates
    if (chCoordType) {
      TP_Read_TwiceADC(&sTP_DEV.Xpoint, &sTP_DEV.Ypoint);
      //Read the screen coordinates
    } else if (TP_Read_TwiceADC(&sTP_DEV.Xpoint, &sTP_DEV.Ypoint)) {
      //          DEBUG("(Xad,Yad) = %d,%d\r\n",sTP_DEV.Xpoint,sTP_DEV.Ypoint);
      if (sTP_DEV.TP_Scan_Dir == R2L_D2U) {       //Converts the result to screen coordinates
        sTP_Draw.Xpoint = sTP_DEV.fXfac * sTP_DEV.Xpoint +
                          sTP_DEV.iXoff;
        sTP_Draw.Ypoint = sTP_DEV.fYfac * sTP_DEV.Ypoint +
                          sTP_DEV.iYoff;
      } else if (sTP_DEV.TP_Scan_Dir == L2R_U2D) {
        sTP_Draw.Xpoint = sLCD_DIS.LCD_Dis_Column -
                          sTP_DEV.fXfac * sTP_DEV.Xpoint -
                          sTP_DEV.iXoff;
        sTP_Draw.Ypoint = sLCD_DIS.LCD_Dis_Page -
                          sTP_DEV.fYfac * sTP_DEV.Ypoint -
                          sTP_DEV.iYoff;
      } else if (sTP_DEV.TP_Scan_Dir == U2D_R2L) {
        sTP_Draw.Xpoint = sTP_DEV.fXfac * sTP_DEV.Ypoint +
                          sTP_DEV.iXoff;
        sTP_Draw.Ypoint = sTP_DEV.fYfac * sTP_DEV.Xpoint +
                          sTP_DEV.iYoff;
      } else {
        sTP_Draw.Xpoint = sLCD_DIS.LCD_Dis_Column -
                          sTP_DEV.fXfac * sTP_DEV.Ypoint -
                          sTP_DEV.iXoff;
        sTP_Draw.Ypoint = sLCD_DIS.LCD_Dis_Page -
                          sTP_DEV.fYfac * sTP_DEV.Xpoint -
                          sTP_DEV.iYoff;
      }
      //          DEBUG("( x , y ) = %d,%d\r\n",sTP_Draw.Xpoint,sTP_Draw.Ypoint);
    }
    if (0 == (sTP_DEV.chStatus & TP_PRESS_DOWN)) {  //Not being pressed
      sTP_DEV.chStatus = TP_PRESS_DOWN | TP_PRESSED;
      sTP_DEV.Xpoint0 = sTP_DEV.Xpoint;
      sTP_DEV.Ypoint0 = sTP_DEV.Ypoint;
    }
  } else {
    if (sTP_DEV.chStatus & TP_PRESS_DOWN) { //0x80
      sTP_DEV.chStatus &= ~(1 << 7);      //0x00
    } else {
      sTP_DEV.Xpoint0 = 0;
      sTP_DEV.Ypoint0 = 0;
      sTP_DEV.Xpoint = 0xffff;
      sTP_DEV.Ypoint = 0xffff;
    }
  }

  return (sTP_DEV.chStatus & TP_PRESS_DOWN);
}

/*******************************************************************************
  function:
        Draw Cross
  parameter:
            Xpoint :    The x coordinate of the point
            Ypoint :    The y coordinate of the point
            Color  :    Set color
*******************************************************************************/
static void TP_DrawCross(POINT Xpoint, POINT Ypoint, COLOR Color)
{
  GUI_DrawLine(Xpoint  -  12, Ypoint, Xpoint + 12, Ypoint,
               Color, LINE_SOLID, DOT_PIXEL_1X1);
  GUI_DrawLine(Xpoint, Ypoint  -  12, Xpoint, Ypoint + 12,
               Color, LINE_SOLID, DOT_PIXEL_1X1);
  GUI_DrawPoint(Xpoint, Ypoint, Color, DOT_PIXEL_2X2 , DOT_FILL_AROUND);
  GUI_DrawCircle(Xpoint, Ypoint, 6, Color, DRAW_EMPTY, DOT_PIXEL_1X1);
}


/*******************************************************************************
  function:
        The corresponding ADC value is displayed on the LC
  parameter:
            (Xpoint0 ,Xpoint0): The coordinates of the first point
            (Xpoint1 ,Xpoint1): The coordinates of the second point
            (Xpoint2 ,Xpoint2): The coordinates of the third point
            (Xpoint3 ,Xpoint3): The coordinates of the fourth point
            hwFac   :   Percentage of error
*******************************************************************************/
static void TP_ShowInfo(POINT Xpoint0, POINT Ypoint0,
                        POINT Xpoint1, POINT Ypoint1,
                        POINT Xpoint2, POINT Ypoint2,
                        POINT Xpoint3, POINT Ypoint3,
                        POINT hwFac)
{
  sFONT* TP_Font = &Font16;
  LENGTH TP_Dx =  TP_Font->Width;

  GUI_DrawRectangle(40, 160, 250, 270, WHITE, DRAW_FULL, DOT_PIXEL_1X1);

  GUI_DisString_EN(40, 160, "x1", TP_Font, FONT_BACKGROUND, RED);
  GUI_DisString_EN(40 + 100, 160, "y1", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisString_EN(40, 180, "x2", TP_Font, FONT_BACKGROUND, RED);
  GUI_DisString_EN(40 + 100, 180, "y2", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisString_EN(40, 200, "x3", TP_Font, FONT_BACKGROUND, RED);
  GUI_DisString_EN(40 + 100, 200, "y3", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisString_EN(40, 220, "x4", TP_Font, FONT_BACKGROUND, RED);
  GUI_DisString_EN(40 + 100, 220, "y4", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisString_EN(40, 240, "fac is : ", TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 3 * TP_Dx, 160, Xpoint0, TP_Font, FONT_BACKGROUND, RED);
  GUI_DisNum(40 + 3 * TP_Dx + 100, 160, Ypoint0, TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 3 * TP_Dx, 180, Xpoint1, TP_Font, FONT_BACKGROUND, RED);
  GUI_DisNum(40 + 3 * TP_Dx + 100, 180, Ypoint1, TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 3 * TP_Dx, 200, Xpoint2, TP_Font, FONT_BACKGROUND, RED);
  GUI_DisNum(40 + 3 * TP_Dx + 100, 200, Ypoint2, TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 3 * TP_Dx, 220, Xpoint3, TP_Font, FONT_BACKGROUND, RED);
  GUI_DisNum(40 + 3 * TP_Dx + 100, 220, Ypoint3, TP_Font, FONT_BACKGROUND, RED);

  GUI_DisNum(40 + 10 * TP_Dx , 240, hwFac, TP_Font, FONT_BACKGROUND, RED);
}

/*******************************************************************************
  function:
        Touch screen adjust
*******************************************************************************/
void TP_Adjust(void)
{
  unsigned char  cnt = 0;
  unsigned int XYpoint_Arr[4][2];
  uint32_t Dx, Dy;
  unsigned int Sqrt1, Sqrt2;
  float Dsqrt;

  LCD_Clear(LCD_BACKGROUND);
  GUI_DisString_EN(0, 60, "Please use the stylus click the cross"\
                   "on the screen. The cross will always move until"\
                   "the screen adjustment is completed.",
                   &Font16, FONT_BACKGROUND, RED);

  unsigned char Mar_Val = 12;
  TP_DrawCross(Mar_Val, Mar_Val, RED);

  sTP_DEV.chStatus = 0;

  while (1) {
    TP_Scan(1);
    if ((sTP_DEV.chStatus & 0xC0) == TP_PRESSED) {
      sTP_DEV.chStatus &= ~(1 << 6);
      XYpoint_Arr[cnt][0] = sTP_DEV.Xpoint;
      XYpoint_Arr[cnt][1] = sTP_DEV.Ypoint;
      //          DEBUG("X%d,Y%d = %d,%d\r\n",
      //                cnt,cnt,
      //                XYpoint_Arr[cnt][0],
      //                XYpoint_Arr[cnt][1]);
      cnt ++;
      Driver_Delay_ms(200);

      switch (cnt) {
        case 1:
          TP_DrawCross(Mar_Val, Mar_Val, WHITE);
          TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val, Mar_Val, RED);
          Driver_Delay_ms(200);
          break;
        case 2:
          TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val, Mar_Val, WHITE);
          TP_DrawCross(Mar_Val, sLCD_DIS.LCD_Dis_Page - Mar_Val, RED);
          Driver_Delay_ms(200);
          break;
        case 3:
          TP_DrawCross(Mar_Val, sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
          TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                       sLCD_DIS.LCD_Dis_Page - Mar_Val, RED);
          Driver_Delay_ms(200);
          break;
        case 4:

          // 1.Compare the X direction
          Dx = abs((int16_t)(XYpoint_Arr[0][0] -
                             XYpoint_Arr[1][0]));//x1 - x2
          Dy = abs((int16_t)(XYpoint_Arr[0][1] -
                             XYpoint_Arr[1][1]));//y1 - y2
          Dx *= Dx;
          Dy *= Dy;
          Sqrt1 = sqrt(Dx + Dy);

          Dx = abs((int16_t)(XYpoint_Arr[2][0] -
                             XYpoint_Arr[3][0]));//x3 - x4
          Dy = abs((int16_t)(XYpoint_Arr[2][1] -
                             XYpoint_Arr[3][1]));//y3 - y4
          Dx *= Dx;
          Dy *= Dy;
          Sqrt2 = sqrt(Dx + Dy);

          Dsqrt = (float)Sqrt1 / Sqrt2;
          if (Dsqrt < 0.95 || Dsqrt > 1.05 || Sqrt1 == 0 || Sqrt2 == 0) {
            DEBUG("Adjust X direction \r\n");
            cnt = 0;
            TP_ShowInfo(XYpoint_Arr[0][0], XYpoint_Arr[0][1],
                        XYpoint_Arr[1][0], XYpoint_Arr[1][1],
                        XYpoint_Arr[2][0], XYpoint_Arr[2][1],
                        XYpoint_Arr[3][0], XYpoint_Arr[3][1],
                        Dsqrt * 100);
            Driver_Delay_ms(1000);
            TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                         sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
            TP_DrawCross(Mar_Val, Mar_Val, RED);
            continue;
          }

          // 2.Compare the Y direction
          Dx = abs((int16_t)(XYpoint_Arr[0][0] -
                             XYpoint_Arr[2][0]));//x1 - x3
          Dy = abs((int16_t)(XYpoint_Arr[0][1] -
                             XYpoint_Arr[2][1]));//y1 - y3
          Dx *= Dx;
          Dy *= Dy;
          Sqrt1 = sqrt(Dx + Dy);

          Dx = abs((int16_t)(XYpoint_Arr[1][0] -
                             XYpoint_Arr[3][0]));//x2 - x4
          Dy = abs((int16_t)(XYpoint_Arr[1][1] -
                             XYpoint_Arr[3][1]));//y2 - y4
          Dx *= Dx;
          Dy *= Dy;
          Sqrt2 = sqrt(Dx + Dy);//

          Dsqrt = (float)Sqrt1 / Sqrt2;
          if (Dsqrt < 0.95 || Dsqrt > 1.05) {
            DEBUG("Adjust Y direction \r\n");
            cnt = 0;
            TP_ShowInfo(XYpoint_Arr[0][0], XYpoint_Arr[0][1],
                        XYpoint_Arr[1][0], XYpoint_Arr[1][1],
                        XYpoint_Arr[2][0], XYpoint_Arr[2][1],
                        XYpoint_Arr[3][0], XYpoint_Arr[3][1],
                        Dsqrt * 100);
            Driver_Delay_ms(1000);
            TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                         sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
            TP_DrawCross(Mar_Val, Mar_Val, RED);
            continue;
          }//

          //3.Compare diagonal
          Dx = abs((int16_t)(XYpoint_Arr[1][0] -
                             XYpoint_Arr[2][0]));//x1 - x3
          Dy = abs((int16_t)(XYpoint_Arr[1][1] -
                             XYpoint_Arr[2][1]));//y1 - y3
          Dx *= Dx;
          Dy *= Dy;
          Sqrt1 = sqrt(Dx + Dy);//;

          Dx = abs((int16_t)(XYpoint_Arr[0][0] -
                             XYpoint_Arr[3][0]));//x2 - x4
          Dy = abs((int16_t)(XYpoint_Arr[0][1] -
                             XYpoint_Arr[3][1]));//y2 - y4
          Dx *= Dx;
          Dy *= Dy;
          Sqrt2 = sqrt(Dx + Dy);//

          Dsqrt = (float)Sqrt1 / Sqrt2;
          if (Dsqrt < 0.95 || Dsqrt > 1.05) {
            DEBUG("Adjust diagonal direction\r\n");
            cnt = 0;
            TP_ShowInfo(XYpoint_Arr[0][0], XYpoint_Arr[0][1],
                        XYpoint_Arr[1][0], XYpoint_Arr[1][1],
                        XYpoint_Arr[2][0], XYpoint_Arr[2][1],
                        XYpoint_Arr[3][0], XYpoint_Arr[3][1],
                        Dsqrt * 100);
            Driver_Delay_ms(1000);
            TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                         sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
            TP_DrawCross(Mar_Val, Mar_Val, RED);
            continue;
          }

          //4.Get the scale factor and offset
          //Get the scanning direction of the touch screen
          sTP_DEV.TP_Scan_Dir = sLCD_DIS.LCD_Scan_Dir;
          sTP_DEV.fXfac = 0;

          //According to the display direction to get
          //the corresponding scale factor and offset
          if (sTP_DEV.TP_Scan_Dir == R2L_D2U) {
            DEBUG("R2L_D2U\r\n");

            sTP_DEV.fXfac = (float)(sLCD_DIS.LCD_Dis_Column - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[1][0] -
                                      XYpoint_Arr[0][0]);
            sTP_DEV.fYfac = (float)(sLCD_DIS.LCD_Dis_Page - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[2][1] -
                                      XYpoint_Arr[0][1]);

            sTP_DEV.iXoff = (sLCD_DIS.LCD_Dis_Column -
                             sTP_DEV.fXfac * (XYpoint_Arr[1][0] +
                                              XYpoint_Arr[0][0])
                            ) / 2;
            sTP_DEV.iYoff = (sLCD_DIS.LCD_Dis_Page -
                             sTP_DEV.fYfac * (XYpoint_Arr[2][1] +
                                              XYpoint_Arr[0][1])
                            ) / 2;

          } else if (sTP_DEV.TP_Scan_Dir == L2R_U2D) {
            DEBUG("L2R_U2D\r\n");

            sTP_DEV.fXfac = (float)(sLCD_DIS.LCD_Dis_Column - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[0][0] -
                                      XYpoint_Arr[1][0]);
            sTP_DEV.fYfac = (float)(sLCD_DIS.LCD_Dis_Page - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[0][1] -
                                      XYpoint_Arr[2][1]);

            sTP_DEV.iXoff = (sLCD_DIS.LCD_Dis_Column -
                             sTP_DEV.fXfac * (XYpoint_Arr[0][0] +
                                              XYpoint_Arr[1][0])
                            ) / 2;
            sTP_DEV.iYoff = (sLCD_DIS.LCD_Dis_Page - sTP_DEV.fYfac *
                             (XYpoint_Arr[0][1] + XYpoint_Arr[2][1])) / 2;
          } else if (sTP_DEV.TP_Scan_Dir == U2D_R2L) {
            DEBUG("U2D_R2L\r\n");

            sTP_DEV.fXfac = (float)(sLCD_DIS.LCD_Dis_Column - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[1][1] - XYpoint_Arr[0][1]);
            sTP_DEV.fYfac = (float)(sLCD_DIS.LCD_Dis_Page - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[2][0] - XYpoint_Arr[0][0]);

            sTP_DEV.iXoff = (sLCD_DIS.LCD_Dis_Column -
                             sTP_DEV.fXfac * (XYpoint_Arr[1][1] +
                                              XYpoint_Arr[0][1])
                            ) / 2;
            sTP_DEV.iYoff = (sLCD_DIS.LCD_Dis_Page -
                             sTP_DEV.fYfac * (XYpoint_Arr[2][0] +
                                              XYpoint_Arr[0][0])
                            ) / 2;
          } else {
            DEBUG("D2U_L2R\r\n");

            sTP_DEV.fXfac = (float)(sLCD_DIS.LCD_Dis_Column - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[0][1] -
                                      XYpoint_Arr[1][1]);
            sTP_DEV.fYfac = (float)(sLCD_DIS.LCD_Dis_Page - 2 * Mar_Val) /
                            (int16_t)(XYpoint_Arr[0][0] -
                                      XYpoint_Arr[2][0]);

            sTP_DEV.iXoff = (sLCD_DIS.LCD_Dis_Column -
                             sTP_DEV.fXfac * (XYpoint_Arr[0][1] +
                                              XYpoint_Arr[1][1])
                            ) / 2;
            sTP_DEV.iYoff = (sLCD_DIS.LCD_Dis_Page -
                             sTP_DEV.fYfac * (XYpoint_Arr[0][0] +
                                              XYpoint_Arr[2][0])
                            ) / 2;
          }

          DEBUG("sTP_DEV.fXfac = %f \r\n");
          DEBUG(sTP_DEV.fXfac);
          DEBUG("sTP_DEV.fYfac = %f \r\n");
          DEBUG(sTP_DEV.fYfac);
          DEBUG("sTP_DEV.iXoff = %d \r\n");
          DEBUG(sTP_DEV.iXoff);
          DEBUG("sTP_DEV.iYoff = %d \r\n");
          DEBUG( sTP_DEV.iYoff);

          //6.Calibration is successful
          DEBUG("Adjust OK\r\n");
          LCD_Clear(LCD_BACKGROUND);
          GUI_DisString_EN(35, 110, "Touch Screen Adjust OK!",
                           &Font16 , FONT_BACKGROUND , RED);
          Driver_Delay_ms(1000);
          LCD_Clear(LCD_BACKGROUND);
          return;
        //Exception handling,Reset  Initial value
        default :
          cnt = 0;
          TP_DrawCross(sLCD_DIS.LCD_Dis_Column - Mar_Val,
                       sLCD_DIS.LCD_Dis_Page - Mar_Val, WHITE);
          TP_DrawCross(Mar_Val, Mar_Val, RED);
          GUI_DisString_EN(40, 26, "TP Need readjust!",
                           &Font16 , FONT_BACKGROUND , RED);
          break;
      }
    }
  }
}



/*******************************************************************************
  function:
        Use the default calibration factor
*******************************************************************************/
void TP_GetAdFac(void)
{
  if ( sTP_DEV.TP_Scan_Dir == D2U_L2R ) { //SCAN_DIR_DFT = D2U_L2R
    sTP_DEV.fXfac = -0.132443F ;
    sTP_DEV.fYfac = 0.089997F ;
    sTP_DEV.iXoff = 516L ;
    sTP_DEV.iYoff = -22L ;
  } else if ( sTP_DEV.TP_Scan_Dir == L2R_U2D ) {
    sTP_DEV.fXfac = 0.089697F ;
    sTP_DEV.fYfac = 0.134792F ;
    sTP_DEV.iXoff = -21L ;
    sTP_DEV.iYoff = -39L ;
  } else if ( sTP_DEV.TP_Scan_Dir == R2L_D2U ) {
    sTP_DEV.fXfac = 0.089915F ;
    sTP_DEV.fYfac =  0.133178F ;
    sTP_DEV.iXoff = -22L ;
    sTP_DEV.iYoff = -38L ;
  } else if ( sTP_DEV.TP_Scan_Dir == U2D_R2L ) {
    sTP_DEV.fXfac = -0.132906F ;
    sTP_DEV.fYfac = 0.087964F ;
    sTP_DEV.iXoff = 517L ;
    sTP_DEV.iYoff = -20L ;
  } else {
    LCD_Clear(LCD_BACKGROUND);
    GUI_DisString_EN(0, 60, "Does not support touch-screen \
                        calibration in this direction",
                     &Font16, FONT_BACKGROUND, RED);
  }
}

/*******************************************************************************
  function:
        Paint the Delete key and paint color choose area
*******************************************************************************/
void TP_Dialog(void)
{
  LCD_Clear(LCD_BACKGROUND);
  DEBUG("Drawing...\r\n");
  //Horizontal screen display
  if (sLCD_DIS.LCD_Dis_Column > sLCD_DIS.LCD_Dis_Page) {
    //Clear screen
    GUI_DisString_EN(sLCD_DIS.LCD_Dis_Column - 60, 0,
                     "CLEAR", &Font16, RED, BLUE);
    //adjustment
    GUI_DisString_EN(sLCD_DIS.LCD_Dis_Column - 120, 0,
                     "AD", &Font24, RED, BLUE);
    //choose the color
    GUI_DrawRectangle(sLCD_DIS.LCD_Dis_Column - 50, 20,
                      sLCD_DIS.LCD_Dis_Column, 70,
                      BLUE, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DrawRectangle(sLCD_DIS.LCD_Dis_Column - 50, 80,
                      sLCD_DIS.LCD_Dis_Column, 130,
                      GREEN, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DrawRectangle(sLCD_DIS.LCD_Dis_Column - 50, 140,
                      sLCD_DIS.LCD_Dis_Column, 190,
                      RED, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DrawRectangle(sLCD_DIS.LCD_Dis_Column - 50, 200,
                      sLCD_DIS.LCD_Dis_Column, 250,
                      YELLOW, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DrawRectangle(sLCD_DIS.LCD_Dis_Column - 50, 260,
                      sLCD_DIS.LCD_Dis_Column, 310,
                      BLACK, DRAW_FULL, DOT_PIXEL_1X1);

  } else { //Vertical screen display
    GUI_DisString_EN(sLCD_DIS.LCD_Dis_Column - 60, 0,
                     "CLEAR", &Font16, RED, BLUE);
    GUI_DisString_EN(sLCD_DIS.LCD_Dis_Column - 120, 0,
                     "AD", &Font24, RED, BLUE);
    GUI_DrawRectangle(20, 20, 70, 70, BLUE, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DrawRectangle(80, 20, 130, 70, GREEN, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DrawRectangle(140, 20, 190, 70, RED, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DrawRectangle(200, 20, 250, 70, YELLOW, DRAW_FULL, DOT_PIXEL_1X1);
    GUI_DrawRectangle(260, 20, 310, 70, BLACK, DRAW_FULL, DOT_PIXEL_1X1);
  }
}

/*******************************************************************************
  function:
        Draw Board
*******************************************************************************/
void TP_DrawBoard(void)
{
  //  sTP_DEV.chStatus &= ~(1 << 6);
  TP_Scan(0);
  if (sTP_DEV.chStatus & TP_PRESS_DOWN) {     //Press the button
    //Horizontal screen
    if (sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
        //Determine whether the law is legal
        sTP_Draw.Ypoint < sLCD_DIS.LCD_Dis_Page) {
      //Judgment is horizontal screen
      if (sLCD_DIS.LCD_Dis_Column > sLCD_DIS.LCD_Dis_Page) {
        if (sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 60) &&
            sTP_Draw.Ypoint < 16) {     //Clear Board
          TP_Dialog();
        } else if (sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 120) &&
                   sTP_Draw.Xpoint < (sLCD_DIS.LCD_Dis_Column - 80) &&
                   sTP_Draw.Ypoint < 24) { //afresh adjustment
          TP_Adjust();
          TP_Dialog();
        } else if (sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                   sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                   sTP_Draw.Ypoint > 20 &&
                   sTP_Draw.Ypoint < 70) {
          sTP_Draw.Color = BLUE;
        } else if (sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                   sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                   sTP_Draw.Ypoint > 80 &&
                   sTP_Draw.Ypoint < 130) {
          sTP_Draw.Color = GREEN;
        } else if (sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                   sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                   sTP_Draw.Ypoint > 140 &&
                   sTP_Draw.Ypoint < 190) {
          sTP_Draw.Color = RED;
        } else if (sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                   sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                   sTP_Draw.Ypoint > 200 && sTP_Draw.Ypoint < 250) {
          sTP_Draw.Color = YELLOW;
        } else if (sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 50) &&
                   sTP_Draw.Xpoint < sLCD_DIS.LCD_Dis_Column &&
                   sTP_Draw.Ypoint > 260 &&
                   sTP_Draw.Ypoint < 310) {
          sTP_Draw.Color = BLACK;
        } else {
          GUI_DrawPoint(sTP_Draw.Xpoint, sTP_Draw.Ypoint,
                        sTP_Draw.Color , DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
          GUI_DrawPoint(sTP_Draw.Xpoint + 1, sTP_Draw.Ypoint,
                        sTP_Draw.Color , DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
          GUI_DrawPoint(sTP_Draw.Xpoint, sTP_Draw.Ypoint + 1,
                        sTP_Draw.Color , DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
          GUI_DrawPoint(sTP_Draw.Xpoint + 1, sTP_Draw.Ypoint + 1,
                        sTP_Draw.Color , DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
          GUI_DrawPoint(sTP_Draw.Xpoint, sTP_Draw.Ypoint,
                        sTP_Draw.Color , DOT_PIXEL_2X2, DOT_FILL_RIGHTUP);
        }
        //Vertical screen
      } else {
        if (sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 60) &&
            sTP_Draw.Ypoint < 16) {//Clear Board
          TP_Dialog();
        } else if (sTP_Draw.Xpoint > (sLCD_DIS.LCD_Dis_Column - 120) &&
                   sTP_Draw.Xpoint < (sLCD_DIS.LCD_Dis_Column - 80) &&
                   sTP_Draw.Ypoint < 24) { //afresh adjustment
          TP_Adjust();
          TP_Dialog();
        } else if (sTP_Draw.Xpoint > 20 && sTP_Draw.Xpoint < 70 &&
                   sTP_Draw.Ypoint > 20 && sTP_Draw.Ypoint < 70) {
          sTP_Draw.Color = BLUE;
        } else if (sTP_Draw.Xpoint > 80 && sTP_Draw.Xpoint < 130 &&
                   sTP_Draw.Ypoint > 20 && sTP_Draw.Ypoint < 70) {
          sTP_Draw.Color = GREEN;
        } else if (sTP_Draw.Xpoint > 140 && sTP_Draw.Xpoint < 190 &&
                   sTP_Draw.Ypoint > 20 && sTP_Draw.Ypoint < 70) {
          sTP_Draw.Color = RED;
        } else if (sTP_Draw.Xpoint > 200 && sTP_Draw.Xpoint < 250 &&
                   sTP_Draw.Ypoint > 20 && sTP_Draw.Ypoint < 70) {
          sTP_Draw.Color = YELLOW;
        } else if (sTP_Draw.Xpoint > 260 && sTP_Draw.Xpoint < 310 &&
                   sTP_Draw.Ypoint > 20 && sTP_Draw.Ypoint < 70) {
          sTP_Draw.Color = BLACK;
        } else {
          GUI_DrawPoint(sTP_Draw.Xpoint, sTP_Draw.Ypoint,
                        sTP_Draw.Color , DOT_PIXEL_2X2,
                        DOT_FILL_RIGHTUP );
        }
      }
    }
  }
}

/*******************************************************************************
  function:
        Touch pad initialization
*******************************************************************************/
void TP_Init( LCD_SCAN_DIR Lcd_ScanDir )
{
  TP_CS_1;

  sTP_DEV.TP_Scan_Dir = Lcd_ScanDir;

  TP_Read_ADC_XY(&sTP_DEV.Xpoint, &sTP_DEV.Ypoint);
}



