/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    BspAdc.c
* 功能描述：    Adc驱动层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#include "BspAdc.h"

/* 等待HSI就绪的最大循环次数(防止振荡器异常时死等) */
#define BSP_ADC_HSI_TIMEOUT     (0xFFFFU)

/*
* @brief ADC通道配置结构体
*/
typedef struct
{
    U32             mChannel;       /* ADC通道号 */
    U32             mRank;          /* 规则组转换序列 */
    U32             mSampleTime;    /* 采样时间(ADC时钟周期) */
    GPIO_TypeDef *  mGpioPort;      /* 采样引脚端口 */
    U16             mGpioPin;       /* 采样引脚 */
}ADC_CH_CFG;

/* ADC句柄 */
static ADC_HandleTypeDef sAdcHandle;

/*
* @brief		ADC通道配置表
* @note        STM32L151C8T6A: PB14 复用为 ADC1_IN20(通道20为Bank A/B公用)
*              采样时间192个ADC时钟周期(约12us @16MHz), 信号源阻抗大时可继续加大
*/
static const ADC_CH_CFG sAdcChCfg[BSP_ADC_CH_MAX] =
{
    {ADC_CHANNEL_20, ADC_REGULAR_RANK_1, ADC_SAMPLETIME_192CYCLES, GPIOB, GPIO_PIN_14},     //PB14 -> ADC1_IN20
};

/*
* @brief		ADC采样引脚初始化
* @param [in]	无
* @param [out]	无
* @return	    无
* @note        模拟输入引脚需配置为 GPIO_MODE_ANALOG 且无上下拉
*/
static void BspAdcGpioInit(void)
{
    U8               Cnt = 0U;
    GPIO_InitTypeDef GpioInitStruct = {0U};

    for (Cnt = 0U; Cnt < BSP_ADC_CH_MAX; Cnt++)
    {
        GpioInitStruct.Pin  = sAdcChCfg[Cnt].mGpioPin;
        GpioInitStruct.Mode = GPIO_MODE_ANALOG;
        GpioInitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(sAdcChCfg[Cnt].mGpioPort, &GpioInitStruct);
    }

    return;
}

/*
* @brief		ADC时钟初始化
* @param [in]	无
* @param [out]	无
* @return	    无
* @note        ADC1挂在APB2; ADC转换时钟为片内HSI异步时钟(复位后HSI默认开启)
*/
static void BspAdcClockInit(void)
{
    U32 WaitCnt = 0U;

    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* ADC异步时钟源为片内HSI(16MHz): 确保开启并等待稳定(复位后HSI默认已开启) */
    __HAL_RCC_HSI_ENABLE();

    WaitCnt = BSP_ADC_HSI_TIMEOUT;
    while ((__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == RESET) && (WaitCnt > 0U))
    {
        WaitCnt--;
    }

    return;
}

/*
* @brief		ADC初始化
* @param [in]	无
* @param [out]	无
* @return	    无
*/
void BspAdcInit(void)
{
    U8                    Cnt = 0U;
    ADC_ChannelConfTypeDef sChCfg = {0U};

    /* 1. 使能时钟 */
    BspAdcClockInit();

    /* 2. 采样引脚配置为模拟输入 */
    BspAdcGpioInit();

    /* 3. ADC时基配置: 12位右对齐、单通道、单次转换、软件触发 */
    sAdcHandle.Instance                   = ADC1;
    sAdcHandle.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;        /* HSI 16MHz 不分频 */
    sAdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
    sAdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    sAdcHandle.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    sAdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    sAdcHandle.Init.LowPowerAutoWait      = DISABLE;
    sAdcHandle.Init.LowPowerAutoPowerOff  = DISABLE;
    sAdcHandle.Init.ChannelsBank          = ADC_CHANNELS_BANK_A;         /* 通道20为A/B组公用 */
    sAdcHandle.Init.ContinuousConvMode    = DISABLE;
    sAdcHandle.Init.NbrOfConversion       = 1U;
    sAdcHandle.Init.DiscontinuousConvMode = DISABLE;
    sAdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    sAdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;

    if (HAL_ADC_Init(&sAdcHandle) != HAL_OK)
    {
        return;
    }

    /* 4. 通道配置 */
    for (Cnt = 0U; Cnt < BSP_ADC_CH_MAX; Cnt++)
    {
        sChCfg.Channel      = sAdcChCfg[Cnt].mChannel;
        sChCfg.Rank         = sAdcChCfg[Cnt].mRank;
        sChCfg.SamplingTime = sAdcChCfg[Cnt].mSampleTime;

        (void)HAL_ADC_ConfigChannel(&sAdcHandle, &sChCfg);
    }

    return;
}

/*
* @brief		获取ADC原始转换值
* @param [in]	无
* @param [out]	无
* @return	    12位AD值(0~4095); 0表示转换失败
*/
U16 BspAdcGetValue(void)
{
    U32 AdcValue = 0U;

    if (HAL_ADC_Start(&sAdcHandle) != HAL_OK)
    {
        return (0U);
    }

    if (HAL_ADC_PollForConversion(&sAdcHandle, BSP_ADC_TIMEOUT_MS) == HAL_OK)
    {
        AdcValue = HAL_ADC_GetValue(&sAdcHandle);
    }

    (void)HAL_ADC_Stop(&sAdcHandle);

    return ((U16)AdcValue);
}

/*
* @brief		获取多次采样平均值(软件滤波)
* @param [in]	Times  采样次数
* @param [out]	无
* @return	    平均后的AD值
*/
U16 BspAdcGetAvgValue(U8 Times)
{
    U32 Sum = 0U;
    U8  Cnt;
    U8  Num = Times;

    if (Num == 0U)
    {
        return (0U);
    }

    if (Num > BSP_ADC_AVG_MAX_TIMES)
    {
        Num = (U8)BSP_ADC_AVG_MAX_TIMES;
    }

    for (Cnt = 0U; Cnt < Num; Cnt++)
    {
        Sum += (U32)BspAdcGetValue();
    }

    return ((U16)(Sum / Num));
}

/*
* @brief		获取采样电压值
* @param [in]	无
* @param [out]	无
* @return	    电压值(mV)
*/
U16 BspAdcGetVoltageMv(void)
{
    U32 VoltageMv;

    VoltageMv = (((U32)BspAdcGetValue() * BSP_ADC_VREF_MV) / BSP_ADC_MAX_VALUE);

    return ((U16)VoltageMv);
}
