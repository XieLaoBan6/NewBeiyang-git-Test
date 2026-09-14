/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    MidSensor.c
* 功能描述：    传感器中间层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#include "Midsensor.h"
#include "../PslDriver.h"

DEV_SENSOR_STATE gDevSensorState;


void CheckSensorState(void)
{
    gDevSensorState.mDoorCheckSensor = GetInputPinValue(DEV_INPUT_4);
    gDevSensorState.mMicroLockSensor = GetInputPinValue(DEV_INPUT_3);
    gDevSensorState.mShutCloseSensor = GetInputPinValue(DEV_INPUT_2);
    gDevSensorState.mShutOpenSensor  = GetInputPinValue(DEV_INPUT_1);
} 

