/*
* 文件名称：    AppProtocol.c
* 功能描述：    SC_Sealer 应用层协议解析与命令分发
* 当前版本号:   V2.0
*/

#include "AppProtocol.h"

/* 上报周期宏 ---------------------------------------------------------------*/
#define SC_MOTOR_REPORT_PERIOD_MS   (5000U)  /* 闸门状态上报周期(ms) */

static U8           sSessionOpen    = 0U;       /* 会话状态：1=已建立 */
static U8           sRxBuf[SC_RX_BUF_SIZE];
static U16          sRxLen          = 0U;
static U8           sTxBuf[SC_TX_BUF_SIZE];
static U32          sReportTick     = 0U;       /* 上次闸门状态上报时刻(HAL_GetTick, 1ms) */
static MOTOR_STATE  sReportState    = MOTOR_STATE_IDLE; /* 上次上报时的电机状态 */

/* 内部函数声明 -------------------------------------------------------------*/
static U8  AppCalcXor(const U8 *pData, U16 Len);
static U16 AppBuildFrame(U8 Ver, U8 Cmd, const U8 *pPayload, U16 PayloadLen);
static void AppSendRaw(const U8 *pData, U16 Len);
static void AppSendAck(void);
static void AppSendChecksumErr(void);
static void AppSendStatusErr(U8 Status);
static void AppSendShutterReport(U8 Status);
static void AppHandleFrame(const U8 *pFrame, U16 Len);
static void AppRxProcess(const U8 *pData, U16 Len);
static void AppMotorReportPoll(void);

/**
* @brief  校验和计算：XOR(除STX与校验字节外的所有字节)，结果 < 10 则 +10
*/
static U8 AppCalcXor(const U8 *pData, U16 Len)
{
    U16 i;
    U8  Xor = 0U;

    for (i = 0U; i < Len; i++)
    {
        Xor = (U8)(Xor ^ pData[i]);
    }

    if (Xor < 10U)
    {
        Xor = (U8)(Xor + 10U);
    }

    return (Xor);
}

/**
* @brief  构造应答/上报帧（带 ETX，与真实抓包格式一致）
* @return 帧长度
*/
static U16 AppBuildFrame(U8 Ver, U8 Cmd, const U8 *pPayload, U16 PayloadLen)
{
    U16 idx = 0U;
    U16 EncLen = 0U;

    sTxBuf[idx++] = SC_STX;
    sTxBuf[idx++] = Ver;
    sTxBuf[idx++] = Cmd;
    sTxBuf[idx++] = SC_FLAGS;

    if ((pPayload != NULL) && (PayloadLen > 0U))
    {
        EncLen = MidBase64Encode(pPayload, PayloadLen, &sTxBuf[idx],
                                 (U16)(SC_TX_BUF_SIZE - idx - 2U));
        idx = (U16)(idx + EncLen);
    }

    sTxBuf[idx++] = SC_ETX;
    sTxBuf[idx]   = AppCalcXor(&sTxBuf[1], (U16)(idx - 1U));   /* XOR b[1]..b[idx-1] */
    idx++;

    return (idx);
}

/**
* @brief  原始数据发送
*/
static void AppSendRaw(const U8 *pData, U16 Len)
{
    (void)UsbSendData((U8 *)pData, Len);
}

/**
* @brief  应答 ACK（0x06）
*/
static void AppSendAck(void)
{
    U8 ack = SC_EVT_ACK;

    AppSendRaw(&ack, 1U);
}

/**
* @brief  校验和错误（0x07）
*/
static void AppSendChecksumErr(void)
{
    U8 err = SC_EVT_CHECKSUM_ERR;

    AppSendRaw(&err, 1U);
}

/**
* @brief  命令处理错误（0x08 + 状态码）
*/
static void AppSendStatusErr(U8 Status)
{
    U8 evt[2];

    evt[0] = SC_EVT_STATUS_ERR;
    evt[1] = Status;

    AppSendRaw(evt, 2U);
}

/**
* @brief  闸门动作完成上报（0xCD，负载 6 字节，首字节为 Status）
*/
static void AppSendShutterReport(U8 Status)
{
    U8  payload[SC_PAYLOAD_REPORT_LEN];
    U16 i;
    U16 len;

    for (i = 0U; i < SC_PAYLOAD_REPORT_LEN; i++)
    {
        payload[i] = 0U;
    }
    payload[0] = Status;

    len = AppBuildFrame(SC_VER_CTRL, SC_CMD_SHUTTER_REPORT, payload, SC_PAYLOAD_REPORT_LEN);
    AppSendRaw(sTxBuf, len);
}

