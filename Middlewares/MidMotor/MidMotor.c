/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    MidMotor.c
* 功能描述：    Motor中间层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#include "../PslDriver.h"
#include "MidMotor.h"
#include "MidSensor/Midsensor.h"
#include "stm32l1xx_hal.h"

#define MOTOR_PWM_DUTY_EXIT_SESSION     (PWM_TOPLIMIT)
#define MOTOR_PWM_DUTY_START            (512U)
#define MOTOR_PWM_DUTY_RUN              (700U)
#define MOTOR_PWM_DUTY_START_SESSION    (0)

/* 闸门动作时序参数(单位: ms) */
#define MOTOR_PRECHARGE_MS      (20U)       /* 高端预充电: 两端P管同时导通 */
#define MOTOR_SLOW_RUN_MS       (200U)      /* 50%占空比启动运行时间(实测192~197ms) */
#define MOTOR_HOLD_MS           (20U)       /* 到位后保持运行时间(实测18.8/19.4ms) */
#define MOTOR_BRAKE_MS          (20U)       /* 到位后高端短路刹车时间 */
#define MOTOR_INPLACE_DEBOUNCE_MS (3U)      /* 微动开关消抖时间(实测抖动1.2~1.7ms) */
#define MOTOR_TIMEOUT_MS        (1000U)     /* 动作总超时保护(实测453ms/715ms) */

/* 急停时序参数(单位: ms, 以gMotorTime为计时基准) */
#define MOTOR_ESTOP_NBRAKE_MS   (10U)       /* 阶段1: 两侧下管50%导通泄流 */
#define MOTOR_ESTOP_POFF_MS     (20U)       /* 阶段2: 两侧上管关断; 之后进入阶段3 */

/* 闸门动作Ad检测值*/
#define MOTOR_AD_THRESHOLD      (1000U)
/**
* @brief 闸门动作配置
*/
typedef struct
{
    DEV_CTRL_CFG    mPfetMain;          /* 常通高端P管(动作方向对应的一端) */
    DEV_CTRL_CFG    mPfetAux;           /* 辅助高端P管(预充电/刹车用) */
    PWM_CTRL_ID     mPwmCh;             /* 低端PWM通道 */
    DEV_INPUT_CFG   mInPlaceInput;      /* 到位检测输入(低电平有效) */
    U16             mStartDelayMs;      /* 预充电结束到PWM启动的延时 */
}MOTOR_ACTION_CFG;

/**
* @brief 闸门动作步骤
*/
typedef enum
{
    MOTOR_STEP_IDLE = 0,        /* 空闲 */
    MOTOR_STEP_PRECHARGE,       /* 高端预充电 */
    MOTOR_STEP_START_DELAY,     /* 启动延时 */
    MOTOR_STEP_RUN_SLOW,        /* 低速运行(50%) */
    MOTOR_STEP_RUN_FAST,        /* 运行(74%)并等待到位 */
    MOTOR_STEP_HOLD,            /* 到位后保持运行(压实) */
    MOTOR_STEP_BRAKE,           /* 高端短路刹车 */
    MOTOR_STEP_MAX,
}MOTOR_STEP;

static const MOTOR_ACTION_CFG sMotorActionCfg[MOTOR_ACTION_MAX] =
{
    /* 关闸门: PFET2常通 + NFET2(TIM2_CH1)PWM, 到位检测IN2, PWM紧随预充电启动 */
    {DEV_CTRL_PFET2, DEV_CTRL_PFET1, N_FET2_PWM_CTRL, DEV_INPUT_2, 0U},
    /* 开闸门: PFET1常通 + NFET1(TIM2_CH4)PWM, 到位检测IN1, PWM延后约10ms启动 */
    {DEV_CTRL_PFET1, DEV_CTRL_PFET2, N_FET1_PWN_CTRL, DEV_INPUT_1, 10U},
};

