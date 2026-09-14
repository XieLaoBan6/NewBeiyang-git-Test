/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    BspPwm.c
* 功能描述：    Pwm驱动层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#include "BspPwm.h"

//闸门关：N1高，N2高，P1高，P2高


/* TIM2 PWM输出频率(Hz)，两通道共用同一时基、频率一致 */
#define PWM_TIM2_FREQ       (99)

typedef struct 
{
    U32                 mCtrlId;      /* 通道控制ID */
    TIM_TypeDef *       mTimeId;      /* 定时器 */
    U16                 mTimeCh;      /* 定时器通道 */
    GPIO_TypeDef *      mGpioPort;    /* PWM输出端口 */
    U16                 mGpioPin;     /* PWM输出引脚 */
    U16                 mGpioAf;      /* 引脚复用功能号(STM32L1的TIM2为AF1) */
    U16                 mPwmValue;    /* 初始占空比寄存器值 */
}PWM_CFG;

/* PWM通道配置表: 两个N-FET均由TIM2产生PWM
   STM32L151C8T6A: TIM2_CH1 复用引脚 PA5, TIM2_CH4 默认复用引脚 PA3(AF1)
   若硬件映射到其它引脚，修改下表端口/引脚即可 */
static PWM_CFG sPwmCfg[PWM_CHANNAL_MAX] = 
{
    {N_FET1_PWN_CTRL,        TIM2, TIM_CHANNEL_4, GPIOA, GPIO_PIN_3,  GPIO_AF1_TIM2, 0},     //N FET_1  TIM2_CH4 -> PA3 (初值CCR=1024,下管关断)
    {N_FET2_PWM_CTRL,        TIM2, TIM_CHANNEL_1, GPIOA, GPIO_PIN_5,  GPIO_AF1_TIM2, 0},     //N FET_2  TIM2_CH1 -> PA5 (初值CCR=1024,下管关断)
};

/* TIM2 PWM句柄(N_FET1/N_FET2共用一个定时器) */
static TIM_HandleTypeDef sPwmTimHandle;

/**
* @brief		BspPwm定时器工作时钟获取
* @param [in]	无
* @param [out]	无
* @return	    定时器时钟频率(Hz)
* @note        TIM2挂在APB1: APB1不分频时为PCLK1，分频时为2*PCLK1
*/
static U32 BspPwmGetTimerClock(void)
{
    U32 TimerClk = 0U;
    RCC_ClkInitTypeDef RccClkInitStruct = {0U};
    U32 FlashLatency = 0U;

    HAL_RCC_GetClockConfig(&RccClkInitStruct, &FlashLatency);

    TimerClk = HAL_RCC_GetPCLK1Freq();
    if (RccClkInitStruct.APB1CLKDivider != RCC_HCLK_DIV1)
    {
        TimerClk = (TimerClk * 2U);
    }

    return (TimerClk);
}

void BspPwmStop(PWM_CTRL_ID DevId)
{
    if (DevId >= PWM_CTRL_MAX)
    {
        return;
    }

    /* 定时器尚未初始化则直接返回 */
    if (sPwmTimHandle.Instance == NULL)
    {
        return;
    }

    HAL_TIM_PWM_Stop(&sPwmTimHandle, sPwmCfg[DevId].mTimeCh);
}

void BspPwmStart(PWM_CTRL_ID DevId)
{
    if (DevId >= PWM_CTRL_MAX)
    {
        return;
    }

    /* 定时器尚未初始化则直接返回 */
    if (sPwmTimHandle.Instance == NULL)
    {
        return;
    }

    HAL_TIM_PWM_Start(&sPwmTimHandle, sPwmCfg[DevId].mTimeCh);
}

/**
* @brief		BspPwm更新占空比
* @param [in]	DevId      PWM通道ID(N_FET1_PWN_CTRL/N_FET2_PWM_CTRL)
* @param [in]	PwmValue   占空比值 0~PWM_TOPLIMIT(1024) 对应 0~100%
* @param [out]	无
* @return	    无
* @note        需先调用BspPwmInit()完成定时器初始化
*/
void BspPwmUpdate(PWM_CTRL_ID DevId, U32 PwmValue)
{
    U32 CompareValue = 0U;

    if (DevId >= PWM_CTRL_MAX)
    {
        return;
    }

    /* 占空比上限保护 */
    if (PwmValue > PWM_TOPLIMIT)
    {
        PwmValue = PWM_TOPLIMIT;
    }

    /* CCR=PWM_TOPLIMIT(1024)时CNT始终小于CCR，输出恒高，即100%占空比 */
    CompareValue = PwmValue;

    /* 记录当前占空比，供初始化/重新配置时使用 */
    sPwmCfg[DevId].mPwmValue = (U16)CompareValue;

    /* 定时器尚未初始化则直接返回 */
    if (sPwmTimHandle.Instance == NULL)
    {
        return;
    }

    __HAL_TIM_SET_COMPARE(&sPwmTimHandle, sPwmCfg[DevId].mTimeCh, CompareValue);
}

