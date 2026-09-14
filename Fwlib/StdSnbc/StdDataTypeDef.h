////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016,山东新北洋信息技术股份有限公司
/// All rights reserved.
///
/// 文件名称：    StdDataTypeDef.h
/// 功能描述：    SNBC标准数据类型定义
/// 当前版本号:   1.1
/// 作者/修改者:  baopengxu
/// 完成日期:     2016-11-09
/// 版本历史信息: V1.1 增加U64\S64\F32\F64支持;
////////////////////////////////////////////////////////////////////

#ifndef __STD_DATATYPE_DEF_H__
#define __STD_DATATYPE_DEF_H__

#ifndef        BOOL_DEFINE
    typedef        unsigned char    	BOOL;
    #define        BOOL_DEFINE
#endif

#ifndef        CHAR_DEFINE
    typedef        char                 CHAR;
    #define        CHAR_DEFINE
#endif

#ifndef        U8_DEFINE
    typedef        unsigned char    	U8;
    #define        U8_DEFINE
#endif

#ifndef        S8_DEFINE
    typedef        signed char      	S8;
    #define        S8_DEFINE
#endif

#ifndef        U16_DEFINE
    typedef        unsigned short   	U16;
    #define        U16_DEFINE
#endif

#ifndef        S16_DEFINE
    typedef        signed short     	S16;
    #define        S16_DEFINE
#endif

#ifndef        U32_DEFINE
    typedef        unsigned int    		U32;
    #define        U32_DEFINE
#endif

#ifndef        S32_DEFINE
    typedef        signed int       	S32;
    #define        S32_DEFINE
#endif

#ifndef        U64_DEFINE
    typedef        unsigned long long 	U64;
    #define        U64_DEFINE
#endif

#ifndef        S64_DEFINE
    typedef        signed long long    	S64;
    #define        S64_DEFINE
#endif

#ifndef        F32_DEFINE
    typedef        float                F32;
    #define        F32_DEFINE
#endif

#ifndef        F64_DEFINE
    typedef        double               F64;
    #define        F64_DEFINE
#endif

#endif
