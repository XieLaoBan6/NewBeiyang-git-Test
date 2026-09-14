/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    MidBase64.h
* 功能描述：    Base64编解码中间层头文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-07
* 版本历史信息: 无
*/

#ifndef __MID_BASE64_H__
#define __MID_BASE64_H__

#include "StdSnbc.h"

/**
* @brief		Base64编码
* @param [in]	pSrc     原始数据
* @param [in]	SrcLen   原始数据长度
* @param [out]	pDst     编码后的Base64文本(不以'\0'结尾)
* @param [in]	DstSize  pDst可用空间
* @return	    编码长度(字节); 0:参数错误或空间不足
* @note        编码长度 = ((SrcLen + 2) / 3) * 4
*/
U16 MidBase64Encode(const U8 *pSrc, U16 SrcLen, U8 *pDst, U16 DstSize);

/**
* @brief		Base64解码
* @param [in]	pSrc     Base64文本
* @param [in]	SrcLen   Base64文本长度
* @param [out]	pDst     解码后的原始数据
* @param [in]	DstSize  pDst可用空间
* @return	    解码长度(字节); 0:含非法字符或空间不足
*/
U16 MidBase64Decode(const U8 *pSrc, U16 SrcLen, U8 *pDst, U16 DstSize);

#endif

