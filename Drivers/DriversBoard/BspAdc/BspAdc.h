/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    BspAdc.h
* 功能描述：    Adc驱动层头文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include "StdSnbc.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_adc.h"
#include "stm32l151xba.h"

/* ADC通道数量 */
#define BSP_ADC_CH_MAX          (1U)

/* 转换参数 */
#define BSP_ADC_TIMEOUT_MS      (10U)       /* 单次转换超时(ms) */
#define BSP_ADC_MAX_VALUE       (4095U)     /* 12位分辨率: 0~4095 */
#define BSP_ADC_VREF_MV         (3300U)     /* 参考电压(mV), 需按实际VDDA校准 */
#define BSP_ADC_AVG_MAX_TIMES   (16U)       /* 平均滤波最大采样次数 */

/**
* @brief		ADC初始化
* @param [in]	无
* @param [out]	无
* @return	    无
* @note        配置PB14为模拟输入(ADC1_IN20), 单次转换、软件触发
*/
void BspAdcInit(void);

/**
* @brief		获取ADC原始转换值
* @param [in]	无
* @param [out]	无
* @return	    12位AD值(0~4095); 0表示转换失败
*/
U16 BspAdcGetValue(void);

/**
* @brief		获取多次采样平均值(软件滤波)
* @param [in]	Times  采样次数(1~16, 超范围按16处理)
* @param [out]	无
* @return	    平均后的AD值
*/
U16 BspAdcGetAvgValue(U8 Times);

/**
* @brief		获取采样电压值
* @param [in]	无
* @param [out]	无
* @return	    电压值(mV)
*/
U16 BspAdcGetVoltageMv(void);

#endif

