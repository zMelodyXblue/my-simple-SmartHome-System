/*************************************************************************
	> File Name: head.h
	> Author: 
	> Mail: 
	> Created Time: Fri 01 Mar 2024 05:13:14 PM CST
 ************************************************************************/

#ifndef _HEAD_H
#define _HEAD_H

#include "color.h"
#include "datatype.h"
#include "client_recv.h"

#ifdef _D
#define DBG(fmt, args...) printf(fmt, ##args)
#else 
#define DBG(fmt, args...)
#endif

#endif  // _HEAD_H
