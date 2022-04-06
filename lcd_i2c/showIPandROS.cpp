/*showIPandROS.cpp
  2022 Copyright (c) Mario Mata.  All right reserved.

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
#include <stdlib.h>
#include <string.h>
#include "lcd_i2c.h"
#include <ctype.h>  //for isdigit

//ROS master process name, used to find out its status
#define ROS_PROCESS_NAME "rosmaster"

LCD_I2C *lcd;
int ConnectLCD();
void LCDprintIPfailed();
void LCDprintNoIP();
void LCDprintROSfailed();
void LCDprintNoROS();
void LCDprintROSok();

char IP_addr[17];

/* This program checks if the robot has IP and if the ROS process is running, and updates the LCD display accordingly 
*/
int main()
{
 bool update = true; //to avoid using the LCD if ROS already running
 int error = 0;
 FILE *fp;
 char aux[100];
 const char *p_aux;
 
 while(1)
 {
    //Get IP address from ifconfig wlan0
    fp=popen("ifconfig wlan0","r");
    if(fp!=NULL) //check if we have IP
    { 
      fgets(aux,100,fp); //get first line -discarded
      fgets(aux,100,fp); //get second line -contains inet or inet6-
      //find 'i' from "inet" in the extracted string
      p_aux = strchr(aux,'i');
      sscanf(p_aux+10,"%17s",IP_addr);
      fclose(fp);
      if(isdigit(IP_addr[0])){ //got the IP! Now check ROS
	   //check if ROS process is running
	   sprintf(aux,"pgrep %s",ROS_PROCESS_NAME);
	   fp=popen(aux,"r");
	   if(fp!=NULL){ //check if ROS process is there 
     	 fgets(aux,30,fp);
         fclose(fp);
         if(isdigit(aux[0])){ //got ROS PID number 
		   if(update){
	         update = false;
	         LCDprintROSok();
		   }
         }
         else { //ROS not ready
	      update = true;
          LCDprintNoROS();
         }
       }
       else { //popen pgrep failed! this should not happen
         update = true;
         LCDprintROSfailed();
       }  	
      }
      else { //No IP assigned
        update = true;
        LCDprintNoIP();
      }
    }
    else { //popen failed
      update = true;
      LCDprintIPfailed(); 
    }

   if(update) sleep(5); //check it frequently
   else sleep(15); //ROS is up, check every now and then
 }//infinite loop
}

int ConnectLCD()
{
 lcd= new LCD_I2C(16,2); //cols, rows
 if (lcd->get_error() <0){
  return(lcd->get_error());
 }
}

void LCDprintIPfailed()
{
 int error = ConnectLCD();
 if(error<0)
   exit(error);
 lcd->setCursor(0,1);
 lcd->setColor(LCD_RED);  
 lcd->prints("IP check failed"); //output ROS status to the LCD
 delete(lcd);
}

void LCDprintNoIP()
{
 int error = ConnectLCD();
 if(error<0)
   exit(error); 
 lcd->setCursor(0,1);
 lcd->setColor(LCD_RED);
 lcd->prints("No IP assigned!");
 delete(lcd);
}

void LCDprintROSfailed()
{
 int error = ConnectLCD();
 if(error<0)
   exit(error);
 lcd->setColor(LCD_BLUE);
 lcd->setCursor(0,0);    
 lcd->prints(IP_addr); //output IP status to the LCD
 lcd->setCursor(0,1);
 lcd->prints("ROS check failed"); //output ROS status to the LCD
 delete(lcd);
}

void LCDprintNoROS()
{
 int error = ConnectLCD();
 if(error<0)
   exit(error);
 lcd->setColor(LCD_BLUE);
 lcd->setCursor(0,0);    
 lcd->prints(IP_addr); //output IP status to the LCD
 lcd->setCursor(0,1);  
 lcd->prints("ROS not ready!"); //output ROS status to the LCD 
 delete(lcd);
}

void LCDprintROSok()
{
 int error = ConnectLCD();
 if(error<0)
   exit(error);
 lcd->setColor(LCD_GREEN);
 lcd->setCursor(0,0);    
 lcd->prints(IP_addr); //output IP status to the LCD
 lcd->setCursor(0,1);  
 lcd->prints("ROS ready!"); //output ROS status to the LCD
 delete(lcd);
}

