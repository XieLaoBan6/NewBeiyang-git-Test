/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    MidLed.h
* 功能描述：    Led中间层头文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#ifndef __MID_LED_H__
#define __MID_LED_H__

typedef enum
{
    LED_CLOSE = 0,
    LED_OPEN,
    LED_BLINK,
    LED_STATUS_MAX,
}LED_STATUS;


/**
* @brief LED模式状态
* @note  主循环(协议层)写、SysTick中断读，跨上下文共享，必须 volatile
*/
extern volatile LED_STATUS gLedPollState;

void LedModePoll(void);
void LedModeInit(void);

#endif
