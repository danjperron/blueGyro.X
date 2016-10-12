#include <htc.h>
#include <stddef.h>
#include <conio.h>
#include <stdlib.h>
#include "i2cMaster.h"
#include "MPU6050.h"


int MPU6050_Test_I2C(void)
{
    unsigned char Data = 0x00;
    LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_WHO_AM_I, &Data, 1);
 
    if(Data == 0x68)
    {
  //    cputs("I2C Read Test Passed, MPU6050 Address: 0x68\r\n");       
      return(1);   
   }
     
  // cputs("ERROR: I2C Read Test Failed, Stopping\r\n");
    cputs("ERROR: I2C\r\n");
    return(0);
}



void Setup_MPU6050(void)
{
   int loop;
   unsigned char TheReg;
  
// I need code space. Just create a table with register to clear
const unsigned char MPU6050RegTable[]= {
    MPU6050_RA_FF_THR,    		// 0x1d Freefall threshold of |0mg|  
    MPU6050_RA_FF_DUR,    		// 0x1e Freefall duration limit of 0 
    MPU6050_RA_MOT_THR,		    // 0x1f Motion threshold of 0mg  
    MPU6050_RA_MOT_DUR,    		// 0x20 Motion duration of 0s    
    MPU6050_RA_ZRMOT_THR,    	// 0x21 Zero motion threshold    
    MPU6050_RA_ZRMOT_DUR,    	// 0x22 Zero motion duration threshold
    MPU6050_RA_FIFO_EN,    		// 0x23 Disable sensor output to FIFO buffer
    MPU6050_RA_I2C_MST_CTRL,    // 0x24 AUX I2C setup    //Sets AUX I2C to single master control, plus other config
    MPU6050_RA_I2C_SLV0_ADDR,   // 0x25 Setup AUX I2C slaves
    MPU6050_RA_I2C_SLV0_REG,    // 0x26	
    MPU6050_RA_I2C_SLV0_CTRL,  	// 0x27
    MPU6050_RA_I2C_SLV1_ADDR,   // 0x28
    MPU6050_RA_I2C_SLV1_REG,    // 0x29
    MPU6050_RA_I2C_SLV1_CTRL,   // 0x2a
    MPU6050_RA_I2C_SLV2_ADDR,   // 0x2b
    MPU6050_RA_I2C_SLV2_REG,    // 0x2c
    MPU6050_RA_I2C_SLV2_CTRL,   // 0x2d
    MPU6050_RA_I2C_SLV3_ADDR,   // 0x2e
    MPU6050_RA_I2C_SLV3_REG,    // 0x2f
    MPU6050_RA_I2C_SLV3_CTRL,   // 0x30
    MPU6050_RA_I2C_SLV4_ADDR,   // 0x31
    MPU6050_RA_I2C_SLV4_REG,    // 0x32
    MPU6050_RA_I2C_SLV4_DO,     // 0x33
    MPU6050_RA_I2C_SLV4_CTRL,   // 0x34
    MPU6050_RA_I2C_SLV4_DI,     // 0x35
    MPU6050_RA_INT_PIN_CFG,     // 0x37
    MPU6050_RA_INT_ENABLE,      // 0x38
    MPU6050_RA_I2C_SLV0_DO,     // 0x63
    MPU6050_RA_I2C_SLV1_DO,     // 0x64
    MPU6050_RA_I2C_SLV2_DO,     // 0x65
    MPU6050_RA_I2C_SLV3_DO,     // 0x66
    MPU6050_RA_I2C_MST_DELAY_CTRL, // 0x67
    MPU6050_RA_SIGNAL_PATH_RESET,  // 0x68
    MPU6050_RA_MOT_DETECT_CTRL,    // 0x69
    MPU6050_RA_USER_CTRL,          // 0x6a
    MPU6050_RA_CONFIG,
 /*   MPU6050_RA_FF_THR,
    MPU6050_RA_FF_DUR,
    MPU6050_RA_MOT_THR,
    MPU6050_RA_MOT_DUR,
    MPU6050_RA_ZRMOT_THR,
    MPU6050_RA_ZRMOT_DUR,
    MPU6050_RA_FIFO_EN,
    MPU6050_RA_I2C_MST_CTRL,
    MPU6050_RA_I2C_SLV0_ADDR,
    MPU6050_RA_I2C_SLV0_REG,
    MPU6050_RA_I2C_SLV0_CTRL,
    MPU6050_RA_I2C_SLV1_ADDR,
    MPU6050_RA_I2C_SLV1_REG,
    MPU6050_RA_I2C_SLV1_CTRL,
    MPU6050_RA_I2C_SLV2_ADDR,
    MPU6050_RA_I2C_SLV2_REG,
    MPU6050_RA_I2C_SLV2_CTRL,
    MPU6050_RA_I2C_SLV3_ADDR,
    MPU6050_RA_I2C_SLV3_REG,
    MPU6050_RA_I2C_SLV3_CTRL,
    MPU6050_RA_I2C_SLV4_ADDR,
    MPU6050_RA_I2C_SLV4_DO,
    MPU6050_RA_I2C_SLV4_CTRL,
    MPU6050_RA_I2C_SLV4_DI,
    MPU6050_RA_I2C_SLV0_DO,
    MPU6050_RA_I2C_SLV1_DO,
    MPU6050_RA_I2C_SLV2_DO,
    MPU6050_RA_I2C_SLV3_DO,
    MPU6050_RA_I2C_MST_DELAY_CTRL,
    MPU6050_RA_SIGNAL_PATH_RESET,
    MPU6050_RA_MOT_DETECT_CTRL,
    MPU6050_RA_USER_CTRL,
  */  MPU6050_RA_INT_PIN_CFG,
    MPU6050_RA_INT_ENABLE,
    MPU6050_RA_FIFO_R_W,
    0xff
};   


     // V1.01 because of magnetometer sample in reduce to 125Hz
    //Sets sample rate to 8000/1+63 = 125Hz
    // we will transmit accelerometer+gyro
    // this is 4 hex number per values = 4 * 7 + crlf + newline = 30  * 10 bit/character
    LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_SMPLRT_DIV, 63);
    //Disable gyro self tests, scale of 2000 degrees/s
    LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_CONFIG, 0b00011000);
    //Disable accel self tests, scale of +-16g, no DHPF
    LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_CONFIG, 0x18);

    loop=0;
   do
   {
       TheReg = MPU6050RegTable[loop++];
       if(TheReg==0xff) break;
       LDByteWriteI2C(MPU6050_ADDRESS,TheReg,0);
    }while(1);



    //Sets clock source to gyro reference w/ PLL
    LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_PWR_MGMT_1, 0b00000010);
    //Controls frequency of wakeups in accel low power mode plus the sensor standby modes
    LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_PWR_MGMT_2, 0x00);
 
    //MPU6050_RA_WHO_AM_I             //Read-only, I2C address
    LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_INT_ENABLE, 0x01);
 //   cputs("\r\nMPU6050 Setup Complete\r\n");

}


GForceStruct  CurrentData;

extern near volatile unsigned short Timerms;

void Get_Accel_Values(void)
{
char cval[14];
LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_XOUT_H, cval, 14);
      di();
     CurrentData.Timer = Timerms;
      ei(); 
	CurrentData.Gx = ((cval[0]<<8)|cval[1]);
	CurrentData.Gy = ((cval[2]<<8)|cval[3]);
	CurrentData.Gz = ((cval[4]<<8)|cval[5]);
    CurrentData.temp = ((cval[6]<<8)|cval[7]);
    CurrentData.Gyrox = ((cval[8]<<8) | cval[9]);
    CurrentData.Gyroy = ((cval[10]<<8) | cval[11]);
    CurrentData.Gyroz = ((cval[12]<<8) | cval[13]);
   }




unsigned char GotInt_MPU6050(void)
{
  unsigned char uc_temp;

// Do we have a new data

	LDByteReadI2C(MPU6050_ADDRESS,MPU6050_RA_INT_STATUS, &uc_temp, 1);

  return ((uc_temp & 1) == 1 ? 1 : 0);
}	