/**
* @brief		BspPwm更新占空比(立即生效)
* @param [in]	DevId      PWM通道ID(N_FET1_PWN_CTRL/N_FET2_PWM_CTRL)
* @param [in]	PwmValue   占空比值 0~PWM_TOPLIMIT(1024) 对应 0~100%
* @param [out]	无
* @return	    无
* @note        与BspPwmUpdate()的区别:
*              1) BspPwmUpdate()写入的是CCR预装载寄存器(HAL的HAL_TIM_PWM_ConfigChannel()
*                 默认置位OCxPE), 需等待下一个更新事件(UEV)才生效, 最大延迟一个PWM周期(约10.11ms);
*              2) 本函数在写入CCR后由软件产生一次更新事件(TIM_EGR_UG), 使新占空比立即生效,
*                 适用于急停、刹车、桥臂关断等对实时性要求高的场合;
*              3) 软件UG会清零计数器CNT, 由于N_FET1/N_FET2共用TIM2,
*                 另一通道的当前PWM周期会被截断一次(仅影响一个周期);
*              4) 置位URS后软件UG不再产生更新中断标志(UIF), 仅刷新预装载寄存器;
*              5) 占空比无变化时不再重复触发UG, 避免频繁清零CNT导致PWM周期被不断重置。
*/
void BspPwmUpdateNow(PWM_CTRL_ID DevId, U32 PwmValue)
{
    if (DevId >= PWM_CTRL_MAX)
    {
        return;
    }

    /* 占空比上限保护 */
    if (PwmValue > PWM_TOPLIMIT)
    {
        PwmValue = PWM_TOPLIMIT;
    }

    /* 记录当前占空比，供初始化/重新配置时使用 */
    sPwmCfg[DevId].mPwmValue = (U16)PwmValue;

    /* 定时器尚未初始化则直接返回 */
    if (sPwmTimHandle.Instance == NULL)
    {
        return;
    }

    /* 与当前已生效的占空比相同则无需重复触发更新事件 */
    if (__HAL_TIM_GET_COMPARE(&sPwmTimHandle, sPwmCfg[DevId].mTimeCh) == PwmValue)
    {
        return;
    }

    /* 写入CCR预装载寄存器 */
    __HAL_TIM_SET_COMPARE(&sPwmTimHandle, sPwmCfg[DevId].mTimeCh, PwmValue);

    /* 软件UG不产生更新中断标志(UIF) */
    sPwmTimHandle.Instance->CR1 |= TIM_CR1_URS;

    /* 软件产生更新事件: 立即将CCR预装载值送入影子寄存器 */
    sPwmTimHandle.Instance->EGR = TIM_EGR_UG;
}

/**
* @brief		BspPwm初始化
* @return		无
*/
void BspPwmInit(void)
{
    U8 Cnt = 0U;
    U32 TimerClk  = 0U;
    U32 Prescaler = 0U;
    U32 Divider   = 0U;
    U32 Period    = 0U;
    GPIO_InitTypeDef    GpioInitStruct = {0U};
    TIM_OC_InitTypeDef  OcInitStruct   = {0U};

    /* 1. 使能定时器及GPIO时钟 */
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 2. PWM输出引脚复用配置
          TIM2_CH1 -> PA5, TIM2_CH4 -> PA3, 复用功能 AF1 */
    for (Cnt = 0U; Cnt < PWM_CTRL_MAX; Cnt++)
    {
        GpioInitStruct.Pin       = sPwmCfg[Cnt].mGpioPin;
        GpioInitStruct.Mode      = GPIO_MODE_AF_PP;
        GpioInitStruct.Pull      = GPIO_NOPULL;
        GpioInitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        GpioInitStruct.Alternate = (uint8_t)sPwmCfg[Cnt].mGpioAf;
        HAL_GPIO_Init(sPwmCfg[Cnt].mGpioPort, &GpioInitStruct);
    }

    /* 3. 时基配置: 占空比调节级数=PWM_TOPLIMIT(1024), PWM频率=PWM_TIM2_FREQ(100Hz)
          实际PWM频率 = TimerClk / ((PSC+1) * PWM_TOPLIMIT) */
    Period   = (U32)PWM_TOPLIMIT - 1U;              /* ARR=1023, CCR有效范围0~1023 */
    Divider  = (U32)PWM_TOPLIMIT * PWM_TIM2_FREQ;   /* 一个PWM周期需要计数的个数 */
    TimerClk = BspPwmGetTimerClock();
    if (Divider != 0U)
    {
        Prescaler = (TimerClk + Divider - 1U) / Divider;    /* 向上取整,保证实际频率不高于目标 */
        if (Prescaler > 1U)
        {
            Prescaler -= 1U;
        }
        else
        {
            Prescaler = 0U;
        }
    }

    sPwmTimHandle.Instance               = sPwmCfg[0].mTimeId;
    sPwmTimHandle.Init.Prescaler         = Prescaler;
    sPwmTimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    sPwmTimHandle.Init.Period            = Period;
    sPwmTimHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    sPwmTimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&sPwmTimHandle) != HAL_OK)
    {
        return;
    }

    /* 4. 逐通道配置PWM模式(边沿对齐PWM1)并启动输出
          N_FET1 = TIM2_CH4, N_FET2 = TIM2_CH1 */
    OcInitStruct.OCMode     = TIM_OCMODE_PWM1;
    OcInitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    OcInitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    for (Cnt = 0U; Cnt < PWM_CTRL_MAX; Cnt++)
    {
        OcInitStruct.Pulse = sPwmCfg[Cnt].mPwmValue;
        (void)HAL_TIM_PWM_ConfigChannel(&sPwmTimHandle, &OcInitStruct, sPwmCfg[Cnt].mTimeCh);
        (void)HAL_TIM_PWM_Start(&sPwmTimHandle, sPwmCfg[Cnt].mTimeCh);
    }
}


