#include <xc.h>
#include <stdint.h>

/*========================================================
   PIC16F877A - CONFIG
========================================================*/
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 20000000UL


/*========================================================
   HX711
========================================================*/
#define HX_DOUT    PORTAbits.RA0
#define HX_SCK     PORTAbits.RA1

#define HX_DOUT_TRIS TRISAbits.TRISA0
#define HX_SCK_TRIS  TRISAbits.TRISA1


/*========================================================
   LED
========================================================*/
#define LED1 PORTBbits.RB1
#define LED2 PORTBbits.RB2
#define LED3 PORTBbits.RB3


/*========================================================
   THÔNG S? CÂN
========================================================*/

/*
   Quy ??c:
   100% loadcell = 3000 gram

   N?u mu?n 5 kg:
   #define MAX_WEIGHT 5000L
*/
#define MAX_WEIGHT 3000L


/*
   HX711 gain 128:

   V?i loadcell:
   Sensitivity = 2mV/V
   Excitation   = 5V

   Full scale output ? 10mV

   Giá tr? ADC lý thuy?t kho?ng:
   4,194,304 counts
*/
#define RAW_FULL_SCALE 4194304L


/*========================================================
   BI?N
========================================================*/
int32_t hx_offset = 0;


/*========================================================
   KH?I T?O PIC
========================================================*/
void PIC_Init(void)
{
    /*
       R?T QUAN TR?NG:

       PIC16F877A m?c ??nh PORTA là analog.
       Chuy?n t?t c? PORTA thành DIGITAL.
    */
    ADCON1 = 0x06;

    /*
       RA0 = INPUT
       RA1 = OUTPUT
    */
    HX_DOUT_TRIS = 1;
    HX_SCK_TRIS = 0;

    /*
       RB1 RB2 RB3 = OUTPUT
    */
    TRISBbits.TRISB1 = 0;
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB3 = 0;

    /*
       Giá tr? ban ??u
    */
    HX_SCK = 0;

    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
}


/*========================================================
   KI?M TRA HX711
========================================================*/
uint8_t HX711_Ready(void)
{
    if (HX_DOUT == 0)
        return 1;

    return 0;
}


/*========================================================
   ??C HX711
========================================================*/
int32_t HX711_Read(void)
{
    uint8_t i;
    uint32_t data = 0;

    /*
       Ch? HX711 s?n sàng
    */
    while (!HX711_Ready())
    {
        __delay_us(10);
    }


    /*
       ??c 24 bit
    */
    for (i = 0; i < 24; i++)
    {
        HX_SCK = 1;
        __delay_us(1);

        data <<= 1;

        if (HX_DOUT)
        {
            data |= 1;
        }

        HX_SCK = 0;
        __delay_us(1);
    }


    /*
       Xung th? 25:
       Channel A, Gain = 128
    */
    HX_SCK = 1;
    __delay_us(1);

    HX_SCK = 0;
    __delay_us(1);


    /*
       Chuy?n 24-bit signed sang 32-bit signed
    */
    if (data & 0x800000UL)
    {
        data |= 0xFF000000UL;
    }

    return (int32_t)data;
}


/*========================================================
   ??C TRUNG BÌNH
========================================================*/
int32_t HX711_Average(uint8_t samples)
{
    uint8_t i;
    int64_t sum = 0;

    for (i = 0; i < samples; i++)
    {
        sum += HX711_Read();
        __delay_ms(5);
    }

    return (int32_t)(sum / samples);
}


/*========================================================
   TARE
========================================================*/
void HX711_Tare(void)
{
    /*
       Khi b?t ??u mô ph?ng:
       Full Scale c?a LOADCELL ph?i = 0%
    */

    __delay_ms(500);

    hx_offset = HX711_Average(10);
}


/*========================================================
   TÍNH KH?I L??NG
========================================================*/
int32_t Get_Weight(void)
{
    int32_t raw;
    int64_t weight;

    /*
       ??c HX711
    */
    raw = HX711_Average(5);


    /*
       Tr? giá tr? tare
    */
    raw = raw - hx_offset;


    /*
       N?u âm thì coi là 0
    */
    if (raw < 0)
    {
        raw = 0;
    }


    /*
       N?u v??t full scale
       thì gi?i h?n l?i
    */
    if (raw > RAW_FULL_SCALE)
    {
        raw = RAW_FULL_SCALE;
    }


    /*
       QUAN TR?NG:

       Dùng int64_t ?? tránh tràn s?.

       Ví d?:

       4,194,304 x 3000
       = 12,582,912,000

       s? này v??t gi?i h?n int32.
    */
    weight = ((int64_t)raw * MAX_WEIGHT)
             / RAW_FULL_SCALE;


    if (weight < 0)
    {
        weight = 0;
    }

    if (weight > MAX_WEIGHT)
    {
        weight = MAX_WEIGHT;
    }

    return (int32_t)weight;
}


/*========================================================
   ?I?U KHI?N LED
========================================================*/
void LED_Update(int32_t weight)
{
    /*
       0 - 999g
       LED1
    */
    if (weight < 1000)
    {
        LED1 = 1;
        LED2 = 0;
        LED3 = 0;
    }

    /*
       1000 - 1999g
       LED2
    */
    else if (weight < 2000)
    {
        LED1 = 0;
        LED2 = 1;
        LED3 = 0;
    }

    /*
       >= 2000g
       LED3
    */
    else
    {
        LED1 = 0;
        LED2 = 0;
        LED3 = 1;
    }
}


/*========================================================
   MAIN
========================================================*/
void main(void)
{
    int32_t weight;

    /*
       Kh?i t?o PIC
    */
    PIC_Init();


    /*
       TARE

       Loadcell ph?i ? 0%.
    */
    HX711_Tare();


    /*
       Vòng l?p chính
    */
    while (1)
    {
        /*
           ??c kh?i l??ng
        */
        weight = Get_Weight();


        /*
           ?i?u khi?n LED
        */
        LED_Update(weight);


        /*
           Ch?
        */
        __delay_ms(100);
    }
}