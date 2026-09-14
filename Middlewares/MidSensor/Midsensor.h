/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    MidSensor.h
* 功能描述：    传感器中间层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#ifndef __MID_SENSOR_H__
#define __MID_SENSOR_H__

#include "StdSnbc.h"

/**
* @brief 传感器状态集合
* @note  取值约定: 1 = 传感器已触发(到位/闭合), 0 = 未触发(断开)
*        硬件开关多为低电平有效(闭合接地), 采集时需转换: 引脚读到低电平 -> 置1
*        各传感器与输入引脚的对应关系以硬件原理图为准
*/
typedef struct
{
    U8 mShutOpenSensor;     /* 闸门开到位传感器: 1 = 闸门已运动到开启位(开位微动开关闭合) */
    U8 mShutCloseSensor;    /* 闸门关到位传感器: 1 = 闸门已运动到关闭位(关位微动开关闭合) */
    U8 mDoorCheckSensor;    /* 门检测传感器:     1 = 门已关好/门到位(门状态检测) */
    U8 mMicroLockSensor;    /* 锁微动开关传感器: 1 = 锁舌已到位/处于锁闭状态 */
}DEV_SENSOR_STATE;

extern DEV_SENSOR_STATE gDevSensorState;

void CheckSensorState(void);

#endif
