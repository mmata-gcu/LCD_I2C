/*
  rgb_lcd.cpp
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

#include <stdio.h>
#include <string.h>
#include "lcd_i2c.h"

#define LCD_TX1_I2C_BUS 1 //use TX1's I2C bus 1

LCD_I2C::LCD_I2C()
{
 LCD_I2C(16, 2, LCD_5x8DOTS);
}

LCD_I2C::LCD_I2C(unsigned char cols, unsigned char lines, unsigned char dotsize)
{
    kI2CBus = LCD_TX1_I2C_BUS;  // Default I2C bus for Lidar-Lite on Jetson TX1
    error = -1;    //not initialized

    char fileNameBuffer[32];
    sprintf(fileNameBuffer,"/dev/i2c-%d", kI2CBus);

    //file descriptor to the RGB backlight address
    kI2C_RGBFileDescriptor = open(fileNameBuffer, O_RDWR);
    if (kI2C_RGBFileDescriptor < 0) {
        // Could not open the file
        error = -2 ;
        return;
    }
    if (ioctl(kI2C_RGBFileDescriptor, I2C_SLAVE, RGB_ADDRESS) < 0) {
        // Could not open the device on the bus
        error = -3 ;
        return;
    }

    //file descriptor to the LCD address
    kI2CFileDescriptor = open(fileNameBuffer, O_RDWR);
    if (kI2CFileDescriptor < 0) {
        // Could not open the file
        error = -4 ;
        return;
    }
    if (ioctl(kI2CFileDescriptor, I2C_SLAVE, LCD_ADDRESS) < 0) {
        // Could not open the device on the bus
        error = -5 ;
        return;
    }

    //initialize things
    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numcols = cols;
    _numlines = lines;
    _currline = 0;

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    usleep(50000);
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23
    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    usleep(4500);  // wait more than 4.1ms
    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    usleep(150);
    // third go
    command(LCD_FUNCTIONSET | _displayfunction);
      usleep(150);
    // finally, set # lines, font size, etc.
    command(LCD_FUNCTIONSET | _displayfunction);
      usleep(150);
    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();
      usleep(150);
    // clear it off
    clear();
      usleep(150);
    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    command(LCD_ENTRYMODESET | _displaymode);
    
    
    // backlight init
    setReg(REG_MODE1, 0);
    // set LEDs controllable by both PWM and GRPPWM registers
    setReg(REG_OUTPUT, 0xFF);
    // set MODE2 values
    // 0010 0000 -> 0x20  (DMBLNK to 1, ie blinky mode)
    setReg(REG_MODE2, 0x20);

    setColorWhite();
    error=0; //no error so far
}

LCD_I2C::~LCD_I2C()
{
    if (kI2CFileDescriptor > 0) {
        close(kI2CFileDescriptor);
        // WARNING - This is not quite right, need to check for error first
        kI2CFileDescriptor = -1 ;
    }
}


/********** high level commands, for the user! */
void LCD_I2C::clear()
{
    command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
    usleep(2000);          // this command takes a long time!
}

void LCD_I2C::home()
{
    command(LCD_RETURNHOME);        // set cursor position to zero
    usleep(2000);        // this command takes a long time!
}

void LCD_I2C::setCursor(unsigned char col, unsigned char row)
{
    col = (row == 0 ? col|0x80 : col|0xc0);
    command(col);
}

// Turn the display on/off (quickly)
void LCD_I2C::noDisplay()
{
    _displaycontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCD_I2C::display() {
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LCD_I2C::noCursor()
{
    _displaycontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCD_I2C::cursor() {
    _displaycontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LCD_I2C::noBlink()
{
    _displaycontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_I2C::blink()
{
    _displaycontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCD_I2C::scrollDisplayLeft(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCD_I2C::scrollDisplayRight(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCD_I2C::leftToRight(void)
{
    _displaymode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LCD_I2C::rightToLeft(void)
{
    _displaymode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LCD_I2C::autoscroll(void)
{
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LCD_I2C::noAutoscroll(void)
{
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// Control the backlight LED blinking
void LCD_I2C::blinkLED(void)
{
    // blink period in seconds = (<reg 7> + 1) / 24
    // on/off ratio = <reg 6> / 256
    setReg(0x07, 0x17);  // blink every second
    setReg(0x06, 0x7f);  // half on, half off
}

void LCD_I2C::noBlinkLED(void)
{
    setReg(0x07, 0x00);
    setReg(0x06, 0xff);
}

/*********** mid level commands, for sending data/cmds */

// send command
inline void LCD_I2C::command(unsigned char value)
{
    int toReturn = i2c_smbus_write_byte_data(kI2CFileDescriptor, 0x80, value);
    // Wait a little bit to make sure it settles
    usleep(10000);
    if (toReturn < 0) {
        error = errno ;
    }
}

// send data
inline void LCD_I2C::write(unsigned char value)
{
    int toReturn = i2c_smbus_write_byte_data(kI2CFileDescriptor, 0x40, value);
    // Wait a little bit to make sure it settles
    usleep(10000);
    if (toReturn < 0) {
        error = errno ;
    }
}

void LCD_I2C::setRGB(unsigned char r, unsigned char g, unsigned char b)
{
    setReg(REG_RED, r);
    setReg(REG_GREEN, g);
    setReg(REG_BLUE, b);
}

const unsigned char color_define[4][3] = 
{
    {255, 255, 255},            // white
    {255, 0, 0},                // red
    {0, 255, 0},                // green
    {0, 0, 255},                // blue
};

void LCD_I2C::setColor(unsigned char color)
{
    if(color > 3)return ;
    setRGB(color_define[color][0], color_define[color][1], color_define[color][2]);
}

// print string
void LCD_I2C::prints(const char *str)
{
 for(int i=0;i<_numcols;i++)
 {
   if((str[i]=='\0')||(str[i]=='\n')) //end of string
       return;
   else
       write(str[i]);
 } 
}


void LCD_I2C::setReg(unsigned char addr, unsigned char dta)
{
    int toReturn = i2c_smbus_write_byte_data(kI2C_RGBFileDescriptor, addr, dta);
    // Wait a little bit to make sure it settles
    usleep(10000);
    if (toReturn < 0) {
        error = errno ;
    }
}

