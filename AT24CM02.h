/* 
 * File:   AT24CM02.h
 * Author: daniel
 * 
 * Created on December 29, 2015, 8:20 PM
 */

#ifndef AT24CM02_H
#define	AT24CM02_H

#ifdef	__cplusplus
extern "C" {
#endif

#define AT24CM02_ENABLE

#ifdef AT24CM02_ENABLE    
    
#define AT24CM02_ADDRESS 0x50

#define AT24CM02_BLOCK_MAX 8192
extern short AT24CM02_BlockIdx;
extern long AT24CM02_BlockID;

void AT24CM02_WriteBlock(short block);
void AT24CM02_ReadBlock(short block);

#endif

#ifdef	__cplusplus
}
#endif

#endif	/* AT24CM02_H */

