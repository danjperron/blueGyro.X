/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  catapult
//
// Simple program to access MPU6050 accelerometer and report
// data via bluetooth using a HC04 serial to bluetooth adapter

//   Date: 26 Dec 2015
//   programmer: Daniel Perron
//   Version: 1.0
//   Processor: PIC12F1840
//   Software: MPLAB X IDE v3.15,  XC8 (v1.35) 

                   

///////////////////  How to program the IC.
//
//    1 - Use MPLAB with pickit 3  
//  or
//    2 - Use standalone pickit2 v 2.61 (Select the I.C. and load the corresponding hex file).
//  or
//    3 - Use Raspberry Pi board  with burnVLPx.py . Software available from https://github.com/danjperron/A2D_PIC_RPI 
//        

////////////////////////////////////  GPL LICENSE ///////////////////////////////////
/*
 This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



/*  COMMUNICATION PROTOCOL
    
      using TTL UART with fix baud at 115200, 8 bits data, no parity.
 
      ASCII COMMAND character
       [esc] 			-> IDLE  Go to idle mode (do nothing)
        G,g or enter	-> READY  Be ready for High G detection
        I or i          -> INFO   Display Acceleration info in G.
        H or h          -> HIT    Force High/Low G detection
        Z or z          -> Zero   Zero Gyro values (set offset)
       
        D or d          -> Dump   Dump flash data from AT24CM02
 

      Command available in IDLE mode only
        ?               -> display Release version.
        V or v          -> Display Battery Voltage.

        
       Record Format:   Everything in HEX Format (for speed, The PIC is too slow to decode decimal ascii)
               TTTTXXXXYYYYZZZZxxxxyyyyzzzz\r\n
        where TTTT is time in ms
              XXXX is acceleration on X axis (signed short integer)
              YYYY is acceleration on Y axis (signed short integer)
              ZZZZ is acceleration on Z axis (signed short integer)
              xxxx is gyro x
              yyyy is gyro y
              zzzz is gyro z


*/    
//////////////////////////  PORT DESCRIPTION
/*

 RA0    IN      not used 
 RA1	I/O    	I2C SCL
 RA2	I/O		I2C SDA
 RA3	IN		MCLR  Master Clear  (Reset cpu when is low)
 RA4	OUT	UART TX
 RA5	IN		UART RX
*/


// Dec 29 Magnetometer data add-on
// if you don't want magnetometer  undef HMC5883L_ENABLE in HMC5883L.h


// Oct 12 , 2016 Flash memory add-on
// Version 1.02



#include <htc.h>
#include <stddef.h>
#include <conio.h>
#include <stdlib.h>
#include "i2cMaster.h"
#include "MPU6050.h"
#include "AT24CM02.h"



#ifndef _XTAL_FREQ
 // Unless specified elsewhere, 4MHz system frequency is assumed
 #define _XTAL_FREQ 32000000
#endif




#ifdef __XC__
// CONFIG1
#pragma config FOSC = INTOSC // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON // Low-Voltage Programming Enable (Low-voltage programming enabled)

#else

__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_OFF & MCLRE_ON & BOREN_OFF & CP_OFF & CPD_OFF & CLKOUTEN_OFF & IESO_OFF & FCMEN_OFF);
__CONFIG(WRT_OFF & PLLEN_OFF & BORV_LO & LVP_ON);
__IDLOC(0000);




#ifndef BORV_LO
#define BORV_LO BORV_19
#endif


#endif

// store/read gyro offset into flash memory

//Set default value
__EEPROM_DATA(0,0,0,0,0,0,0,0);

typedef struct{
  short GyroX,GyroY,GyroZ;
}settingsStruct;


settingsStruct Setting;

#define iabs(A)  (A<0 ?  (-A) :  A)

 near volatile unsigned short Timerms;       //  Interrupt Timer counter in ms 
 volatile unsigned short TimerCrash;   // Interrupt Timer counter in ms  

 
 
 // variable for Gyro offset calculation
 long  AccGyroX;
 long  AccGyroY;
 long  AccGyroZ;
 short AccGyroCount;
 
 //unsigned short Timerms;       //  Interrupt Timer counter in ms 
 //unsigned short TimerCrash;   // Interrupt Timer counter in ms  

 
// MODE OF OPERATION
//  
//  IDLE  => Do nothing
//  INFO  => Display every second the MPU6050 Data
//  READY => Wait until the unit  get more than 1.5G 
//  HIT   => Output data no matter what
 

