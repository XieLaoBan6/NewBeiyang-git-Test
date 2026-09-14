/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usbd_cdc_interface.c
  * @brief   USB CDC 接口层实现（USB 直连应用层模式）
  ******************************************************************************
  * @note    已移除原 ST 例程的 "USB <-> USART1 桥接" 逻辑：
  *          - 不再初始化 USART1 / DMA / TIM3 轮询
  *          - 主机下发的数据存入 RX 环形缓冲，由主循环交给应用层协议处理
  *          - 应用层应答经 UsbSendData() 入队，由 UsbTxProcess() 发出
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_interface.h"

/* Private variables ---------------------------------------------------------*/
extern USBD_HandleTypeDef USBD_Device;      /* 定义于 Middlewares/MidUsb/MidUsb.c */

/* CDC 类驱动使用的收发缓冲 */
static U8 sRxBuf[64];                       /* USB OUT 单包缓冲（FS 最大 64 字节） */
static U8 sTxPkt[USB_TX_PKT_SIZE];          /* 单次提交给 USB 的发送快照 */

/* 主机 -> MCU：USB 中断写，主循环读 */
static U8            sRxRing[USB_RX_RING_SIZE];
static volatile U16  sRxHead;
static U16           sRxTail;

/* MCU -> 主机：主循环写，UsbTxProcess 读 */
static U8            sTxRing[USB_TX_RING_SIZE];
static volatile U16  sTxHead;
static U16           sTxTail;

/* 串口行编码：主机打开虚拟串口时会下发，仅保存以便回读，不再作用于真实串口 */
USBD_CDC_LineCodingTypeDef LineCoding =
{
  115200, /* baud rate*/
  0x00,   /* stop bits-1*/
  0x00,   /* parity - none*/
  0x08    /* nb. of bits 8*/
};

/* Private function prototypes -----------------------------------------------*/
static int8_t CDC_Itf_Init     (void);
static int8_t CDC_Itf_DeInit   (void);
static int8_t CDC_Itf_Control  (uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Itf_Receive  (uint8_t *pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_CDC_fops =
{
  CDC_Itf_Init,
  CDC_Itf_DeInit,
  CDC_Itf_Control,
  CDC_Itf_Receive
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CDC 媒体层初始化（枚举配置阶段由类驱动回调）
  */
static int8_t CDC_Itf_Init(void)
{
  UsbRxInit();

  /* 挂接类驱动使用的收发缓冲；首个 OUT 接收由 USBD_CDC_Init 内部完成 */
  USBD_CDC_SetRxBuffer(&USBD_Device, sRxBuf);
  USBD_CDC_SetTxBuffer(&USBD_Device, sTxPkt, 0);

  return (USBD_OK);
}

/**
  * @brief  CDC 媒体层去初始化
  */
static int8_t CDC_Itf_DeInit(void)
{
  return (USBD_OK);
}

/**
  * @brief  CDC 类请求处理
  */
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
    break;

  case CDC_SET_COMM_FEATURE:
    break;

  case CDC_GET_COMM_FEATURE:
    break;

  case CDC_CLEAR_COMM_FEATURE:
    break;

  case CDC_SET_LINE_CODING:
    if (length >= 7U)
    {
      LineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |
                                         (pbuf[2] << 16) | (pbuf[3] << 24));
      LineCoding.format     = pbuf[4];
      LineCoding.paritytype = pbuf[5];
      LineCoding.datatype   = pbuf[6];
    }
    break;

  case CDC_GET_LINE_CODING:
    pbuf[0] = (uint8_t)(LineCoding.bitrate);
    pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
    pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
    pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
    pbuf[4] = LineCoding.format;
    pbuf[5] = LineCoding.paritytype;
    pbuf[6] = LineCoding.datatype;
    break;

  case CDC_SET_CONTROL_LINE_STATE:
    /* 可在此根据 DTR/RTS 判断上位机是否打开串口 */
    break;

  case CDC_SEND_BREAK:
    break;

  default:
    break;
  }

  return (USBD_OK);
}

/**
  * @brief  USB OUT 端点收到数据（中断上下文）
  * @note   只做"入环形缓冲 + 重新挂起接收"，不做业务处理
  */
static int8_t CDC_Itf_Receive(uint8_t *Buf, uint32_t *Len)
{
  U16 i;
  U16 len = (U16)((*Len > sizeof(sRxBuf)) ? sizeof(sRxBuf) : *Len);

  for (i = 0U; i < len; i++)
  {
    U16 next = (U16)((sRxHead + 1U) % USB_RX_RING_SIZE);

    if (next == sRxTail)      /* 缓冲满，丢弃新数据 */
    {
      break;
    }
    sRxRing[sRxHead] = Buf[i];
    sRxHead = next;
  }

  /* 重新挂起 OUT 端点，准备接收下一包 */
  USBD_CDC_ReceivePacket(&USBD_Device);

  return (USBD_OK);
}

/**
  * @brief  复位收发环形缓冲
  */
void UsbRxInit(void)
{
  sRxHead = 0U;
  sRxTail = 0U;
  sTxHead = 0U;
  sTxTail = 0U;
}

/**
  * @brief  取走主机下发的数据
  * @retval 实际取到的字节数
  */
U16 UsbRxRead(U8 *pBuf, U16 MaxLen)
{
  U16 cnt = 0U;

  while ((sRxTail != sRxHead) && (cnt < MaxLen))
  {
    pBuf[cnt++] = sRxRing[sRxTail];
    sRxTail = (U16)((sRxTail + 1U) % USB_RX_RING_SIZE);
  }

  return cnt;
}

/**
  * @brief  应答数据入队（非阻塞，可在中断/主循环调用）
  * @retval USBD_OK 成功；USBD_BUSY 队列空间不足
  */
int8_t UsbSendData(U8 *pBuf, U16 Len)
{
  U16 i;

  for (i = 0U; i < Len; i++)
  {
    U16 next = (U16)((sTxHead + 1U) % USB_TX_RING_SIZE);

    if (next == sTxTail)
    {
      return (int8_t)USBD_BUSY;
    }
    sTxRing[sTxHead] = pBuf[i];
    sTxHead = next;
  }

  return (int8_t)USBD_OK;
}

/**
  * @brief  把发送队列里的数据通过 USB 发出（主循环周期调用）
  */
void UsbTxProcess(void)
{
  U16 n = 0U;
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)USBD_Device.pClassData;

  if (hcdc == NULL)               /* 尚未枚举完成 */
  {
    return;
  }

  if (hcdc->TxState != 0U)        /* 上一包仍在发送，等下一轮 */
  {
    return;
  }

  if (sTxTail == sTxHead)
  {
    return;
  }

  while ((sTxTail != sTxHead) && (n < USB_TX_PKT_SIZE))
  {
    sTxPkt[n++] = sTxRing[sTxTail];
    sTxTail = (U16)((sTxTail + 1U) % USB_TX_RING_SIZE);
  }

  USBD_CDC_SetTxBuffer(&USBD_Device, sTxPkt, n);
  (void)USBD_CDC_TransmitPacket(&USBD_Device);
}
