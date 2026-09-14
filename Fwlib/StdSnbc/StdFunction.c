////////////////////////////////////////////////////////////////////
/// Copyright (c)2016, 山东新北洋信息技术股份有限公司
/// All rights reserved.
/// @file     StdFunction.c
/// @brief    SNBC基本函数功能实现
/// @author   wangbotao
/// @version  1.0
/// @date     2016-11-09
////////////////////////////////////////////////////////////////////


#include "StdDataTypeDef.h"
#include "StdMacro.h"

///////////////////////////////////////////////////////////////////
/// @brief		字符串拷贝函数
/// @author		车磊
/// @version	1.000
/// @date		2011-6-16
/// @param [in]	SrcPtr:		U8*型数据，源数据起始地址
///				DstPtr:		U8*型数据，目标数据起始地址
/// @param [out]将SrcPtr指向的内容拷贝至DstPtr中，SrcPtr以0x00作为结束
/// @return		无
/// @attention	SrcPtr以第一个0x00数据作为结束
/// @note		首次发布
/// @see		无
////////////////////////////////////////////////////////////////////
void StringCopy(U8 *SrcPtr, U8 *DstPtr)
{
	U32 Cnt = 0;

	while(SrcPtr[Cnt] != 0)
	{
		DstPtr[Cnt] = SrcPtr[Cnt];
		Cnt++;
	}

	DstPtr[Cnt] = 0;

	return;
}

////////////////////////////////////////////////////////////////////
/// @brief		字符串追加函数
/// @author		车磊
/// @version	1.000
/// @date		2011-6-16
/// @param [in]	SrcPtr:		U8*型数据，源数据起始地址
///				DstPtr:		U8*型数据，目标数据起始地址
/// @param [out]将SrcPtr指向的内容拷贝至DstPtr最后一个有效数据之后，SrcPtr以0x00作为结束
/// @return		无
/// @attention	SrcPtr以第一个0x00数据作为结束
/// @note		首次发布
/// @see		无
////////////////////////////////////////////////////////////////////
void StringCat(U8 *SrcPtr, U8 *DstPtr)
{
	U8 *DetPt = DstPtr;
	U8 *SrcPt = SrcPtr;

	while(*DetPt != 0)		/**查找目标字符串的结束位置*/
	{
		DetPt++;
	}

	while(*SrcPt != 0)		/**项目表字符串中追加源字符串，源字符串以0作为结束*/
	{
		*DetPt++ = *SrcPt++;
	}

	*DetPt = 0;

	return;
}
////////////////////////////////////////////////////////////////////
/// @brief		CharCpy：字符copy
/// @author		xjb
/// @version	1.1
/// @date		2011-07-20
/// @param [in]	StrDest:目标地址
///		        StrSrc:源地址
///				CpyCnt:要复制的字符个数
///				CpyCnt > 0: 按个数复制
///	            CpyCnt == 0: 按字符串格式复制
/// @param [out]无
/// @return		下一个字符位置
/// @attention	无
/// @note		修改NULL为STD_NULL
/// @see		无
////////////////////////////////////////////////////////////////////
void CharCopy(U8 *StrDest,const U8 *StrSrc,U32 CpyCnt)
{
	U32 Cnt = 0;

	if((StrDest == STD_NULL) || (StrSrc == STD_NULL))
	{
		return;
	}

	while((StrSrc[Cnt] != '\0') && (StrSrc[Cnt] != 0xFF))
	{
		StrDest[Cnt] = StrSrc[Cnt];
		Cnt++;
		if(CpyCnt != 0)
		{
			if(Cnt == CpyCnt)
			{
				break;
			}
		}
	}

	return;
}

////////////////////////////////////////////////////////////////////
/// @brief		比较字符串是否相同
/// @author		丁进峰
/// @version	1.000
/// @date		2011-4-16
/// @param [in]
/// @param [out]
/// @return		无
/// @attention	无
/// @note		首次发布
/// @see		无
////////////////////////////////////////////////////////////////////
S32 CompareString(const U8 *String0,const U8 *String1,U32 MaxCnt)
{
	U32 Cnt = 0;
	S32 Flg = 0;

	while((String0[Cnt] != 0) && (String1[Cnt] != 0))
	{
		if(String0[Cnt] != String1[Cnt])
		{
			Flg = -1;
			break;
		}
		Cnt++;
		if(Cnt >= MaxCnt)
		{
			break;
		}
	}

	if(Cnt == 0)
	{
		Flg = -1;
	}

	return(Flg);
}

