/*************************************************************************
	> File Name: head.h
	> Author: 
	> Mail: 
	> Created Time: Fri 01 Mar 2024 05:13:14 PM CST
 ************************************************************************/

#ifndef _HEAD_H
#define _HEAD_H
/*
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ncurses.h>
#include <locale.h>
*/
#include "common.h"
#include "chat_ui.h"
#include "color.h"
#include "datatype.h"
#include "client_recv.h"

#ifdef _D
#define DBG(fmt, args...) printf(fmt, ##args)
#else 
#define DBG(fmt, args...)
#endif

#endif  // _HEAD_H
