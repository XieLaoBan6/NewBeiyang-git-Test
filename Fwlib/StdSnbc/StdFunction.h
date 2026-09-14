////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016,山东新北洋信息技术股份有限公司
/// All rights reserved.
///
/// 文件名称：    StdFunction.h
/// 功能描述：    SNBC基本函数功能头文件
/// 当前版本号:   1.0
/// 作者/修改者:  wangbotao
/// 完成日期:     2016-11-09
/// 版本历史信息: 无
////////////////////////////////////////////////////////////////////

#ifndef __STD_FUNCTION_H__
#define __STD_FUNCTION_H__

#include "StdDataTypeDef.h"

extern void StringCopy(U8 *SrcPtr, U8 *DstPtr);
extern void StringCat(U8 *SrcPtr, U8 *DstPtr);
extern void CharCopy(U8 *StrDest,const U8 *StrSrc,U32 CpyCnt);
extern S32  CompareString(const U8 *String0,const U8 *String1,U32 MaxCnt);

extern void U8ToString(U8 Byte,U8 *Buffer);
extern void U16ToString(U16 Word,U8 *Buffer);
extern void U32ToString(U32 Dword, U8 *Buffer);
extern void U8ToHexString(U8 Byte,U8 *Buffer);

#endif
