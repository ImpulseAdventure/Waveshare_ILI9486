/*****************************************************************************
* | File      	:	Debug.h
* | Author      :   Waveshare team
* | Function    :	debug with prntf
* | Info        :
*   Image scanning
*      Please use progressive scanning to generate images or fonts
*----------------
* |	This version:   V1.0
* | Date        :   2018-01-11
* | Info        :   Basic version
*
******************************************************************************/
#ifndef __DEBUG_H
#define __DEBUG_H
#include <Wire.h>

#define DEV_DEBUG 1
#if DEV_DEBUG
	#define DEBUG(__info) Serial.print(__info)
#else
	#define DEBUG(__info)  
#endif

#endif

