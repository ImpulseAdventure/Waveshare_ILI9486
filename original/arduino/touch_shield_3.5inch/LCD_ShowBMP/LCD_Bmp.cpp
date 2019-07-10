/*****************************************************************************
* | File          :   LCD_Bmp.c
* | Author      :   Waveshare team
* | Function    :   Show SDcard BMP picto LCD 
* | Info        :
*   Image scanning
*      Please use progressive scanning to generate images or fonts
*----------------
* | This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#include <SD.h>
#include <SPI.h>

#include "LCD_Bmp.h"
#include "Debug.h"

BMP_DIS BMP_Dis;
extern LCD_DIS sLCD_DIS;

char BMP_File[BMP_NUM][FILENAME_LEN] =           // add file name here
{
  "rain.bmp",
  "board.bmp",
  "sunflower.bmp",
  "universe.bmp",
};
File bmpFile;

/*********************************************/
// These read data from the SD card file and convert them to big endian
// (the data is stored in little endian format!)
static uint16_t Read16(File f)
{
  uint16_t d;
  uint8_t b;
  b = f.read();
  d = f.read();
  d <<= 8;
  d |= b;
  return d;
}

// LITTLE ENDIAN!
static uint32_t Read32(File f)
{
  uint32_t d;
  uint16_t b;

  b = Read16(f);
  d = Read16(f);
  d <<= 16;
  d |= b;
  return d;
}

static boolean ReadBmpHeader(File f)
{
  uint16_t BMP_File;
  BMP_File = Read16(f) ;//0000h 2byte: file type
  
  if (BMP_File != 0x4D42) {
    // magic bytes missing
    return false;
  }

  // read file size
  BMP_Dis.Size = Read32(f);//0002h 4byte: file size

  // read and ignore creator bytes
  Read32(f);//0006h 4byte:

  BMP_Dis.Index = Read32(f);//000ah 4byte: Offset between file head and image (offbits)

  // read DIB header
  BMP_Dis.Header_Size = Read32(f);//000Eh 4byte: Bitmap header = 40

  BMP_Dis.BMP_Width = Read32(f);//12-15:Image wide
  BMP_Dis.BMP_Height = Read32(f);//16-19:Image high

  if (BMP_Dis.BMP_Width != sLCD_DIS.LCD_Dis_Column || BMP_Dis.BMP_Height != sLCD_DIS.LCD_Dis_Page)  {   // if image is not 480*320, return false
    DEBUG("BMP is not 480*320");
    return false;
  }

  if (Read16(f) != 1)//1A-1B :0 for the use of the palette, 1 for the number of color indices
    return false;

  BMP_Dis.Bit_Pixel = Read16(f);//1C-1D：Whether the image is compressed

  if (Read32(f) != 0) {//1E-21：
    // compression not supported!
    return false;
  }

  return true;
}

#define BUFFPIXEL_X3(__val)    ( (__val) * 3)                 // BUFFPIXELx3
#define RGB24TORGB16(R,G,B) (( (R) >> 3 ) << 11 ) | (( (G) >> 2 ) << 5) | ( (B) >> 3)
static void LCD_DrawBmp( int x, int y)
{
  bmpFile.seek( BMP_Dis.Index);
  uint32_t time = millis();
  uint16_t BuffPixel =  BMP_Dis.BMP_Width / 4;
  uint16_t X_Point = (uint16_t)x;
  uint16_t Y_Point = (uint16_t)y;

  uint8_t ReadBuff[BUFFPIXEL_X3(BuffPixel)];
  //clear buffer
  for (uint16_t N = 0; N < BUFFPIXEL_X3(BuffPixel); N++) {
    ReadBuff[N] = 0;
  }

  for ( Y_Point = 0; Y_Point < BMP_Dis.BMP_Height; Y_Point++) {//Total display column
    for ( X_Point = 0; X_Point < BMP_Dis.BMP_Width / BuffPixel; X_Point++) {//Show a line in the line
      bmpFile.read(ReadBuff, BUFFPIXEL_X3(BuffPixel));

      //show bmp to LCD
      int k = 0;
      LCD_SetWindow( X_Point * BuffPixel , Y_Point, ( X_Point + 1 ) * BuffPixel , Y_Point);
      LCD_DC_1;
      LCD_CS_0;
      for (uint16_t j = 0; j < BuffPixel; j++) {
        k = j * 3;
        SPI4W_Write_Byte( (RGB24TORGB16( ReadBuff[k + 2], ReadBuff[k + 1], ReadBuff[k])) >> 8);
        SPI4W_Write_Byte( (RGB24TORGB16(ReadBuff[k + 2], ReadBuff[k + 1], ReadBuff[k]))  & 0XFF);
      }
      LCD_CS_1;
    }
  }
  Serial.print("show Bmp use ");
  Serial.print(millis() - time, DEC);
  Serial.println(" ms");
}
/**************************************************************************
    This procedure reads a bitmap and draws it to the screen
    its sped up by reading many pixels worth of data at a
    time instead of just one pixel at a time. increading the buffer takes
    more RAM but makes the drawing a little faster. 20 pixels' worth
    is probably a good place
**************************************************************************/
void SD_Init(void) 
{
  SD_CS_1;
  LCD_CS_1;
  TP_CS_1;

  Sd2Card card;
  card.init(SPI_FULL_SPEED, SD_CS);
  if (!SD.begin( SD_CS ))  {
    DEBUG("SD init failed!");
    while (1);                              // init fail, die here
  }
  DEBUG("SD init OK!");
}

void LCD_ShowBMP(void) 
{
  unsigned char i;
  for (i = 0; i < BMP_NUM; i++) {
    bmpFile = SD.open(BMP_File[i]);
    if (! bmpFile) {
      DEBUG(BMP_File[i]);
      DEBUG(" image not found");
      continue ;
    } else {
      DEBUG(BMP_File[i]);
      DEBUG(" image open");
    }

    if (! ReadBmpHeader(bmpFile)) {
      DEBUG("find next image");
      continue;
    }

    LCD_SetGramScanWay(D2U_R2L);
    LCD_DrawBmp(0, 0);
    bmpFile.close();
    LCD_SetGramScanWay(SCAN_DIR_DFT);
    
    delay(1000);
    delay(1000);
  }
}