static MOTOR_STATE      sMotorState  = MOTOR_STATE_IDLE;
static MOTOR_STEP       sMotorStep   = MOTOR_STEP_IDLE;
static MOTOR_ACTION     sMotorAction = MOTOR_ACTION_CLOSE_SHUTTER;
static U32              sStepTick    = 0U;      /* 当前步骤起始时刻 */
static U32              sActionTick  = 0U;      /* 动作起始时刻(超时计时) */
static U8               sInPlaceFlg  = 0U;      /* 到位信号低电平计时标志 */
static U32              sInPlaceTick = 0U;      /* 到位信号低电平起始时刻 */
U32                     gMotorTime   = 0U;
MOTOR_START_ACTION      gMotorStartAction = MOTOR_STATE_CLOSE;
static U32              sEstopBase   = 0U;      /* 急停时序计时基准(变化即视为新一轮急停) */
static U8               sEstopStage  = 0U;      /* 急停时序已完成阶段(防止重复写寄存器) */

/**
 * @brief		电机急停：关闭全部输出，上下桥臂全关断（安全状态）	
* @author		XLB
 * @param [in]	无
 * @param [out]	无
 * @return	    无
 * @note        
 */
void MidMotor_ForceStop(void)
{
    BspPwmUpdateNow(N_FET2_PWM_CTRL, MOTOR_PWM_DUTY_EXIT_SESSION);
    BspPwmUpdateNow(N_FET1_PWN_CTRL, MOTOR_PWM_DUTY_EXIT_SESSION);
    BspDeviceCtrl(DEV_CTRL_PFET1, DRV_GPIO_OUTPUT_HIGH);
    BspDeviceCtrl(DEV_CTRL_PFET2, DRV_GPIO_OUTPUT_HIGH);
}

/**
 * @brief		电机急停：关闭全部输出，上下桥臂全关断（安全状态）	
* @author		XLB
 * @param [in]	无
 * @param [out]	无
 * @return	    无
 * @note        
 */
void MidMotor_EmergencyStop(void)
{
    U32 Now     = HAL_GetTick();
    U32 Elapsed = 0U;

    /* 计时基准变化 → 新一轮急停, 时序阶段复位 */
    if (gMotorTime != sEstopBase)
    {
        sEstopBase  = gMotorTime;
        sEstopStage = 0U;
        BspPwmStart(N_FET1_PWN_CTRL);
        BspPwmStart(N_FET2_PWM_CTRL);
    }

    Elapsed = Now - gMotorTime;

    if (Elapsed <= MOTOR_ESTOP_NBRAKE_MS)                /* 阶段1: 下管50%导通 */
    {
        if (sEstopStage == 0U)                          /* 1ms周期内只写一次 */
        {
            BspPwmUpdateNow(N_FET2_PWM_CTRL, MOTOR_PWM_DUTY_START);    // CCR=512
            BspPwmUpdateNow(N_FET1_PWN_CTRL, MOTOR_PWM_DUTY_START);
            sEstopStage = 1U;
        }
    }
    else if (Elapsed <= MOTOR_ESTOP_POFF_MS)             /* 阶段2: 上管关断 */
    {
        if (sEstopStage <= 1U)
        {
            BspPwmUpdateNow(N_FET2_PWM_CTRL, MOTOR_PWM_DUTY_EXIT_SESSION); // CCR=1024 → N管全关
            BspPwmUpdateNow(N_FET1_PWN_CTRL, MOTOR_PWM_DUTY_EXIT_SESSION);
            sEstopStage = 2U;
        }
    }
    else if(Elapsed > MOTOR_ESTOP_POFF_MS)                                               /* 阶段3: 下管全关(最终安全态) */
    {
        BspDeviceCtrl(DEV_CTRL_PFET1, DRV_GPIO_OUTPUT_HIGH);    // P管关断
        BspDeviceCtrl(DEV_CTRL_PFET2, DRV_GPIO_OUTPUT_HIGH);
        sEstopStage = 3U;
        
        gMotorStartAction = MOTOR_STATE_OTHER;
    }
}