#define MODE_IDLE      			0
#define MODE_READY  	      	1
#define MODE_HIT			 	3
#define MODE_GYRO_OFFSET        4
#define MODE_DUMP               5
#define MODE_INFO				100

volatile unsigned char Mode= 0xff;
volatile unsigned char NewMode = MODE_IDLE;

// serial buffer 
#define SERIAL_BUFFER_SIZE 32
volatile unsigned char InFiFo, OutFiFo;   // these are the buffer pointers for interrupt serial communication; InFiFo ctrl by putch, OutFiFo ctrl by interrupt
char SerialBuffer[SERIAL_BUFFER_SIZE];


// eerom function
// EEPROM LOAD AND SAVE SETTING
void LoadSetting(void)
{
  unsigned char  idx;
  unsigned char  * pointer = (unsigned char *) &Setting;

  for(idx=0; idx < sizeof(Setting);idx++)
     *(pointer++) = eeprom_read(idx);
}

void SaveSetting(void)
{
  unsigned char  idx;
  unsigned char  * pointer = (unsigned char *) &Setting;
 
  for(idx=0; idx < sizeof(Setting);idx++)
      eeprom_write(idx, *(pointer++));
}


short GetGyroWithOffset(short value, short Goffset)
{
    short _temp;
    
    if(value > 0)
    {
      if(Goffset <0)
      {
          _temp = 32767 +Goffset;
          if(value >= _temp)
              return 32767;
      }
    }   
    else 
    {
    if(Goffset >0)
    {
        _temp = -32768 + Goffset;
        if(value <= _temp)
            return -32768;
    }
    }
      return(value - Goffset);
}



void Init1msTimer()   
{

// we want 1mss period so
// base osc= 32Mhz   clock= 32Mhz/4 
// clock of 8Mhz = 125ns   1ms / 125ns = 8000
// 8000  prescaler of 64 = 125 start count
T2CON=0;
 PR2 = 125; 
TMR2=0;
T2CON= 0b00000111; // Timer on prescaller 64
 // Enable IRQ
TMR2IF=0;
Timerms=0;
TMR2IE=1;
PEIE = 1;
GIE=1;
}




void CalculateSumOfSquare(void);


static void interrupt isr(void){
// check serial transmit Interrupt
if(TXIE)
 if(TXIF)
  {
     // do we have a new char to send
    if(InFiFo != OutFiFo)
      {
        TXREG= SerialBuffer[OutFiFo];
        OutFiFo++;
       if(OutFiFo >= SERIAL_BUFFER_SIZE)
         OutFiFo=0;
      }
     else 
   if(OutFiFo == InFiFo)
     {
       // no more char to send
       // just disable interrupt
       TXIE=0;
     }
  }

// Timer 1 ms
	if(TMR2IF){
 TMR2IF=0; 
 Timerms++;
}
}



void putch(char char_out)
{
   unsigned char temp;

// increment InFiFo and  loop resize if need it.
   temp = InFiFo + 1;
   if(temp >= SERIAL_BUFFER_SIZE)
     temp = 0;

//  wait  if buffer full
  while(temp == OutFiFo);      

// ok write the buffer
  SerialBuffer[InFiFo]=char_out;
// now tell the interrupt routine we have a new char
InFiFo= temp;
// and enable interrupt
 TXIE=1;    
}





// from  	http://www.azillionmonkeys.com/qed/sqroot.html
static unsigned short isqrt(unsigned long val) {
	    unsigned long temp, g=0, b = 0x8000, bshft = 15;
	    do {
	        if (val >= (temp = (((g << 1) + b)<<bshft--))) {
	           g += b;
	           val -= temp;
	        }
	    } while (b >>= 1);
	    return g;
	}


void printValue(short value)
{
    char buffer[16];
    itoa(buffer,value,10);
    cputs(buffer);
}


void printCentiValue(long value)
{
  char buffer[16];
  long _lvalue;
  unsigned short T;

  if(value <0)
   {
    putch('-');
    _lvalue = (-value); 
   }
  else
   _lvalue = value;
  
  itoa(buffer,(short) (_lvalue /100),10);cputs(buffer);
  putch('.');

  T = (unsigned short) _lvalue % 100;

  if(T < 10)
    putch('0');
  utoa(buffer,T,10);cputs(buffer);  
   
}

