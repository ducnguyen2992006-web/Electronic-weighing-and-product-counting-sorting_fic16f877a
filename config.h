/* ============================================================================
 * config.h (Ch? gi? c?u hình c? b?n và LCD I2C)
 * MCU        : PIC16F877A
 * Compiler   : XC8
 * Th?ch anh  : 20MHz (Có th? thay ??i _XTAL_FREQ n?u dùng t?n s? khác)
 * ============================================================================ */
#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>

#define _XTAL_FREQ 20000000UL

/* ---------------- CONFIG BITS ---------------- */
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP  = OFF
#pragma config CPD  = OFF
#pragma config WRT  = OFF
#pragma config CP   = OFF

/* ---------------- I2C cho LCD I2C (PCF8574) ---------------- */
#define I2C_SDA   RC4
#define I2C_SCL   RC3

#define HE_SO_CAN             500.0f  /* thay bang he so da hieu chuan */
#define SO_LAN_LAY_MAU        5
#define NGUONG_RONG_G         1.0f

#define SERVO1_GOC_DAY        55      /* goc servo 1 day vat */
#define SERVO1_GOC_BAN_DAU    0
#define SERVO2_GOC_BAN_DAU    0
#define SERVO2_GOC_LOAI_1     55
#define SERVO2_GOC_LOAI_2     135

/* So sanh khoi luong theo gram */
#define NGUONG_LOAI_1_G       50.0f
#define NGUONG_LOAI_2_G   

#endif /* CONFIG_H */