/**
 * @brief		电机上电/保持：两侧上管导通、两侧下管关断（高端预充电/保持态）	
* @author		XLB
 * @param [in]	无
 * @param [out]	无
 * @return	    无
 * @note        
 */
void MidMotor_EnableAll(void)
{
    BspPwmStop(N_FET1_PWN_CTRL);
    BspPwmStop(N_FET2_PWM_CTRL);
    BspDeviceCtrl(DEV_CTRL_PFET1, DRV_GPIO_OUTPUT_LOW);    // 两侧上管导通
    BspDeviceCtrl(DEV_CTRL_PFET2, DRV_GPIO_OUTPUT_LOW);

    gMotorStartAction = MOTOR_STATE_OTHER;
}

/**
 * @brief		电机动作切换	
* @author		XLB
 * @param [in]	无
 * @param [out]	无
 * @return	    无
 * @note        
 */
void MidMotorChange(void)
{
    switch(gMotorStartAction)
    {
        case MOTOR_STATE_CLOSE:
            MidMotor_EnableAll();
            break;
        case MOTOR_STATE_START:
            MidMotor_EmergencyStop();
            break;
        default:
            break;
    }
}

/**
* @brief		动作结束处理	
* @author		XLB
* @param [in]	Result  1:成功  0:失败
* @param [out]	无
* @return	    无
*/
static void MidMotorFinish(U8 Result)
{
    MidMotor_ForceStop();

    sMotorState = ((Result != 0U) ? MOTOR_STATE_OK : MOTOR_STATE_ERR);
    sMotorStep  = MOTOR_STEP_IDLE;
    sInPlaceFlg = 0U;
}

/**
* @brief		到位检测(微动开关, 低电平有效)	
* @author		XLB
* @param [in]	无
* @param [out]	无
* @return	    1:已到位  0:未到位
* @note        微动开关闭合瞬间存在1.2~1.7ms机械抖动, 需连续低电平才算有效
*/
static U8 MidMotorInPlaceCheck(void)
{
    U8  Ret = 0U;
    U32 Now = HAL_GetTick();

    if (GetInputPinValue(sMotorActionCfg[sMotorAction].mInPlaceInput) == 1U)
    {
        if (sInPlaceFlg == 0U)
        {
            sInPlaceFlg  = 1U;
            sInPlaceTick = Now;
        }
        else if ((Now - sInPlaceTick) >= MOTOR_INPLACE_DEBOUNCE_MS)
        {
            Ret = 1U;
        }
    }
    else
    {
        sInPlaceFlg = 0U;
    }

    return (Ret);
}

/**
* @brief		电机控制初始化	
* @author		XLB
* @param [in]	无
* @param [out]	无
* @return	    无
*/
void MidMotorInit(void)
{
    sMotorState  = MOTOR_STATE_IDLE;
    sMotorStep   = MOTOR_STEP_IDLE;
    sMotorAction = MOTOR_ACTION_CLOSE_SHUTTER;
    sInPlaceFlg  = 0U;
}

/**
* @brief		启动电机动作	
* @author		XLB
* @param [in]	Action  动作类型
* @param [out]	无
* @return	    0:启动成功  1:参数错误或上一动作未完成
*/
U8 MidMotorStart(MOTOR_ACTION Action)
{
    if (Action >= MOTOR_ACTION_MAX)
    {
        return (1U);
    }

    if (sMotorState == MOTOR_STATE_RUN)
    {
        return (1U);
    }

    if(Action == MOTOR_ACTION_CLOSE_SHUTTER)
    {    
        if(gDevSensorState.mShutCloseSensor == STD_TRUE)
        {
            sMotorState = MOTOR_STATE_OK;
            return (0U);
        }
    }

    if(Action == MOTOR_ACTION_OPEN_SHUTTER)
    {    
        if(gDevSensorState.mShutOpenSensor == STD_TRUE)
        {
            sMotorState = MOTOR_STATE_OK;
            return (0U);
        }
    }

    sMotorAction = Action;
    sMotorState  = MOTOR_STATE_RUN;
    sMotorStep   = MOTOR_STEP_PRECHARGE;
    sStepTick    = HAL_GetTick();
    sActionTick  = sStepTick;
    sInPlaceFlg  = 0U;

    /* 两端高端P管导通, 进入预充电阶段 */
    BspDeviceCtrl(sMotorActionCfg[Action].mPfetMain, DRV_GPIO_OUTPUT_LOW);
    BspDeviceCtrl(sMotorActionCfg[Action].mPfetAux,  DRV_GPIO_OUTPUT_LOW);

    return (0U);
}