void printDeciValue(long value)
{
  char buffer[16];
  long _lvalue;
  unsigned short T;

  if(value <0)
   {
    putch('-');
    _lvalue = (-value); 
   }
  else
   _lvalue = value;
  
  itoa(buffer,(short) (_lvalue /10),10);cputs(buffer);
  putch('.');

  T = (unsigned short) _lvalue % 10;

  utoa(buffer,T,10);cputs(buffer);  
   
}



void printGForce(long RawG)
{
    // result are in 16G=32768
    // 16 * 100  for centi value
    //  1600 / 32768  =>  50/1024
  RawG *= 50L;
  RawG /= 1024L;
  printCentiValue(RawG);
}

void printGyro(long RawG)
{
    // result in degree per second
    //setting  is 2000 degree/sec
    // 2000 * 10 in deci value
    // 20000 / 32768 => 625 / 1024
    RawG *= 625L;
    RawG /= 1024L;
    printDeciValue(RawG);   
}

 void printUShort( unsigned short value)
{
   char buffer[16];
   utoa(buffer,value,10); cputs(buffer);
}
     



void CalculateSumOfSquares(void)
{

unsigned long GxSquare, GySquare, GzSquare;
long temp;

 // Calculate The Total Force
 // GForce = sqrt(x**2 + y**2 + z**2)
 // First  set them all positive. Sum of three 15 bits square won't overflow 32 bits unsigned
  
     
// iabs  ~9us
    GxSquare = (unsigned long)  iabs(CurrentData.Gx);
    GySquare = (unsigned long)  iabs(CurrentData.Gy);
    GzSquare = (unsigned long)  iabs(CurrentData.Gz);



  GxSquare *= GxSquare;
  GySquare *= GySquare;
  GzSquare *= GzSquare;
  CurrentData.SumSquare =  GxSquare + GySquare + GzSquare;

}

 
char FindDeltaG(void)
{
   // if all absolute acceleration axis outside 0.8 and 1.2G this is a hit
  // 1.2 G is equivalent to 1.2 *  32768/16 => 2458
     const long HG = 3072L * 3072L;
     const long LG = 1638L * 1638L;
     
  if(CurrentData.SumSquare > HG)
      return 1;
  if(CurrentData.SumSquare < LG) 
      return 1;     
   return 0;

}



void DisplayInfo(void)
{
  unsigned short  Gt;
  
  const char * cy=" y=";
  const char * cz=" z=";
  
  // assuming that interrupt routine is off
  CalculateSumOfSquares();
  // now let's display the info
  cputs(" T(ms)=");
  printUShort(CurrentData.Timer);
  cputs(" G x=");
  printGForce(CurrentData.Gx);
  cputs(cy);
  printGForce(CurrentData.Gy);
  cputs(cz);
  printGForce(CurrentData.Gz);
  // now  for the full force we will do the square root of the sum of the squares.
  Gt = isqrt(CurrentData.SumSquare);
  cputs(" Gt=");
  printGForce(Gt);

  cputs(" Gyro x=");
  printGyro(GetGyroWithOffset(CurrentData.Gyrox,Setting.GyroX));
  cputs(cy);
  printGyro(GetGyroWithOffset(CurrentData.Gyroy,Setting.GyroY));
  cputs(cz);
  printGyro(GetGyroWithOffset(CurrentData.Gyroz,Setting.GyroZ));
  
#ifdef HMC5883L_ENABLE
  cputs(" Mag x=");
  printValue(CurrentData.MagX);  
  cputs(cy);   
  printValue(CurrentData.MagY);
  cputs(cz);        
  printValue(CurrentData.MagZ);
#endif
  cputs("\r\n");  
}

void putHexNibble(unsigned char value)
{
   value &= 0xf;
   if(value > 9)
     value +=  'A' - 10;
 else
    value += '0';
   putch(value);

}



void putHex(unsigned short value)
{

    putHexNibble(value >> 12);
    putHexNibble(value >> 8);
    putHexNibble(value >> 4);
    putHexNibble(value);
}

