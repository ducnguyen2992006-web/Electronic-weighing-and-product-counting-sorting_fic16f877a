#include <xc.h>
#include <stdio.h>

#include "config.h"
#include "i2c_master.h"
#include "lcd_i2c.h"
#include "Hx711.h"
#include "servo.h"

//==================================================
// CAU HINH PHAN LOAI
//==================================================

#define NGUONG_L1       10.0f
#define NGUONG_L2       20.0f

#define SERVO_DAY       1
#define SERVO_PHANLOAI  2

// He so tam thoi, can hieu chuan voi can thuc te
#define HE_SO_CAN       950.0f

// Nguong xac dinh da lay vat khoi can
#define NGUONG_CAN_RONG 5.0f

//==================================================
// BIEN TOAN CUC
//==================================================

unsigned int dem_L1 = 0;
unsigned int dem_L2 = 0;
unsigned int dem_L3 = 0;

unsigned char da_xuly = 0;
unsigned char loai = 0;

long raw_thuc = 0;
float khoiluong = 0.0f;

char dong1[25];
char dong2[17];

//==================================================
// TIMER1: CHI BAT SAU KHI DA CAN XONG VAT
//RegValue = 65536-(Delay * Fosc)/(Prescalar*4)) = 65536-((100ms * 20Mhz)/(8*4)) = 59286
// 20 MHz
// 10 ms
// Nap lai Timer1 = 65536 - 6250 = 59286
//==================================================

#define THOI_GIAN_TIMER_MS   1000U    // Them 1 moc thoi gian: 1000 ms ke tu luc can xong
#define TIMER1_TICKS_CAN_CHO (THOI_GIAN_TIMER_MS / 10U)

volatile unsigned char timer1_dem_10ms = 0;

void Timer1_Init(void)
{
    T1CON = 0x30;                  // Internal clock, prescaler 1:8, Timer1 tat
    TMR1IE = 0;
    TMR1IF = 0;
    TMR1=59286;
}

void Timer1_BatSauKhiCan(void)
{
    TMR1ON = 0;
    TMR1IE = 0;
    timer1_dem_10ms = 0;
    TMR1=59286;
    TMR1IF = 0;
    TMR1IE = 1;
    PEIE = 1;
    GIE = 1;
    TMR1ON = 1;
}

void Timer1_ChoDuThoiGian(void)
{
    while (timer1_dem_10ms < TIMER1_TICKS_CAN_CHO)
    {
        // Doi timer du moc thoi gian; khong sua __delay_ms goc
    }
}

void Timer1_Tat(void)
{
    TMR1IE = 0;
    TMR1ON = 0;
    TMR1IF = 0;
}

// PIC16F877A chi co mot vector ngat.
// Khong bat Timer1 khi doc HX711; Timer1 chi chay sau khi co khoi luong.
void __interrupt() ISR(void)
{
    if (TMR1IE && TMR1IF)
    {
        TMR1=59286;
        TMR1IF = 0;
        if (timer1_dem_10ms < 250U)
            timer1_dem_10ms++;
    }
}

//==================================================
// HIEN THI LCD 16x2
// Dong 1: RAW va khoi luong
// Dong 2: so luong tung loai
//==================================================

void HienThiLCD(long raw, float gram)
{
    // Gioi han dong hien thi de khong tran bo dem
    // Neu RAW qua lon, chi hien thi 6 chu so cuoi
    long raw_hien_thi = raw;

    if (raw_hien_thi > 999999L)
        raw_hien_thi = raw_hien_thi % 1000000L;

    if (raw_hien_thi < -99999L)
        raw_hien_thi = raw_hien_thi % 100000L;

    sprintf(dong1, "R:%ld W:%d g",
            raw_hien_thi, (int)gram);

    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("                ");
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String(dong1);

    sprintf(dong2, "L1:%u L2:%u L3:%u",
            dem_L1, dem_L2, dem_L3);

    Lcd_Set_Cursor(2, 1);
    Lcd_Write_String("                ");
    Lcd_Set_Cursor(2, 1);
    Lcd_Write_String(dong2);
}

//==================================================
// PHAN LOAI SAN PHAM
// Loai 1: < 50 g
// Loai 2: >= 50 g va < 100 g
// Loai 3: >= 100 g
//==================================================

unsigned char PhanLoai(float gram)
{
    if (gram < NGUONG_L1)
    {
        return 1;
    }
    else if (gram < NGUONG_L2)
    {
        return 2;
    }
    else
    {
        return 3;
    }
}

//==================================================
// DIEU KHIEN SERVO PHAN LOAI
//==================================================