/**
* @brief		电机动作轮询	
* @author		XLB
* @param [in]	无
* @param [out]	无
* @return	    无
*/
U16 gMotorAd = 0;
void MidMotorPoll(void)
{
    const MOTOR_ACTION_CFG *pCfg = &sMotorActionCfg[sMotorAction];
    U32 Now = HAL_GetTick();
    U16 gMotorAd = BspAdcGetValue();

    if (sMotorState != MOTOR_STATE_RUN)
    {
        return;
    }

    /* 总超时保护: 到位信号异常时不至于一直通电 */
    if (((Now - sActionTick) >= MOTOR_TIMEOUT_MS)
    || (gMotorAd > MOTOR_AD_THRESHOLD))
    {
        MidMotorFinish(0U);
        return;
    }

    switch (sMotorStep)
    {
    case MOTOR_STEP_PRECHARGE:
        if ((Now - sStepTick) > MOTOR_PRECHARGE_MS)
        {
            BspDeviceCtrl(pCfg->mPfetAux, DRV_GPIO_OUTPUT_HIGH);     /* 辅助高端关断 */
            sMotorStep = MOTOR_STEP_START_DELAY;
            sStepTick  = Now;
        }
        break;

    case MOTOR_STEP_START_DELAY:
        BspPwmUpdateNow(pCfg->mPwmCh, MOTOR_PWM_DUTY_RUN);       /* 50%启动 */
        sMotorStep = MOTOR_STEP_RUN_SLOW;
        sStepTick  = Now;
        break;

    case MOTOR_STEP_RUN_SLOW:
        sMotorStep = MOTOR_STEP_RUN_FAST;
        sStepTick  = Now;
        break;

    case MOTOR_STEP_RUN_FAST:
        if (MidMotorInPlaceCheck() != 0U)
        {
            sMotorStep = MOTOR_STEP_HOLD;                           /* 到位后保持运行 */
            sStepTick  = Now;
        }
        break;

    case MOTOR_STEP_HOLD:
        if ((Now - sStepTick) >= MOTOR_HOLD_MS)
        {
            /* 高端短路刹车：两侧上管导通 + 两侧下管关断 → 电机绕组经上管短路制动 */
            BspPwmUpdate(pCfg->mPwmCh, MOTOR_PWM_DUTY_EXIT_SESSION);   // CCR=1024 → 本侧下管关断
            BspDeviceCtrl(pCfg->mPfetAux, DRV_GPIO_OUTPUT_LOW);        // 辅助上管导通
            sMotorStep = MOTOR_STEP_BRAKE;
            sStepTick  = Now;
        }
        break;

    case MOTOR_STEP_BRAKE:
        if ((Now - sStepTick) > MOTOR_BRAKE_MS)
        {
            MidMotorFinish(1U);         /* 刹车结束: 全关并置成功状态 */
        }
        break;

    default:
        MidMotorFinish(0U);
        break;
    }

    return;
}

/**
* @brief		获取电机动作状态	
* @author		XLB
* @param [in]	无
* @param [out]	无
* @return	    MOTOR_STATE
*/
MOTOR_STATE MidMotorGetState(void)
{
    return (sMotorState);
}

/**
* @brief		设置电机动作状态	
* @author		XLB
* @param [in]	无
* @param [out]	无
* @return	    MOTOR_STATE
*/
void MidMotorSetState(MOTOR_STATE CurState)
{
    sMotorState = CurState;
}
