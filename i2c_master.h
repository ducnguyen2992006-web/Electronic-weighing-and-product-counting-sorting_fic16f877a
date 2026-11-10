/* ============================================================================
 * i2c_master.h
 * I2C Master dung phan cung MSSP cua PIC16F877A (SDA=RC4, SCL=RC3).
 * Dung cho LCD I2C (PCF8574).
 * ============================================================================ */
#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <xc.h>
#include "config.h"

#define I2C_CLOCK 100000UL   /* chuan 100kHz */

void I2C_Init(void)
{
    TRISC3 = 1;    /* SCL: de input, MSSP tu keo xuong khi can (open-drain) */
    TRISC4 = 1;    /* SDA: tuong tu */

    SSPCON  = 0b00101000;   /* SSPEN=1, SSPM3:0=1000 -> I2C Master mode */
    SSPCON2 = 0x00;
    SSPSTAT = 0x00;          /* slew rate control bat, chuan cho 100kHz  */
    SSPADD  = (unsigned char)((_XTAL_FREQ / 4UL) / I2C_CLOCK - 1);
}

/* Dung MACRO (khong phai ham) de khong ton them 1 muc ngan xep phan cung
 * (PIC16F877A chi co 8 muc). */
#define I2C_Wait()  do { while ((SSPCON2 & 0x1F) || R_nW); } while (0)

void I2C_Start(void)
{
    I2C_Wait();
    SEN = 1;
    while (SEN);
}

void I2C_Stop(void)
{
    I2C_Wait();
    PEN = 1;
    while (PEN);
}

/* Tra ve 1 neu slave ACK, 0 neu khong (mat lien lac / sai dia chi) */
unsigned char I2C_Write(unsigned char data)
{
    I2C_Wait();
    SSPBUF = data;
    while (BF);
    I2C_Wait();               /* cho xong chu ky ACK roi moi doc ACKSTAT */
    return (unsigned char)(ACKSTAT == 0);
}

#endif /* I2C_MASTER_H */