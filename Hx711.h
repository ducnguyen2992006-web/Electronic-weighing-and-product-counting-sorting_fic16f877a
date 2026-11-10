#ifndef HX711_H
#define HX711_H

#include <xc.h>
#include <stdint.h>

#ifndef _XTAL_FREQ
#define _XTAL_FREQ 20000000UL
#endif

//==================================================
// 1. KHAI BAO CHAN HX711
//==================================================

#define HX711_DOUT       RA0
#define HX711_SCK        RA1

#define HX711_GAIN_128   128
#define HX711_GAIN_64    64
#define HX711_GAIN_32    32

//==================================================
// 2. BIEN NOI BO
//==================================================

static long HX711_OFFSET = 0;
static float HX711_SCALE = 390;
static unsigned char HX711_GAIN = HX711_GAIN_128;
static unsigned char hx711_error = 0;

//==================================================
// 3. KHOI TAO HX711
//==================================================

static void HX711_SetGain(unsigned char gain);

static void HX711_Begin(unsigned char gain)
{
    // Cau hinh cac chan analog thanh digital
    ADCON1 = 0x06;

    // RA0: DOUT, input
    // RA1: SCK, output
    TRISA0 = 1;
    TRISA1 = 0;

    HX711_SCK = 0;

    HX711_OFFSET = 0;
    HX711_SCALE = 1.0f;
    hx711_error = 0;

    HX711_SetGain(gain);
}

//==================================================
// 4. KIEM TRA HX711 SAN SANG
//==================================================

static unsigned char HX711_IsReady(void)
{
    return (HX711_DOUT == 0);
}

// Cho HX711 san sang, timeout tinh bang ms
static unsigned char HX711_WaitReady(unsigned int timeout_ms)
{
    unsigned int i;

    for (i = 0; i < timeout_ms; i++)
    {
        if (HX711_IsReady())
            return 1;

        __delay_ms(1);
    }

    if (HX711_IsReady())
        return 1;

    hx711_error = 1;
    return 0;
}

//==================================================
// 5. DOC 24 BIT DU LIEU
//==================================================

static long HX711_Read(void)
{
    unsigned long data = 0;
    unsigned char i;
    unsigned char pulses;

    hx711_error = 0;

    if (!HX711_WaitReady(1000))
        return 0;

    // Doc 24 bit
    for (i = 0; i < 24; i++)
    {
        HX711_SCK = 1;
        __delay_us(1);

        data <<= 1;

        if (HX711_DOUT)
            data |= 1UL;

        HX711_SCK = 0;
        __delay_us(1);
    }

    // Chon gain cho lan chuyen doi tiep theo
    if (HX711_GAIN == HX711_GAIN_128)
        pulses = 1;
    else if (HX711_GAIN == HX711_GAIN_32)
        pulses = 2;
    else
        pulses = 3;

    for (i = 0; i < pulses; i++)
    {
        HX711_SCK = 1;
        __delay_us(1);

        HX711_SCK = 0;
        __delay_us(1);
    }

    // Mo rong dau cho du lieu 24 bit co dau
    if (data & 0x800000UL)
        data |= 0xFF000000UL;

    return (long)data;
}

//==================================================
// 6. DOC TRUNG BINH
//==================================================

static long HX711_ReadAverage(unsigned char times)
{
    long sum = 0;
    unsigned char i;

    if (times == 0)
    {
        hx711_error = 1;
        return 0;
    }

    for (i = 0; i < times; i++)
    {
        long reading = HX711_Read();

        if (hx711_error)
            return 0;

        sum += reading;
    }

    return sum / times;
}

//==================================================
// 7. CAN BI
//==================================================

static unsigned char HX711_Tare(unsigned char times)
{
    long value;

    value = HX711_ReadAverage(times);

    if (hx711_error)
        return 0;

    HX711_OFFSET = value;
    return 1;
}

//==================================================
// 8. LAY GIA TRI SAU KHI TRU BI
//==================================================

static long HX711_GetValue(unsigned char times)
{
    long value = HX711_ReadAverage(times);

    if (hx711_error)
        return 0;

    return value - HX711_OFFSET;
}

//==================================================
// 9. CAI DAT HE SO HIEU CHUAN
//==================================================

static void HX711_SetScale(float scale)
{
    if (scale != 0.0f)
        HX711_SCALE = scale;
}

static float HX711_GetScale(void)
{
    return HX711_SCALE;
}

//==================================================
// 10. QUY DOI SANG KHOI LUONG
//==================================================

static float HX711_GetUnits(unsigned char times)
{
    long value;

    if (HX711_SCALE == 0.0f)
    {
        hx711_error = 1;
        return 0.0f;
    }

    value = HX711_GetValue(times);

    if (hx711_error)
        return 0.0f;

    return (float)value / HX711_SCALE;
}

//==================================================
// 11. DOC VA CAI DAT OFFSET
//==================================================

static void HX711_SetOffset(long offset)
{
    HX711_OFFSET = offset;
}

static long HX711_GetOffset(void)
{
    return HX711_OFFSET;
}

//==================================================
// 12. CHON GAIN
//==================================================

static void HX711_SetGain(unsigned char gain)
{
    if (gain != HX711_GAIN_128 &&
        gain != HX711_GAIN_64 &&
        gain != HX711_GAIN_32)
    {
        gain = HX711_GAIN_128;
    }

    HX711_GAIN = gain;
}

//==================================================
// 13. POWER DOWN / POWER UP
//==================================================

static void HX711_PowerDown(void)
{
    HX711_SCK = 0;
    __delay_us(1);

    HX711_SCK = 1;
    __delay_us(80);
}

static void HX711_PowerUp(void)
{
    HX711_SCK = 0;
    __delay_ms(1);
}

//==================================================
// 14. KIEM TRA LOI
//==================================================

static unsigned char HX711_HasError(void)
{
    return hx711_error;
}

#endif