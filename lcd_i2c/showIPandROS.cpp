#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <termios.h>
//#include <time.h>
#include "lcd_i2c.h"
#include <ctype.h>  //for isdigit
//#include <unistd.h> //for sleep, already in lcd_i2c.h

#define ROS_PROCESS_NAME "rosmaster"

LCD_I2C *lcd;
int ConnectLCD();
void LCDprintIPfailed();
void LCDprintNoIP();
void LCDprintROSfailed();
void LCDprintNoROS();
void LCDprintROSok();

char IP_addr[17];

/*
This program checks if the Jet robot has IP and if the ROS process is running, and updates the LCD display accordingly
Can be run from crontab every minute (remember to run it as root or sudo to be able to access the I2C bus).
To do that: sudo gedit /etc/crontab
Add the following line before the last '#':
* *	* * *	root	/path_for_program/name_of_program 
*/
int main()
{
 bool update = true; //to avoid using the LCD if ROS running
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
         //printf("Extracted: %s\n",aux);
      //find 'i' from "inet" in the extracted string
      p_aux = strchr(aux,'i');
      sscanf(p_aux+10,"%17s",IP_addr);
         //printf("Extracted:%s\n",IP_addr);
         //printf("First char is:%c\n",IP_addr[0]);
      fclose(fp);
      if(isdigit(IP_addr[0])) //got the IP! Now check ROS
      {
	 //check if ROS process is running
	 sprintf(aux,"pgrep %s",ROS_PROCESS_NAME);
	 fp=popen(aux,"r");
	 if(fp!=NULL) //check if ROS process is there
	 { 
     	    fgets(aux,30,fp);
            fclose(fp);
            if(isdigit(aux[0])) //got ROS PID number 
            {
		   if(update){
	         update = false;
	         LCDprintROSok();
		   }
            }
            else //ROS not ready
            {
	     update = true;
             LCDprintNoROS();
            }
         }
         else //popen pgrep failed! this should not happen
         {
	   update = true;
           LCDprintROSfailed();
         }  	
      }
      else //No IP assigned
      {
        update = true;
        LCDprintNoIP();
      }
    }
    else //popen failed! this should not happen
    {
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
 if (lcd->get_error() <0) 
 {
  //printf("Error initializing LCD: %d\n", lcd->get_error());
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
   //printf("Failed to check IP!\n");
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
    //printf("IP: NONE!\n");
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
   //printf("Failed to check ROS!\n");
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
      //printf("ROS not ready!\n");
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
      //printf("%s exists!\n",ROS_PROCESS_NAME);
 delete(lcd);
}

