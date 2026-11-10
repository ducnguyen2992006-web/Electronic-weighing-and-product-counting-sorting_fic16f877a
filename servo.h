#ifndef servo_h
#define servo_h

#include <xc.h>

#ifndef _XTAL_FREQ
#define _XTAL_FREQ 20000000UL
#endif

#define SERVO1 RB0
#define SERVO2 RB1

#define SERVO_1  1
#define SERVO_2  2

/* Phat mot khung xung dieu khien servo.
   Moi don vi delay xap xi 100 us. */
static void Servo_Pulse(unsigned char servo,
                        unsigned char pulse_units)
{
    unsigned int i;

    if (servo == SERVO_1)
        SERVO1 = 1;
    else
        SERVO2 = 1;

    for (i = 0; i < pulse_units; i++)
        __delay_us(100);

    if (servo == SERVO_1)
        SERVO1 = 0;
    else
        SERVO2 = 0;

    for (i = pulse_units; i < 200; i++)
        __delay_us(100);
}

/* Khoi tao hai chan servo */
static void Servo_Init(void)
{
    TRISB0 = 0;
    TRISB1 = 0;

    SERVO1 = 0;
    SERVO2 = 0;
}

/* Dat goc servo tu 0 den 180 do.
   Gui 25 khung xung de servo co thoi gian di chuyen. */

static void Servo_SetAngle(unsigned char servo,
                           unsigned char angle)
{
    unsigned char pulse_units;
    unsigned char i;

    if (angle > 180)
        angle = 180;

    // 500 us -> 0 do
    // 1500 us -> 90 do
    // 2500 us -> 180 do
    pulse_units = 5 + (unsigned char)
                  (((unsigned int)angle * 20 + 90) / 180);

    for (i = 0; i < 25; i++)
    {
        Servo_Pulse(servo, pulse_units);
    }
}

/* Dua servo ve vi tri ban dau */
static void Servo_Reset(void)
{
    Servo_SetAngle(SERVO_1, 45);
    Servo_SetAngle(SERVO_2, 180);
}

#endif