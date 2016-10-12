
#include "AT24CM02.h"

#ifdef AT24CM02_ENABLE

#include "MPU6050.h"
#include "i2cMaster.h"



short AT24CM02_BlockIdx;
long AT24CM02_BlockID;

unsigned char  AT24CM02_A16A17;
unsigned char  AT24CM02_A8A15;
unsigned char  AT24CM02_A0A7;
short AT24CM02_A0A15;

void AT24CM02_GetAddress(short block)
{
    AT24CM02_A16A17 = ((block >> 11) & 3) + AT24CM02_ADDRESS;
    AT24CM02_A8A15  = (block >> 3);
    AT24CM02_A0A7   = (block << 5) & 0xe0;
}


void AT24CM02_WriteBlock(short block)
{
    if(block >= AT24CM02_BLOCK_MAX)  return;
    CurrentData.BlockID= AT24CM02_BlockID;
    AT24CM02_GetAddress(block);
    LDRAMWriteI2C(AT24CM02_A16A17,AT24CM02_A8A15,AT24CM02_A0A7,(unsigned char *)&CurrentData,32);    
}


void AT24CM02_ReadBlock(short block)
{
    if(block >= AT24CM02_BLOCK_MAX) return;
    AT24CM02_GetAddress(block);
    LDRAMReadI2C(AT24CM02_A16A17,AT24CM02_A8A15,AT24CM02_A0A7,(unsigned char *)&CurrentData,32);
    return;
}
#endif