////////////////////////////////////////////////////////////////////
/// @brief		16进制字节型(8Bit)数据转换为10进制字符串函数
/// @author		车磊
/// @version	1.000
/// @date		2011-6-16
/// @param [in]	Byte:		U8型数据，需要转换的字节数据
///				Buffer:		U8*型数据，转换后字符串的存储缓冲区
/// @param [out]将转换后的数据存储在Buffer中，以0x00作为结束;如Byte=0x64，转换后的数据为0x31,0x30,0x30,0x00
/// @return		无
/// @attention	当需要转换的数据为0时，Buffer中的数据位0x30,0x00;
/// @note		首次发布
/// @see		无
////////////////////////////////////////////////////////////////////
void U8ToString(U8 Byte,U8 *Buffer)
{
	U8 j = 0, k = 0;

	j = Byte/100;		Byte = Byte % 100;		if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Byte/10;		Byte = Byte % 10;		if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Byte;									if(k||j)	{Buffer[k++] = j + 0x30;}

	Buffer[k] = 0;

	if(Buffer[0] == 0)
	{
		Buffer[0] = 0x30;
		Buffer[1] = 0;
	}

	return;
}

////////////////////////////////////////////////////////////////////
/// @brief		16进制字型(16Bit)数据转换为10进制字符串函数
/// @author		车磊
/// @version	1.000
/// @date		2011-6-16
/// @param [in]	Word:		U16型数据，需要转换的16位数据
///				Buffer:		U8*型数据，转换后字符串的存储缓冲区
/// @param [out]将转换后的数据存储在Buffer中，以0x00作为结束;如Word=0x64，转换后的数据为0x31,0x30,0x30,0x00
/// @return		无
/// @attention	当需要转换的数据为0时，Buffer中的数据位0x30,0x00;
/// @note		首次发布
/// @see		无
////////////////////////////////////////////////////////////////////
void U16ToString(U16 Word,U8 *Buffer)
{
	U8 j = 0, k = 0;

	j = Word/10000;		Word = Word % 10000;	if(j)		{Buffer[k++] = j + 0x30;}
	j = Word/1000;		Word = Word % 1000;		if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Word/100;		Word = Word % 100;		if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Word/10;		Word = Word % 10;		if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Word;									if(k||j)	{Buffer[k++] = j + 0x30;}

	Buffer[k] = 0;

	if(Buffer[0] == 0)
	{
		Buffer[0] = 0x30;
		Buffer[1] = 0;
	}

	return;
}
////////////////////////////////////////////////////////////////////
/// @brief		16进制双字(32Bit)型数据转换为10进制字符串函数
/// @author		车磊
/// @version	1.000
/// @date		2011-6-16
/// @param [in]	Dword:		U32型数据，需要转换的32位数据
///				Buffer:		U8*型数据，转换后字符串的存储缓冲区
/// @param [out]将转换后的数据存储在Buffer中，以0x00作为结束;如Dword=0x64，转换后的数据为0x31,0x30,0x30,0x00
/// @return		无
/// @attention	当需要转换的数据为0时，Buffer中的数据位0x30,0x00;
/// @note		首次发布
/// @see		无
////////////////////////////////////////////////////////////////////
void U32ToString(U32 Dword, U8 *Buffer)
{
	U8 j = 0, k = 0;

	j = Dword/1000000000;	Dword = Dword % 1000000000;		if(j)		{Buffer[k++] = j + 0x30;}
	j = Dword/100000000;	Dword = Dword % 100000000;		if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Dword/10000000;		Dword = Dword % 10000000;		if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Dword/1000000;		Dword = Dword % 1000000;		if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Dword/100000;		Dword = Dword % 100000;			if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Dword/10000;		Dword = Dword % 10000;			if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Dword/1000;			Dword = Dword % 1000;			if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Dword/100;			Dword = Dword % 100;			if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Dword/10;			Dword = Dword % 10;				if(k||j)	{Buffer[k++] = j + 0x30;}
	j = Dword;												if(k||j)	{Buffer[k++] = j + 0x30;}

	Buffer[k] = 0;

	if(Buffer[0] == 0)
	{
		Buffer[0] = 0x30;
		Buffer[1] = 0;
	}

	return;
}

////////////////////////////////////////////////////////////////////
/// @brief		16进制字节型(8Bit)数据转换为16进制字符串函数
/// @author		丁进峰
/// @version	1.000
/// @date		2011-6-16
/// @param [in]	Dword:		U32型数据，需要转换的32位数据
///				Buffer:		U8*型数据，转换后字符串的存储缓冲区
/// @param [out]将转换后的数据存储在Buffer中，以0x00作为结束;如Dword=0x64，转换后的数据为0x36,0x34,0x00
/// @return		无
/// @attention	当需要转换的数据为0时，Buffer中的数据位0x30,0x00;
/// @note		首次发布
/// @see		无
////////////////////////////////////////////////////////////////////

void U8ToHexString(U8 Byte,U8 *Buffer)
{
	U8 i = 0;

	i = Byte >> 4;
	if(i < 0x0a)
	{
		Buffer[0]	= i + 0x30;
	}
	else
	{
		Buffer[0]	= i + 0x37;
	}

	i = Byte - (i << 4);
	if(i < 0x0a)
	{
		Buffer[1]	= i + 0x30;
	}
	else
	{
		Buffer[1]	= i + 0x37;
	}

	Buffer[2] = 0;

	return;
}
