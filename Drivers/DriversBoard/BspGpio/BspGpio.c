/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    BspGpio.c
* 功能描述：    Gpio驱动层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#include "BspGpio.h"

/*
* @brief		输入Gpio配置表
* @param [in]	无
* @param [out]	无
* @return	    无	
*/
static GPIO_TYPE_CFG sInputGpio[DEV_INPUT_MAX] =
{
	{DEV_INPUT_1, GPIOB,	{(U16)GPIO_PIN_3, (U16)GPIO_MODE_INPUT, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
	{DEV_INPUT_2, GPIOB,	{(U16)GPIO_PIN_4, (U16)GPIO_MODE_INPUT, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
	{DEV_INPUT_3, GPIOB,	{(U16)GPIO_PIN_5, (U16)GPIO_MODE_INPUT, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
    {DEV_INPUT_4, GPIOB,    {(U16)GPIO_PIN_6, (U16)GPIO_MODE_INPUT, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
};

/*
* @brief		设备器件控制配置表
* @param [in]	无
* @param [out]	无
* @return	    无	
*/
static GPIO_TYPE_CFG sDevDevCtrlConfig[DEV_CTRL_MAX] =
{
    {DEV_CTRL_SOL1,  GPIOA,  {(U16)GPIO_PIN_1,  (U16)GPIO_MODE_OUTPUT_PP, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
    {DEV_CTRL_SOL2,  GPIOA,  {(U16)GPIO_PIN_2,  (U16)GPIO_MODE_OUTPUT_PP, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
    {DEV_CTRL_PFET1, GPIOA,  {(U16)GPIO_PIN_6,  (U16)GPIO_MODE_OUTPUT_PP, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
    {DEV_CTRL_PFET2, GPIOA,  {(U16)GPIO_PIN_7,  (U16)GPIO_MODE_OUTPUT_PP, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
    {DEV_CTRL_LOCK,  GPIOB,  {(U16)GPIO_PIN_0,  (U16)GPIO_MODE_OUTPUT_PP, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
    {DEV_CTRL_LED,   GPIOB,  {(U16)GPIO_PIN_12, (U16)GPIO_MODE_OUTPUT_PP, DRV_GPIO_POLL_NA, DRV_GPIO_OUTPUT_LOW}},
};

/*
* @brief		输出引脚初始化
* @param [in]	无
* @param [out]	无
* @return	    无	
*/
/* 器件输出统一映射: 板上器件为低电平有效, DRV_GPIO_OUTPUT_HIGH 对应引脚输出低 */
static void BspGpioWriteOut(GPIO_TypeDef *pPort, U16 Pin, DRV_GPIO_OUTPUT_TYPE State)
{
    GPIO_PinState PinState = GPIO_PIN_RESET;

    if (State == DRV_GPIO_OUTPUT_HIGH)
    {
        PinState = GPIO_PIN_SET;
    }

    HAL_GPIO_WritePin(pPort, Pin, PinState);
}

static void BspGpioPinInit(const GPIO_TYPE_CFG *GpioCfg, U8 GpioNum)
{
    U8 Cnt = 0U;

    if (GpioCfg == NULL)
    {
        return;
    }

    for (Cnt = 0U; Cnt < GpioNum; Cnt++)
    {
        const GPIO_TYPE_CFG *pCfg = &GpioCfg[Cnt];
        GPIO_InitTypeDef     GpioInitStruct = {0};   /* 每项独立初始化，避免字段残留 */

        GpioInitStruct.Pin  = (uint32_t)pCfg->mGpioCfg.mGPIOPin;
        GpioInitStruct.Mode = (uint32_t)pCfg->mGpioCfg.mGPIOMode;
        GpioInitStruct.Pull = (uint32_t)pCfg->mGpioCfg.mGPIOPullType;

        HAL_GPIO_Init(pCfg->mGpioPort, &GpioInitStruct);

        if (pCfg->mGpioCfg.mGPIOMode != GPIO_MODE_INPUT)
        {
            GpioInitStruct.Speed = GPIO_SPEED_FREQ_LOW;   /* 输出低速，降低EMI与功耗 */
            /* 仅输出引脚设置默认电平，输入引脚写ODR无意义 */
            BspGpioWriteOut(pCfg->mGpioPort, pCfg->mGpioCfg.mGPIOPin, pCfg->mGpioCfg.mOutState);
        }
    }
}


/*
* @brief		GPIO引脚时钟初始化
* @param [in]	无
* @param [out]	无
* @return	    无
*/
static void BspGpioClockInit(void)
{
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

/*
* @brief		引脚初始化
* @param [in]	无
* @param [out]	无
* @return	    无	
*/
void BspGpioInit(void)
{
    BspGpioClockInit();

    BspGpioPinInit(&sInputGpio[0], (sizeof(sInputGpio) / sizeof(sInputGpio[0])));
    BspGpioPinInit(&sDevDevCtrlConfig[0], (sizeof(sDevDevCtrlConfig) / sizeof(sDevDevCtrlConfig[0])));
}

/*
* @brief		获取输入引脚的值
* @param [in]	无
* @param [out]	无
* @return	    无	
*/
U8 GetInputPinValue(DEV_INPUT_CFG InputId)
{
    GPIO_PinState PinValue;

    if (InputId >= DEV_INPUT_MAX)
    {
        return (DRV_INPUT_IDLE);        /* 非法ID返回"未触发", 防止被误判为到位 */
    }

    PinValue = HAL_GPIO_ReadPin(sInputGpio[InputId].mGpioPort,
                                (uint16_t)sInputGpio[InputId].mGpioCfg.mGPIOPin);

    if (PinValue == GPIO_PIN_RESET)
    {
        return (DRV_INPUT_TRIGGER);     /* 低电平有效: 开关闭合 */
    }

    return (DRV_INPUT_IDLE);
}


/*
* @brief		设备器件输出引脚控制
* @param [in]	无
* @param [out]	无
* @return	    无	
*/
void BspDeviceCtrl(DEV_CTRL_CFG DevId, DRV_GPIO_OUTPUT_TYPE GpioOutType)
{
    if ((DevId >= DEV_CTRL_MAX) || (GpioOutType > DRV_GPIO_OUTPUT_HIGH))
    {
        return;
    }

    BspGpioWriteOut(sDevDevCtrlConfig[DevId].mGpioPort,
                    sDevDevCtrlConfig[DevId].mGpioCfg.mGPIOPin,
                    GpioOutType);
}
