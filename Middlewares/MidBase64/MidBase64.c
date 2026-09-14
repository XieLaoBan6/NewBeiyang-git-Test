/*
* Copyright (c) 2016,山东新北洋信息技术股份有限公司
* All rights reserved.
* 
* 文件名称：    MidBase64.c
* 功能描述：    Base64编解码中间层源文件
* 当前版本号:   V1.0
* 作者/修改者:  XLB
* 完成日期:     2026-09-07
* 版本历史信息: 无
*/

#include "MidBase64.h"

static const U8 sBase64EncTab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
* @brief		Base64字符转6位值
* @param [in]	c       Base64字符
* @param [out]	pVal    6位值
* @return	    1:有效  0:非法字符
*/
static U8 MidBase64CharToVal(U8 c, U8 *pVal)
{
    U8 Ret = 1U;

    if ((c >= (U8)'A') && (c <= (U8)'Z'))
    {
        *pVal = (U8)(c - (U8)'A');
    }
    else if ((c >= (U8)'a') && (c <= (U8)'z'))
    {
        *pVal = (U8)(c - (U8)'a' + 26U);
    }
    else if ((c >= (U8)'0') && (c <= (U8)'9'))
    {
        *pVal = (U8)(c - (U8)'0' + 52U);
    }
    else if (c == (U8)'+')
    {
        *pVal = 62U;
    }
    else if (c == (U8)'/')
    {
        *pVal = 63U;
    }
    else
    {
        Ret = 0U;
    }

    return (Ret);
}

/**
* @brief		Base64编码
* @param [in]	pSrc     原始数据
* @param [in]	SrcLen   原始数据长度
* @param [out]	pDst     编码后的Base64文本
* @param [in]	DstSize  pDst可用空间
* @return	    编码长度; 0:失败
*/
U16 MidBase64Encode(const U8 *pSrc, U16 SrcLen, U8 *pDst, U16 DstSize)
{
    U16 i;
    U16 OutLen = 0U;
    U16 NeedLen;
    U32 Acc;
    U8  b0;
    U8  b1;
    U8  b2;
    U16 Remain;

    if ((pSrc == NULL) || (pDst == NULL))
    {
        return (0U);
    }

    NeedLen = (U16)(((SrcLen + 2U) / 3U) * 4U);
    if (DstSize < NeedLen)
    {
        return (0U);
    }

    for (i = 0U; i < SrcLen; i = (U16)(i + 3U))
    {
        Remain = (U16)(SrcLen - i);

        b0 = pSrc[i];
        b1 = ((Remain > 1U) ? pSrc[i + 1U] : 0U);
        b2 = ((Remain > 2U) ? pSrc[i + 2U] : 0U);

        Acc = (((U32)b0 << 16) | ((U32)b1 << 8) | (U32)b2);

        pDst[OutLen++] = sBase64EncTab[(Acc >> 18) & 0x3FU];
        pDst[OutLen++] = sBase64EncTab[(Acc >> 12) & 0x3FU];
        pDst[OutLen++] = ((Remain > 1U) ? sBase64EncTab[(Acc >> 6) & 0x3FU] : (U8)'=');
        pDst[OutLen++] = ((Remain > 2U) ? sBase64EncTab[Acc & 0x3FU] : (U8)'=');
    }

    return (OutLen);
}

/**
* @brief		Base64解码
* @param [in]	pSrc     Base64文本
* @param [in]	SrcLen   Base64文本长度
* @param [out]	pDst     解码后的原始数据
* @param [in]	DstSize  pDst可用空间
* @return	    解码长度; 0:失败
*/
U16 MidBase64Decode(const U8 *pSrc, U16 SrcLen, U8 *pDst, U16 DstSize)
{
    U16 i;
    U16 OutLen = 0U;
    U32 Acc = 0U;
    U16 Bits = 0U;
    U8  Val;

    if ((pSrc == NULL) || (pDst == NULL))
    {
        return (0U);
    }

    for (i = 0U; i < SrcLen; i++)
    {
        if (pSrc[i] == (U8)'=')
        {
            break;                                  /* 填充符, 结束 */
        }

        if ((pSrc[i] == (U8)'\r') || (pSrc[i] == (U8)'\n'))
        {
            continue;                               /* 跳过换行 */
        }

        Val = 0U;
        if (MidBase64CharToVal(pSrc[i], &Val) == 0U)
        {
            return (0U);                            /* 非法字符 */
        }

        Acc  = ((Acc << 6) | (U32)Val);
        Bits = (U16)(Bits + 6U);

        if (Bits >= 8U)
        {
            Bits = (U16)(Bits - 8U);
            if (OutLen >= DstSize)
            {
                return (0U);
            }
            pDst[OutLen++] = (U8)((Acc >> Bits) & 0xFFU);
        }
    }

    return (OutLen);
}
