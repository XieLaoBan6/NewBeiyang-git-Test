/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    MidMotor.h
* 功能描述：    Motor中间层头文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#ifndef __MID_MOTOR_H__
#define __MID_MOTOR_H__

#include "StdSnbc.h"

/**
* @brief 电机动作
*/
typedef enum
{
    MOTOR_ACTION_CLOSE_SHUTTER = 0,     /* 关闭闸门: PFET2高端常通 + NFET2低端PWM, 到位检测IN3 */
    MOTOR_ACTION_OPEN_SHUTTER,          /* 打开闸门: PFET1高端常通 + NFET1低端PWM, 到位检测IN4 */
    MOTOR_ACTION_MAX,
}MOTOR_ACTION;

/**
* @brief 电机运行状态
*/
typedef enum
{
    MOTOR_STATE_IDLE = 0,               /* 空闲(未执行动作) */
    MOTOR_STATE_RUN,                    /* 动作执行中 */
    MOTOR_STATE_OK,                     /* 动作成功(到位) */
    MOTOR_STATE_ERR,                    /* 动作失败(超时/异常) */
    MOTOR_STATE_MAX,
}MOTOR_STATE;

typedef enum
{
    MOTOR_STATE_CLOSE = 0,
    MOTOR_STATE_START = 1,
    MOTOR_STATE_OTHER = 2,
}MOTOR_START_ACTION;

extern U32                     gMotorTime;
extern MOTOR_START_ACTION      gMotorStartAction;

/**
* @brief		电机控制初始化
* @param [in]	无
* @param [out]	无
* @return	    无
* @note        需在BspGpioInit()/BspPwmInit()之后调用
*/
void MidMotorInit(void);

/**
* @brief		启动电机动作
* @param [in]	Action  动作类型(MOTOR_ACTION)
* @param [out]	无
* @return	    0:启动成功  1:参数错误或上一动作未完成
*/
U8 MidMotorStart(MOTOR_ACTION Action);

/**
* @brief		电机动作轮询(非阻塞状态机)
* @param [in]	无
* @param [out]	无
* @return	    无
* @note        在主循环中周期调用, 时基为HAL_GetTick()(1ms)
*/
void MidMotorPoll(void);

/**
* @brief		获取电机动作状态
* @param [in]	无
* @param [out]	无
* @return	    MOTOR_STATE
*/
MOTOR_STATE MidMotorGetState(void);

/**
 * @brief		电机全开使能：上下桥臂全导通（仅用于测试/调试，慎用）
 * @param [in]	无
 * @param [out]	无
 * @return	    无
 * @note        上下桥臂全导通，存在直通风险，仅用于出厂测试
 */
void MidMotor_EnableAll(void);

/**
* @brief		设置电机动作状态
* @param [in]	无
* @param [out]	无
* @return	    MOTOR_STATE
*/
void MidMotorSetState(MOTOR_STATE CurState);

void MidMotorChange(void);
#endif

