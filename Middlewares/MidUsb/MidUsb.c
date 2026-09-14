/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    MidUsb.c
* 功能描述：    Usb中间层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-04
* 版本历史信息: 无
*/

#include "MidUsb.h"

USBD_HandleTypeDef USBD_Device;

/**
* @brief		USB初始化
* @author		XLB
* @param [in]	无
* @param [out]	无
* @return	    无
*/
void USBInit(void)
{

  USBD_Init(&USBD_Device, &VCP_Desc, 0);

  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

  /* Add CDC Interface Class */
  USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

  /* Start Device Process */
  USBD_Start(&USBD_Device);

}