void XuLySanPham(unsigned char loai_sp)
{
    // Bao tren LCD
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("Dang phan loai  ");

    //----------------------------------------------
    // SERVO 1: DAY VAT KHOI BAN CAN
    //----------------------------------------------
    
    //----------------------------------------------
    // SERVO 2: PHAN LOAI VAT
    //----------------------------------------------

    switch (loai_sp)
    {
        case 1:
            __delay_ms(500);
            Servo_SetAngle(SERVO_DAY, 0);
            //__delay_ms(500);

            Servo_SetAngle(SERVO_DAY, 45);
            //__delay_ms(500);
            Servo_SetAngle(SERVO_PHANLOAI, 50);
            dem_L1++;
            break;

        case 2:
            __delay_ms(500);
            Servo_SetAngle(SERVO_DAY, 0);
            //__delay_ms(500);

            Servo_SetAngle(SERVO_DAY, 45);
            //__delay_ms(500);
            Servo_SetAngle(SERVO_PHANLOAI, 130);
            dem_L2++;
            break;

        case 3:
            __delay_ms(500);
            Servo_SetAngle(SERVO_DAY, 0);
            //__delay_ms(500);

            Servo_SetAngle(SERVO_DAY, 45);
            //__delay_ms(500);
            Servo_SetAngle(SERVO_PHANLOAI, 180);
            dem_L3++;
            break;

        default:
            return;
    }

    __delay_ms(3000);

    // Dua servo 2 ve vi tri cho
    Servo_SetAngle(SERVO_PHANLOAI, 180);

    __delay_ms(300);
}

//==================================================
// CHUONG TRINH CHINH
//==================================================

    // Khoi 
void main(void)
{
    // Khoi tao cac cong vao/ra
    ADCON1 = 0x06;

    PORTA = 0;
    PORTC = 0;
    PORTD = 0;

    // Khoi tao LCD I2C
    I2C_Init();
    __delay_ms(100);

    Lcd_Init();
    Lcd_Clear();

    // Khoi tao 2 servo
    Servo_Init();
    Servo_Reset();

    // Khoi tao timer, NHUNG CHUA BAT DEM
    Timer1_Init();

    // Khoi tao HX711
    HX711_Begin(HX711_GAIN_128);

    // Hien thi huong dan can chinh 0
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("Dang can chinh 0");

    Lcd_Set_Cursor(2, 1);
    Lcd_Write_String("Bo trong can");

    __delay_ms(1000);

    // Can bi khi chua dat vat len
    if (!HX711_Tare(10))
    {
        Lcd_Clear();

        Lcd_Set_Cursor(1, 1);
        Lcd_Write_String("Loi HX711!");

        Lcd_Set_Cursor(2, 1);
        Lcd_Write_String("Kiem tra day");

        while (1)
        {
            __delay_ms(500);
        }
    }

    // Cai dat he so quy doi
    HX711_SetScale(HE_SO_CAN);

    Lcd_Clear();

    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("Can san sang");

    __delay_ms(800);
    Lcd_Clear();

    //==================================================
    // VONG LAP CHINH
    //==================================================

    while (1)
    {
        // Doc gia tri RAW da tru gia tri bi
        raw_thuc = HX711_GetValue(5);

        if (HX711_HasError())
        {
            Lcd_Set_Cursor(1, 1);
            Lcd_Write_String("Loi doc HX711   ");

            __delay_ms(200);
            continue;
        }

        // Doi RAW sang gam theo he so can
        khoiluong = HX711_GetUnits(5);

        if (HX711_HasError())
        {
            __delay_ms(100);
            continue;
        }

        // Neu gia tri am thi lay tri tuyet doi
        if (khoiluong < 0.0f)
        {
            khoiluong = -khoiluong;
        }

        // Hien thi RAW, khoi luong va bo dem
        HienThiLCD(raw_thuc, khoiluong);

        //----------------------------------------------
        // KIEM TRA VAT TREN CAN
        //----------------------------------------------

        if (khoiluong < NGUONG_CAN_RONG)
        {
            // Can gan 0: san sang nhan vat tiep theo
            da_xuly = 0;
        }
        else if (da_xuly == 0)
        {
            // Danh dau da xu ly vat hien tai
            // Tranh servo lap lai khi vat con tren can
            da_xuly = 1;

            // Xac dinh loai san pham
            loai = PhanLoai(khoiluong);

            // THEM: chi sau khi CAN XONG moi cho Timer1 bat dau dem.
            Timer1_BatSauKhiCan();

            // servo phan loai quay TRUOC khi servo day hoat dong.
            // Ham Servo_SetAngle goc van phat 25 xung (~500 ms).
            if (loai == 1)
                Servo_SetAngle(SERVO_PHANLOAI, 50);
            else if (loai == 2)
                Servo_SetAngle(SERVO_PHANLOAI, 130);
            else
                Servo_SetAngle(SERVO_PHANLOAI, 180);

            // THEM: timer dem du 1000 ms tu thoi diem can xong.
            // Sau do ham XuLySanPham GOC van giu __delay_ms(500).
            Timer1_ChoDuThoiGian();
            Timer1_Tat();

            // Dieu khien 2 servo - GIU NGUYEN ham XuLySanPham goc
            XuLySanPham(loai);

            // Cap nhat bo dem sau khi phan loai
            HienThiLCD(raw_thuc, khoiluong);
        }

        __delay_ms(150);
    }
}
