/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    BspGpio.h
* 功能描述：    Gpio驱动层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include "StdSnbc.h"
#include "stm32l1xx_hal.h"
#include "stm32l151xba.h"

/* 输入状态: 1 = 触发(开关闭合, 引脚低电平), 0 = 未触发 */
#define DRV_INPUT_TRIGGER            1U
#define DRV_INPUT_IDLE               0U

/**
 * @brief 管脚上下拉属性
 */
typedef enum
{
	DRV_GPIO_POLL_NA	= 0,					///< 高阻态
	DRV_GPIO_PULL_UP	= 1,					///< 上拉态
	DRV_GPIO_PULL_DOWN	= 2,					///< 下拉态
}DRV_GPIO_POLL_TYPE;

/* 输出极性(板上器件低电平有效) */
typedef enum
{
    DRV_GPIO_OUTPUT_LOW  = 0,
    DRV_GPIO_OUTPUT_HIGH = 1,
}DRV_GPIO_OUTPUT_TYPE;

/*
* @brief	GPIO初始化的结构体
*/
typedef struct
{
    U16			         mGPIOPin;				///< pin引脚
    U16	                 mGPIOMode;				///< Mode
    DRV_GPIO_POLL_TYPE	 mGPIOPullType;		    ///< 内部上拉 下拉
    DRV_GPIO_OUTPUT_TYPE mOutState;             ///< 输出时，用于指定默认的输出电平
}GPIOTypeConf;

/*
* @brief		输入Gpio配置结构体
*/
typedef struct
{
    U8                mNumId;
    GPIO_TypeDef *	  mGpioPort;      // Port
    GPIOTypeConf	  mGpioCfg;       // Pin、方向、上下拉
}GPIO_TYPE_CFG;


typedef enum
{
    DEV_CTRL_SOL1   = 0,
    DEV_CTRL_SOL2   = 1,
    DEV_CTRL_PFET1  = 2,
    DEV_CTRL_PFET2  = 3,
    DEV_CTRL_LOCK   = 4,
    DEV_CTRL_LED    = 5,
    DEV_CTRL_MAX    = 6,
}DEV_CTRL_CFG;

/* 
配置1
Input1:闸门开
Input2:闸门关
Input3:电磁锁
Input4:门禁
*/
typedef enum
{
    DEV_INPUT_1     = 0,
    DEV_INPUT_2     = 1,
    DEV_INPUT_3     = 2,
    DEV_INPUT_4     = 3,
    DEV_INPUT_MAX   = 4,
}DEV_INPUT_CFG;

void BspGpioInit(void);
U8 GetInputPinValue(DEV_INPUT_CFG InputId);
void BspDeviceCtrl(DEV_CTRL_CFG DevId, DRV_GPIO_OUTPUT_TYPE GpioOutType);

#endif

