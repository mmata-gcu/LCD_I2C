// MODIFICATION OF lidarlite.h
// lidarlite.h
// Interface for Lidar-Lite V2 (Blue Label) with NVIDIA Jetson TK1
// The MIT License (MIT)
// 
// Copyright (c) 2015 Jetsonhacks
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/*
  rgb_lcd.h
  2013 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  2013-9-18

  add rgb backlight fucnction @ 2013-10-15
  
  The MIT License (MIT)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.1  USA
*/

#ifndef _LCD_I2C_H
#define _LCD_I2C_H

#include <cstddef>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// Information taken from Grove_LCD_RGB_Backlight-master
// Device I2C Arress
#define LCD_ADDRESS     (0x7c>>1)//(0x3E)//(0x70) //why the >>1? 
#define RGB_ADDRESS     (0xc4>>1)//(0x62) //CORRECT ADDRESS but why the >>1? Because of bus 1?

// color define 
#define LCD_WHITE           0
#define LCD_RED             1
#define LCD_GREEN           2
#define LCD_BLUE            3

#define REG_RED         0x04        // pwm2
#define REG_GREEN       0x03        // pwm1
#define REG_BLUE        0x02        // pwm0

#define REG_MODE1       0x00
#define REG_MODE2       0x01
#define REG_OUTPUT      0x08

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LCD_I2C //: public Print
{

public:
  LCD_I2C();
  LCD_I2C(unsigned char cols, unsigned char rows, unsigned char charsize = LCD_5x8DOTS);
  ~LCD_I2C();

  // print string
  void prints(const char *str);

  void clear();
  void home();
  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void setCursor(unsigned char col, unsigned char row); 
  
  // color control
  void setRGB(unsigned char r, unsigned char g, unsigned char b);               
  void setPWM(unsigned char color, unsigned char pwm){setReg(color, pwm);}      
  
  void setColor(unsigned char color);
  void setColorAll(){setRGB(0, 0, 0);}
  void setColorWhite(){setRGB(255, 255, 255);}

  // blink the LED backlight
  void blinkLED(void);
  void noBlinkLED(void);              

  int get_error(){return error;} //0=no error, error=errno (<0)
  
private:
  unsigned char kI2CBus;         // I2C bus of the LCD
  int kI2CFileDescriptor;        // File Descriptor to the LCD
  int kI2C_RGBFileDescriptor;    // File Descriptor to the RGB display
  int error;			 // Last error code

  inline void command(unsigned char value);
  void write(unsigned char);
  void setReg(unsigned char addr, unsigned char dta);

  unsigned char _displayfunction;
  unsigned char _displaycontrol;
  unsigned char _displaymode;

  unsigned char _initialized;
  unsigned char _numlines,_currline;
  unsigned char _numcols;
};

#endif
