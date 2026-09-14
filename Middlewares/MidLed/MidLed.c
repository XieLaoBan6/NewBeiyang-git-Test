/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    MidLed.c
* 功能描述：    LED中间层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#include "../PslDriver.h"
#include "MidLed.h"

/* 闪烁时间参数(ms) */
#define LED_BLINK_ON_MS     (1000U)
#define LED_BLINK_OFF_MS    (1000U)

volatile LED_STATUS gLedPollState = LED_OPEN;

static U8           sLedLevel   = 0xFFU;        /* 当前输出电平缓存, 0xFF=未知(强制刷新) */
static U32          sBlinkTick  = 0U;           /* 闪烁相位起始时刻 */
static U8           sBlinkPhase = 0U;           /* 0=亮  1=灭 */
static LED_STATUS   sLastMode   = LED_STATUS_MAX;

/**
* @brief		LED电平控制	
* @author		XLB
* @version		1.000	
* @note         仅在电平发生变化时才写GPIO
* @date			2023-10-27																
*/
static void MidLedCtrl(DRV_GPIO_OUTPUT_TYPE LedState)
{
    if(LedState > DRV_GPIO_OUTPUT_HIGH)
    {
        return;
    }

    if(sLedLevel == (U8)LedState)
    {
        return;
    }

    sLedLevel = (U8)LedState;

    BspDeviceCtrl(DEV_CTRL_LED, LedState);
}

/**
* @brief		LED闪烁控制(非阻塞)
* @author		XLB
* @version		1.000	
* @note         以HAL_GetTick()为时间基准, 闪烁周期与调用频率无关
* @date			2023-10-27																
*/
static void LedBlinkCtrl(void)
{
    U32 Now    = HAL_GetTick();
    U32 Period = ((sBlinkPhase == 0U) ? LED_BLINK_ON_MS : LED_BLINK_OFF_MS);

    MidLedCtrl(((sBlinkPhase == 0U) ? DRV_GPIO_OUTPUT_HIGH : DRV_GPIO_OUTPUT_LOW));

    if((U32)(Now - sBlinkTick) >= Period)       /* 减法比较, U32溢出安全 */
    {
        sBlinkTick  = Now;
        sBlinkPhase = (U8)(sBlinkPhase ^ 1U);
    }
}

/**
* @brief		LED模式控制	
* @author		XLB
* @version		1.000	
* @note         在SysTick_Handler中每1ms调用; 运行于中断上下文, 禁止任何阻塞操作(含HAL_Delay)
* @date			2023-10-27																
*/
void LedModePoll(void)
{
    LED_STATUS Mode = gLedPollState;

    if(Mode >= LED_STATUS_MAX)
    {
        MidLedCtrl(DRV_GPIO_OUTPUT_LOW);

        return;
    }

    if(Mode != sLastMode)                       /* 模式切换: 复位闪烁相位并强制刷新输出 */
    {
        sLastMode   = Mode;
        sBlinkTick  = HAL_GetTick();
        sBlinkPhase = 0U;
        sLedLevel   = 0xFFU;
    }

    switch(Mode)
    {
        case LED_CLOSE:
            MidLedCtrl(DRV_GPIO_OUTPUT_HIGH);
            break;
        case LED_OPEN:
            MidLedCtrl(DRV_GPIO_OUTPUT_LOW);
            break;
        case LED_BLINK:
            LedBlinkCtrl();
            break;
        default:
            MidLedCtrl(DRV_GPIO_OUTPUT_LOW);
            break;
    }

    return;
}


/**
* @brief		LED模式初始化	
* @author		XLB
* @version		1.000	
* @note         
* @date			2023-10-27																
*/
void LedModeInit(void)
{
    gLedPollState = LED_OPEN;
}

