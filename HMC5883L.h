/* 
 * File:   HMC5883L.h
 * Author: Daniel Perron
 *
 * Created on December 29, 2015, 11:12 AM
 */

#ifndef HMC5883L_H
#define	HMC5883L_H

#ifdef	__cplusplus
extern "C" {
#endif

    
#define HMC5883L_ENABLE

#define HMC5883L_ADDRESS 0x1e    
#define HMC5883L_CONFIGA 0
#define HMC5883L_CONFIGB 1
#define HMC5883L_MODE    2    
#define HMC5883L_X_MSB   3
#define HMC5883L_X_LSB   4
#define HMC5883L_Z_MSB   5
#define HMC5883L_Z_LSB   6
#define HMC5883L_Y_MSB   7
#define HMC5883L_Y_LSB   8
#define HMC5883L_STATUS  9
#define HMC5883L_IDA     10
#define HMC5883L_IDB     11
#define HMC5883L_IDC     12

void Setup_HMC5883L(void);
void Get_HMC5883L_Values(void);
    
#ifdef	__cplusplus
}
#endif

#endif	/* HMC5883L_H */