void DisplayData(void)
{
// data will be display in hex since atoi is way too slow
// the format will be  TTTTXXXXYYYYZZZZ\r\n  (total of 18 characters).
// where 
// TTTT is 16 bits hex   time in ms.
// XXXX is 16 bits hex   Raw Gx value
// XXXX is 16 bits hex   Raw Gy value
// XXXX is 16 bits hex   Raw Gz value
   
   
   putHex(CurrentData.Timer);
   putHex(CurrentData.Gx);
   putHex(CurrentData.Gy);
   putHex(CurrentData.Gz);
   
   
   
   putHex(GetGyroWithOffset(CurrentData.Gyrox,Setting.GyroX));
   putHex(GetGyroWithOffset(CurrentData.Gyroy,Setting.GyroY));
   putHex(GetGyroWithOffset(CurrentData.Gyroz,Setting.GyroZ));
   
#ifdef HMC5883L_ENABLE
   putHex(CurrentData.MagX);
   putHex(CurrentData.MagY);
   putHex(CurrentData.MagZ);
#endif
   cputs("\r\n");

}


void printVersion(void)
{
   cputs("Catapult V1.03\r\n");
}


void printVoltage(void)
{
  // A/D use VDD has REFERENCE  and WE will check 2.048V Reference
  // so formula is   A/D Value = 2.048V * 1024 / VDD
  //         
  //  then
  //
  //       VDD =  2.048 * 1024 / (A/D Value)
  //  with the CentiValue
  //
  //       VDD = 2097,152 / (A/D Value)
  //   

   unsigned short value;
   long  VDDValue;
   // Enable A/D And VFR
    // Right Justified, clock / 64 , VRef=VDD
    ADCON1= 0b11100000;
    // FIXED VOLTAGE REFERENCE  2.048V For A/D
    FVRCON= 0b11000010;
    // A/D Always on FVR
    ADCON0= 0b01111101;
    //wait a little
    __delay_ms(100);
   // start conversion
     ADGO=1;
   // wait
    while(ADGO);
    value = ADRES;
   cputs("VDD = ");
    if(value ==0)
      cputs("---");
    else
     {
    
       VDDValue = (long)209715 / ((long)value);
       printCentiValue(VDDValue);
     }
    cputs("V\r\n");

    
// disable A/D and VFR
   ADCON0=0;
   FVRCON=0;
}


 main(void){
    unsigned short m_Timer; 
    char UserKey;

	OSCCON		= 0b11110000;	// 32MHz
	OPTION_REG	= 0b00000011;	// pullups on, TMR0 @ Fosc/4/16 ( need to interrupt on every 80 clock 16*5)
	ANSELA		= 0b00000;	    // NO Analog 
	PORTA   	= 0b00100000;	
	WPUA		= 0b00111111;	// pull-up ON 
	TRISA		= 0b00011111;	// ALL INPUT  RA5 OUTPUT
//	VREGCON		= 0b00000010;	// lower power sleep please
    INTCON		= 0b00000000;	// no interrupt  


//  A/D & FVR OFF
     ADCON0=0;
     FVRCON=0;

   // set serial com with 115200 baud

    APFCON = 0b10000100;   // Set Rx/Tx to pin 2&3
    TXSTA = 0b10000010;
    RCSTA = 0;
//    BRGH =0;   //9600
    BRGH=1;      //115200  

    BRG16 = 0;
    SYNC =0;
 //   SPBRGL = 51;  //9600
    SPBRGL = 16; //115200

    SPBRGH =0;
    TXEN =1;   // enable transmitter
    SPEN = 1;  // enable serial port
    CREN = 1;  // enable receiver
    RCIE =0;   // disable received interrup;
    TXIE =0;   // disable transmit interrupt
    RCIF =0;   // clear received flag
    TXIF = 0;
    SCKP = 0;
    ABDEN = 0;
// reset interrupt fifo buffer
    InFiFo=0;
    OutFiFo=0;
    GIE = 1;
    PEIE =1;  // enable peripheral


    // load Gyro offset
    
    LoadSetting();
     

     Init1msTimer() ;



 // wait for  serial uart  i/o pin toggle ( at least one full caracter length) 
  __delay_ms(100);
  printVersion();
  printVoltage();

  // wait 5 seconds 
 __delay_ms(5000);
  i2c_Init();
 __delay_ms(1000); // wait another 2 seconds
  MPU6050_Test_I2C();
  Setup_MPU6050();
  
#ifdef HMC5883L_ENABLE
  Setup_HMC5883L();
#endif

  Mode= RCREG; // clear receive buffer;
  Mode= 255;

 while(1)
{

 // check user command
 if(RCIF)
   { 
     UserKey = RCREG;  // get user command
     
    if ((UserKey == 'D')  || (UserKey == 'd' ))
     {
 #ifdef AT24CM02_ENABLE         
        AT24CM02_BlockIdx=0;
        AT24CM02_ReadBlock(0);
        AT24CM02_BlockID= CurrentData.BlockID;       
        NewMode= MODE_DUMP;
 #else
        NewMode= MODE_IDLE;
 #endif        
    }
    if ((UserKey == 'G' ) || (UserKey == 'g' ))
         NewMode=MODE_READY;
    else if (UserKey== 27)
         NewMode= MODE_IDLE;

    else if ((UserKey== 'I') || (UserKey== 'i'))
         NewMode= MODE_INFO;

    else if ((UserKey == 'H' ) || (UserKey == 'h'))
         NewMode = MODE_HIT;
    else
       if(Mode == MODE_IDLE)
        {
           if  ((UserKey == 'V')  || (UserKey == 'v'))
                    printVoltage();
           else if ((UserKey == 'Z') || (UserKey=='z'))
           {
               NewMode= MODE_GYRO_OFFSET;
               AccGyroX=0;
               AccGyroY=0;
               AccGyroZ=0;
               AccGyroCount=0;
           }
           else if (UserKey =='?')
                    printVersion(); 
       }

  }



  if(Mode != NewMode)
  {
     Mode=NewMode;

     switch(Mode)
    {
       case MODE_GYRO_OFFSET:  cputs("Zero Gyro.\r\n");
                               break;
       case MODE_IDLE:   cputs("IDLE\r\n");
                                      break;
       case MODE_READY: cputs("READY\r\n");
						    // clear the  datas
                                        CurrentData.SumSquare=0;
                                        TimerCrash=65535;
                                        break;
    } 
  }

 if(Mode == MODE_GYRO_OFFSET)
     {
       while(GotInt_MPU6050()==0);
         Get_Accel_Values();
 
         AccGyroX += CurrentData.Gyrox;
         AccGyroY += CurrentData.Gyroy;
         AccGyroZ += CurrentData.Gyroz;
         AccGyroCount++;
         
         if(AccGyroCount>=250)
       {
          AccGyroX/=AccGyroCount;
          AccGyroY/=AccGyroCount;
          AccGyroZ/=AccGyroCount;
 
          Setting.GyroX=AccGyroX;
          Setting.GyroY=AccGyroY;
          Setting.GyroZ=AccGyroZ;

          SaveSetting(); // store into eerom 
                     
          NewMode=MODE_IDLE;
       }
         continue;
     }

 if(Mode == MODE_DUMP)
 {
#ifdef AT24CM02_ENABLE       
        AT24CM02_ReadBlock(AT24CM02_BlockIdx++);
        if(CurrentData.BlockID!= AT24CM02_BlockID)
        { 
            NewMode= MODE_IDLE;
            continue;
        }
        DisplayData();
        if(AT24CM02_BlockIdx >= AT24CM02_BLOCK_MAX)
            NewMode= MODE_IDLE;
#else
        NewMode=MODE_IDLE;

#endif        
         continue;  
         
 }
 
 if(Mode == MODE_HIT)
     {
       while(GotInt_MPU6050()==0);
         Get_Accel_Values();
#ifdef HMC5883L_ENABLE
         Get_HMC5883L_Values();
#endif
         CalculateSumOfSquares();
         DisplayData();
#ifdef AT24CM02_ENABLE
         AT24CM02_WriteBlock(AT24CM02_BlockIdx++);
#endif

         continue;
     }
   if(Mode == MODE_READY)
    {
       di();
        Timerms=0;
       ei();
       Get_Accel_Values();
#ifdef HMC5883L_ENABLE
         Get_HMC5883L_Values();
#endif
       CalculateSumOfSquares();
       if(FindDeltaG())
         {
           DisplayData();
#ifdef AT24CM02_ENABLE
           AT24CM02_BlockIdx=0;
           AT24CM02_BlockID++;    
           AT24CM02_WriteBlock(AT24CM02_BlockIdx++);
#endif
            NewMode= MODE_HIT;
           continue;
         }
    }


   if(Mode == MODE_INFO)
   {
      Get_Accel_Values();
#ifdef HMC5883L_ENABLE
      Get_HMC5883L_Values();
#endif
      DisplayInfo();
      NewMode = MODE_IDLE;
      Mode    = MODE_IDLE;
   }

 }
}


