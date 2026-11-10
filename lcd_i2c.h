/* ============================================================================
 * lcd_i2c.h
 * Driver LCD 16x2 qua module I2C (PCF8574 / PCF8574A).
 *
 * Day la BAN THAY THE cho file lcd.h ban da gui (lcd.h ban goc dieu khien
 * LCD kieu song song 6 day D4-D7,RS,EN, khong phai I2C). Vi de bai yeu cau
 * "LCD I2C" nen file nay viet lai toan bo phan giao tiep qua I2C, nhung
 * GIU NGUYEN ten cac ham cong khai (Lcd_Init, Lcd_Clear, Lcd_Set_Cursor,
 * Lcd_Write_Char, Lcd_Write_String) de main.c dung y het nhu voi lcd.h cu.
 *
 * Neu thuc te module LCD cua ban la loai song song (khong phai I2C), bao
 * lai de dung file lcd.h ban goc (chi can them 1 file dinh nghia chan
 * D4..D7, RS, EN, EN tro ve dung PORT thuc te).
 *
 * Dia chi I2C mac dinh cua module la 0x27 (PCF8574) -> da <<1 thanh 0x4E.
 * Neu module cua ban la 0x3F (PCF8574A) thi doi LCD_I2C_ADDR thanh 0x7E.
 * ============================================================================ */
#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <xc.h>
#include "config.h"
#include "i2c_master.h"

#ifndef LCD_I2C_ADDR          /* cho phep chuong trinh thu (lcd_test.c) tu dinh nghia dia chi */
#define LCD_I2C_ADDR   0x4E   /* = 0x27 << 1. Doi thanh 0x7E neu module 0x3F */
#endif
#define LCD_BACKLIGHT  0x08
#define LCD_EN_BIT     0x04

/* Gui 1 byte (lenh rs=0 hoac du lieu rs=1) bang DUNG 1 giao dich I2C:
 *   START, dia chi, [hi|EN], [hi], [lo|EN], [lo], STOP
 * Truoc day moi nibble phai START/STOP rieng qua 3 ham long nhau (sau 9 muc
 * ngan xep -> tran stack, cham gap 2.5 lan). Gio chi con: Lcd_xxx -> LCD_I2C_Byte
 * -> I2C_Write (3 muc). */
static void LCD_I2C_Byte(unsigned char value, unsigned char rs)
{
    unsigned char ctl = (unsigned char)(LCD_BACKLIGHT | (rs ? 0x01 : 0x00));
    unsigned char hi  = (unsigned char)((value & 0xF0) | ctl);
    unsigned char lo  = (unsigned char)(((unsigned char)(value << 4) & 0xF0) | ctl);

    I2C_Start();
    I2C_Write(LCD_I2C_ADDR);
    I2C_Write((unsigned char)(hi | LCD_EN_BIT));
    I2C_Write(hi);
    I2C_Write((unsigned char)(lo | LCD_EN_BIT));
    I2C_Write(lo);
    I2C_Stop();
    __delay_us(50);
}

/* Chi dung trong Lcd_Init: gui MOT nibble (4 bit cao) de ep LCD vao che do 4-bit */
static void LCD_I2C_InitNibble(unsigned char nibble)
{
    unsigned char d = (unsigned char)((nibble << 4) | LCD_BACKLIGHT);
    I2C_Start();
    I2C_Write(LCD_I2C_ADDR);
    I2C_Write((unsigned char)(d | LCD_EN_BIT));
    I2C_Write(d);
    I2C_Stop();
    __delay_us(50);
}

/* Gui 1 lenh day du 8-bit */
void Lcd_Cmd(unsigned char cmd)
{
    LCD_I2C_Byte(cmd, 0);
}

void Lcd_Write_Char(char a)
{
    LCD_I2C_Byte((unsigned char)a, 1);
}

void Lcd_Clear(void)
{
    LCD_I2C_Byte(0x01, 0);
    __delay_ms(2);
}

/* a = dong (1 hoac 2), b = cot (1..16). Giong het chu ky ham cua lcd.h goc */
void Lcd_Set_Cursor(char a, char b)
{
    unsigned char addr;
    if (a == 1) addr = (unsigned char)(0x80 + b - 1);
    else        addr = (unsigned char)(0xC0 + b - 1);
    Lcd_Cmd(addr);
}

void Lcd_Init(void)
{
    __delay_ms(50);
    LCD_I2C_InitNibble(0x03);
    __delay_ms(5);
    LCD_I2C_InitNibble(0x03);
    __delay_us(150);
    LCD_I2C_InitNibble(0x03);
    LCD_I2C_InitNibble(0x02);      /* chuyen sang che do 4-bit */

    Lcd_Cmd(0x28);   /* 4-bit, 2 dong, font 5x8 */
    Lcd_Cmd(0x0C);   /* bat man hinh, tat con tro */
    Lcd_Cmd(0x06);   /* tu dong tang dia chi con tro sau moi ky tu */
    LCD_I2C_Byte(0x01, 0);   /* xoa man hinh (goi thang, khong qua Lcd_Clear de bot 1 muc stack) */
    __delay_ms(2);
}

/* PHAI la "const char *": chuoi hang nam trong Flash cua PIC, neu khai bao
 * "char *" thi trinh bien dich doc nham vao vung RAM -> hien ky tu rac. */
void Lcd_Write_String(const char *a)
{
    unsigned int i;
    for (i = 0; a[i] != '\0'; i++)
        Lcd_Write_Char(a[i]);
}

/* In so nguyen khong dau len LCD, thay cho sprintf (rat nang RAM tren PIC16) */
void Lcd_Write_UInt(unsigned int v)
{
    char tmp[6];
    unsigned char i = 0;
    if (v == 0) { Lcd_Write_Char('0'); return; }
    while (v > 0 && i < 6) { tmp[i++] = (char)('0' + (v % 10)); v /= 10; }
    while (i > 0) Lcd_Write_Char(tmp[--i]);
}


#endif /* LCD_I2C_H */