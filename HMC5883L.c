#include "i2cMaster.h"
#include "MPU6050.h"
#include "HMC5883L.h"


#ifdef HMC5883L_ENABLE


void Setup_HMC5883L(void)
{
    
   // set scale to 1.3Gauss
    LDByteWriteI2C(HMC5883L_ADDRESS, HMC5883L_CONFIGB, 0x20);
   // set single shot
    LDByteWriteI2C(HMC5883L_ADDRESS, HMC5883L_MODE, 0x01);
}
    
void Get_HMC5883L_Values(void)
{
    char cval[6];
    unsigned char loop;
    
  //  for(loop=0;loop<6;loop++)
  //      cval[loop]= LDSingleByteReadI2C(HMC5883L_ADDRESS,HMC5883L_X_MSB+loop);
    LDByteWriteI2C(HMC5883L_ADDRESS, HMC5883L_MODE, 0x01);
    LDByteReadI2C(HMC5883L_ADDRESS, HMC5883L_X_MSB, cval,6);
    CurrentData.MagX = ((cval[0]<<8)|cval[1]);
    CurrentData.MagZ = ((cval[2]<<8)|cval[3]);
	CurrentData.MagY = ((cval[4]<<8)|cval[5]);
}


#endif
