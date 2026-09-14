/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    BspPwm.h
* 功能描述：    Pwm驱动层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#ifndef __BSP_PWM_H__
#define __BSP_PWM_H__

#include "StdSnbc.h"
#include "stm32l1xx_hal.h"
#include "stm32l151xba.h"

#define PWM_CHANNAL_MAX (2)
#define PWM_TOPLIMIT			    1024	    //Pwm不能大于1024

typedef enum
{
    N_FET1_PWN_CTRL = 0,         //N_FET1
    N_FET2_PWM_CTRL,             //N_FET2
    PWM_CTRL_MAX,
}PWM_CTRL_ID;

void BspPwmUpdate(PWM_CTRL_ID DevId, U32 PwmValue);
void BspPwmUpdateNow(PWM_CTRL_ID DevId, U32 PwmValue);
void BspPwmInit(void);
void BspPwmStop(PWM_CTRL_ID DevId);
void BspPwmStart(PWM_CTRL_ID DevId);

#endif