/**
* @brief  命令分发
*/
static void AppHandleFrame(const U8 *pFrame, U16 Len)
{
    U8  Cmd = pFrame[2];
    U8  Payload[SC_PAYLOAD_MAX];
    U16 PayloadLen = 0U;
    U16 B64Len;
    U16 i;
    U16 TxLen;

    for (i = 0U; i < SC_PAYLOAD_MAX; i++)
    {
        Payload[i] = 0U;
    }

    /* 负载：存在 ETX 时为 b[4..Len-3]，否则为 b[4..Len-2] */
    if (pFrame[Len - 2U] == SC_ETX)
    {
        B64Len = (U16)(Len - 6U);
    }
    else
    {
        B64Len = (U16)(Len - 5U);
    }

    if (B64Len > 0U)
    {
        PayloadLen = MidBase64Decode(&pFrame[4], B64Len, Payload, SC_PAYLOAD_MAX);
    }

    /* 会话检查：除会话命令外，未建立会话一律返回错误 */
    if ((Cmd != SC_CMD_SESSION) && (sSessionOpen == 0U))
    {
        AppSendStatusErr(SC_STATUS_DOOR_CFG_ERR);
        return;
    }

    switch (Cmd)
    {
    case SC_CMD_SESSION:                            /* START/EXIT_SESSION 命令字节相同，靠负载区分 */
        if (PayloadLen < 1U)
        {
            AppSendStatusErr(SC_STATUS_DOOR_CFG_ERR);
        }
        else if (Payload[0] == SC_SESSION_START)
        {
            if(sSessionOpen == 0)
            {
                sSessionOpen = 1U;
                gLedPollState = LED_BLINK;
                gMotorTime = HAL_GetTick();
                gMotorStartAction = MOTOR_STATE_START;
            }

            AppSendAck();
        }
        else if (Payload[0] == SC_SESSION_EXIT)
        {
            if(sSessionOpen == 1)
            {
                sSessionOpen = 0U;
                gLedPollState = LED_OPEN;
                gMotorStartAction = MOTOR_STATE_CLOSE;
            }
        }
        else
        {
            AppSendStatusErr(SC_STATUS_DOOR_CFG_ERR);
        }
        break;

    case SC_CMD_GET_CTRL_VERSION:                   /* 13字节 = 00×9 + 4字节版本号 */
        Payload[9]  = (U8)SC_CTRL_VER_MAJOR;
        Payload[10] = (U8)SC_CTRL_VER_MINOR;
        Payload[11] = (U8)SC_CTRL_VER_BUILD;
        Payload[12] = (U8)SC_CTRL_VER_REV;
        TxLen = AppBuildFrame(SC_VER_SESSION, Cmd, Payload, SC_PAYLOAD_VERSION_LEN);
        AppSendRaw(sTxBuf, TxLen);
        break;

    case SC_CMD_GET_SENSOR_INFO:                    /* 6字节传感器位图 */
        /* TODO: 接入真实传感器或由串口向Sealer子板转发获取，当前全部为FALSE */
        TxLen = AppBuildFrame(SC_VER_SESSION, Cmd, Payload, SC_PAYLOAD_SENSOR_LEN);
        AppSendRaw(sTxBuf, TxLen);
        break;

    case SC_CMD_GET_INPUTS:                         /* 6字节开关位图，低电平=闭合 */
        gDevSensorState.mDoorCheckSensor = GetInputPinValue(DEV_INPUT_4);
        gDevSensorState.mMicroLockSensor = GetInputPinValue(DEV_INPUT_3);
        gDevSensorState.mShutCloseSensor = GetInputPinValue(DEV_INPUT_2);
        gDevSensorState.mShutOpenSensor  = GetInputPinValue(DEV_INPUT_1);

        Payload[0] = (U8)(gDevSensorState.mShutCloseSensor 
                            | (U8)(gDevSensorState.mDoorCheckSensor << 1)
                            | (U8)(gDevSensorState.mMicroLockSensor << 2)
                            | (U8)(gDevSensorState.mShutOpenSensor << 3));
        
        TxLen = AppBuildFrame(SC_VER_SESSION, Cmd, Payload, SC_PAYLOAD_INPUTS_LEN);
        AppSendRaw(sTxBuf, TxLen);
        break;

    case SC_CMD_CLOSE_SHUTTER:
        if (MidMotorStart(MOTOR_ACTION_CLOSE_SHUTTER) == 1U)
        {
            AppSendStatusErr(SC_STATUS_SHUTTER_BUSY);
        }
        else
        {
            AppSendAck();                           /* 动作完成后由 AppMotorReportPoll 上报 0xCD */
        }
        break;
    case SC_CMD_OPEN_SHUTTER:
        if (MidMotorStart(MOTOR_ACTION_OPEN_SHUTTER) == 1U)
        {
            AppSendStatusErr(SC_STATUS_SHUTTER_BUSY);
        }
        else
        {
            AppSendAck();                           /* 动作完成后由 AppMotorReportPoll 上报 0xCD */
        }
        break;
    default:
        AppSendStatusErr(SC_STATUS_DOOR_CFG_ERR);
        break;
    }
}

