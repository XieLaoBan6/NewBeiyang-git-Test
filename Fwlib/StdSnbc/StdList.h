////////////////////////////////////////////////////////////////////
/// Copyright (c) 2016,山东新北洋信息技术股份有限公司
/// All rights reserved.
/// 
/// 文件名称：    StdList.h
/// 功能描述：    链表操作
/// 当前版本号:   1.0
/// 作者/修改者:  dingjinfeng
/// 完成日期:     2016-11-09
/// 版本历史信息: 无 
/// 备注:         1.该链表操作借用RTT源码中的算法(kservice.h)。没有直接应用RTT源码的原因：该算法比较通用,应该作为我们自己的资源,减少对操作系统的依赖
///				2.该文件里面的函数均为内联函数
////////////////////////////////////////////////////////////////////

#ifndef __STD_LIST_H__
#define __STD_LIST_H__

#include "StdDataTypeDef.h"	

////////////////////////////////////////////////////////////////////
/// @brief		坐标属性
/// @note			用于记录坐标设置	
/// @attention	无
////////////////////////////////////////////////////////////////////
struct STD_LIST_NODE
{
	/**
	* @brief		上一个指针
	* @note			无
	* @attention	无
	*/
	struct STD_LIST_NODE *next;

	/**
	* @brief		下一个指针
	* @note			无
	* @attention	无
	*/
	struct STD_LIST_NODE *prev;
};

////////////////////////////////////////////////////////////////////
/// @brief		链表初始化
/// @author		丁进峰
/// @version		1.000
/// @date			2012-4-18
/// @param [in]	
/// @param [out]	
/// @return		无
/// @attention	无
/// @note			该函数为内联函数，无实体
/// @see			无
////////////////////////////////////////////////////////////////////
__inline static void ListInit(struct STD_LIST_NODE *l)
{
	l->next = l->prev = l;
}

////////////////////////////////////////////////////////////////////
/// @brief		在当前节点之后插入对象
/// @author		丁进峰
/// @version		1.000
/// @date			2012-4-18
/// @param [in]	
/// @param [out]	
/// @return		无
/// @attention	无
/// @note			该函数为内联函数，无实体
/// @see			无
////////////////////////////////////////////////////////////////////
__inline static void ListInsertAfter(struct STD_LIST_NODE *l, struct STD_LIST_NODE *n)
{
	l->next->prev = n;
	n->next = l->next;

	l->next = n;
	n->prev = l;
}

////////////////////////////////////////////////////////////////////
/// @brief		在当前节点之后插入对象
/// @author		丁进峰
/// @version		1.000
/// @date			2012-4-18
/// @param [in]	
/// @param [out]	
/// @return		无
/// @attention	无
/// @note			该函数为内联函数，无实体
/// @see			无
////////////////////////////////////////////////////////////////////
__inline static void ListInsertBefore(struct STD_LIST_NODE *l, struct STD_LIST_NODE *n)
{
	l->prev->next = n;
	n->prev = l->prev;

	l->prev = n;
	n->next = l;
}

////////////////////////////////////////////////////////////////////
/// @brief		删除对象
/// @author		丁进峰
/// @version		1.000
/// @date			2012-4-18
/// @param [in]	
/// @param [out]	
/// @return		无
/// @attention	无
/// @note			该函数为内联函数，无实体
/// @see			无
////////////////////////////////////////////////////////////////////
__inline static void ListRemove(struct STD_LIST_NODE *n)
{
	n->next->prev = n->prev;
	n->prev->next = n->next;

	n->next = n->prev = n;
}

////////////////////////////////////////////////////////////////////
/// @brief		判断是否到达链表尾
/// @author		丁进峰
/// @version		1.000
/// @date			2012-4-18
/// @param [in]	
/// @param [out]	
/// @return		无
/// @attention	无
/// @note			该函数为内联函数，无实体
/// @see			无
////////////////////////////////////////////////////////////////////
__inline static U32 ListIsEmpty(const struct STD_LIST_NODE *l)
{
	return (l->next == l);
}

#define STD_LIST_ENTRY(node, type, member) \
    ((type *)((U8 *)(node) - (U32)(&((type *)0)->member)))

#endif

