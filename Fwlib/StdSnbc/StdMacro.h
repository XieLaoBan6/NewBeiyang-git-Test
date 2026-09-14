////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016,山东新北洋信息技术股份有限公司
/// All rights reserved.
///
/// 文件名称：    StdMacro.h
/// 功能描述：    SNBC标准基本宏定义
/// 当前版本号:   1.1
/// 作者/修改者:  wangbotao
/// 完成日期:     2016-11-09
/// 版本历史信息: V1.1宏定义前增加STD_防止重复定义
////////////////////////////////////////////////////////////////////

#ifndef __STD_MACRO_H__
#define __STD_MACRO_H__

#include "StdDataTypeDef.h"

#ifndef STD_NULL
#define STD_NULL				((void *)0)
#endif

#ifndef STD_OPEN
#define STD_OPEN				1
#endif

#ifndef STD_CLOSE
#define STD_CLOSE				0
#endif

#ifndef STD_ENABLE
#define STD_ENABLE				1
#endif

#ifndef STD_DISABLE
#define STD_DISABLE				0
#endif

#ifndef STD_TRUE
#define STD_TRUE				1
#endif

#ifndef STD_FALSE
#define STD_FALSE				0
#endif

#ifndef STD_XON
#define STD_XON					0x11
#endif

#ifndef STD_XOFF
#define STD_XOFF				0x13
#endif

//BIT0~BIT31的宏定义
#ifndef STD_BIT0
#define STD_BIT0				0x00000001u
#define STD_BIT1				0x00000002u
#define STD_BIT2				0x00000004u
#define STD_BIT3				0x00000008u
#define STD_BIT4				0x00000010u
#define STD_BIT5				0x00000020u
#define STD_BIT6				0x00000040u
#define STD_BIT7				0x00000080u
#define STD_BIT8				0x00000100u
#define STD_BIT9				0x00000200u
#define STD_BIT10				0x00000400u
#define STD_BIT11				0x00000800u
#define STD_BIT12				0x00001000u
#define STD_BIT13				0x00002000u
#define STD_BIT14				0x00004000u
#define STD_BIT15				0x00008000u
#define STD_BIT16				0x00010000u
#define STD_BIT17				0x00020000u
#define STD_BIT18				0x00040000u
#define STD_BIT19				0x00080000u
#define STD_BIT20				0x00100000u
#define STD_BIT21				0x00200000u
#define STD_BIT22				0x00400000u
#define STD_BIT23				0x00800000u
#define STD_BIT24				0x01000000u
#define STD_BIT25				0x02000000u
#define STD_BIT26				0x04000000u
#define STD_BIT27				0x08000000u
#define STD_BIT28				0x10000000u
#define STD_BIT29				0x20000000u
#define STD_BIT30				0x40000000u
#define STD_BIT31				0x80000000u
#endif

//求二者比较值
#define STD_MAXVAR(x,y) (((x)>=(y)) ? (x) : (y))
#define STD_MINVAR(x,y) (((x)>=(y)) ? (y) : (x))

//将v与mx,mn比较,超界则赋临界值
#define STD_MAX_MIN_LIMIT(v,mx,mn) (STD_MAXVAR(mn, STD_MINVAR(v,mx)))

//计算x y的绝对值
#define STD_ABS(x,y)(STD_MAXVAR(x,y)-STD_MINVAR(x,y))

//结构体拷贝
#define STD_STRUCT_COPY(Dst,Src,Struct) {U16 Cnt,i;Cnt = sizeof(Struct);for(i=0;i<Cnt;i++) *((U8 *)Dst + i) = *((U8 *)Src + i);}

//地址对齐宏定义
#define ALIGN_U16(Address)	(((U32)(Address) + 1) & 0xFFFFFFFE)
#define ALIGN_U32(Address)	(((U32)(Address) + 3) & 0xFFFFFFFC)

#define ALIGN_U16_CUT(Address)	(((U32)(Address)) & 0xFFFFFFFE)
#define ALIGN_U32_CUT(Address)	(((U32)(Address)) & 0xFFFFFFFC)

//数据合并宏定义
#define MAKE_U8_TO_U16(Low,High)	(((U8)(Low))| (((U8)(High)) << 8))
#define MAKE_U16_TO_U32(Low,High)	(((U16)(Low)) | (((U16)(High)) << 16))

//给函数指针赋值
#define SET_FUNC_POINTER(Fuc, Value) {*(U32 **)&(Fuc) = (U32 *)Value;}


#endif