/**
* @brief  接收数据处理：累积 + 校验试探定帧界（帧无长度字段）
*/
static void AppRxProcess(const U8 *pData, U16 Len)
{
    U16 n;
    U8  Found;

    if ((sRxLen + Len) > SC_RX_BUF_SIZE)
    {
        sRxLen = 0U;                                /* 溢出保护，丢弃历史数据 */
    }

    (void)memcpy(&sRxBuf[sRxLen], pData, Len);
    sRxLen = (U16)(sRxLen + Len);

    for (;;)
    {
        if (sRxLen < 5U)
        {
            break;
        }

        if (sRxBuf[0] != SC_STX)
        {
            sRxLen = (U16)(sRxLen - 1U);            /* 丢弃非法首字节 */
            (void)memmove(sRxBuf, &sRxBuf[1], sRxLen);
            continue;
        }

        Found = 0U;
        for (n = 5U; n <= sRxLen; n++)
        {
            if ((sRxBuf[3] == SC_FLAGS)
            &&  (AppCalcXor(&sRxBuf[1], (U16)(n - 2U)) == sRxBuf[n - 1U]))
            {
                AppHandleFrame(sRxBuf, n);

                sRxLen = (U16)(sRxLen - n);
                if (sRxLen > 0U)
                {
                    (void)memmove(sRxBuf, &sRxBuf[n], sRxLen);
                }
                Found = 1U;
                break;
            }
        }

        if (Found == 0U)
        {
            if (sRxLen >= SC_RX_BUF_SIZE)           /* 无法解析且缓冲已满，清空 */
            {
                sRxLen = 0U;
            }
            break;
        }
    }
}

/**
* @brief  闸门动作完成上报（状态跳变时立即上报一次，此后每 1s 上报一次）
* @note   时基 HAL_GetTick()(1ms)；状态回到空闲/运行中后重新计时
*/
static void AppMotorReportPoll(void)
{
    MOTOR_STATE State = MidMotorGetState();
    U32         Now   = HAL_GetTick();

    if ((State != MOTOR_STATE_OK) && (State != MOTOR_STATE_ERR))
    {
        sReportState = State;
        sReportTick  = Now;
        return;
    }

    if ((State != sReportState) ||
        ((U32)(Now - sReportTick) >= SC_MOTOR_REPORT_PERIOD_MS))
    {
        sReportState = State;
        sReportTick  = Now;

        if (State == MOTOR_STATE_OK)
        {
            AppSendShutterReport(SC_REPORT_OK);
        }
        else
        {
            AppSendShutterReport(SC_REPORT_ERR);
        }
    }
}

/**
* @brief  协议层初始化
*/
void AppProtocolInit(void)
{
    sSessionOpen    = 0U;
    sRxLen          = 0U;
    sReportTick     = HAL_GetTick();
    sReportState    = MOTOR_STATE_IDLE;
}

/**
* @brief  协议层轮询：取出 USB 数据 -> 定帧 -> 分发 -> 上报闸门状态
* @note   在主循环中调用；USB 接收在中断里完成，此处只做解析与业务处理
*/
void AppProtocolPoll(void)
{
    U8  buf[64];
    U16 num;

    num = UsbRxRead(buf, (U16)sizeof(buf));
    if (num > 1U)
    {
        AppRxProcess(buf, num);
    }
    else if ((num == 1U) && (buf[0] == SC_EVT_ACK))
    {
        MidMotorSetState(MOTOR_STATE_IDLE);
    }

    AppMotorReportPoll();

    UsbTxProcess();
}
