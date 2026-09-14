/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usbd_cdc_interface.h
  * @brief   USB CDC 接口层：只负责 USB 收发，数据交给应用层处理
  ******************************************************************************
  * @note    工作模式：USB 直连应用层（非 USB<->串口桥接）
  *          主机下发 -> UsbRxRead() 取走 -> AppProtocol 解析
  *          应答数据 -> UsbSendData() 入队 -> UsbTxProcess() 发送
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CDC_IF_H
#define __USBD_CDC_IF_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"
#include "StdSnbc.h"

/* Exported constants --------------------------------------------------------*/
/* USB 收发环形缓冲大小（FS 单包最大 64 字节，留足余量） */
#define USB_RX_RING_SIZE                512U
#define USB_TX_RING_SIZE                512U
#define USB_TX_PKT_SIZE                 256U   /* 单次提交给 USB 的最大长度 */

/* Exported functions ------------------------------------------------------- */
void   UsbRxInit(void);                             /* 复位收发缓冲 */
U16    UsbRxRead(U8 *pBuf, U16 MaxLen);             /* 取走主机下发数据，返回字节数 */
int8_t UsbSendData(U8 *pBuf, U16 Len);              /* 应答入队（非阻塞） */
void   UsbTxProcess(void);                          /* 主循环周期调用，真正发送 */

extern USBD_CDC_ItfTypeDef  USBD_CDC_fops;

#endif /* __USBD_CDC_IF_H */
