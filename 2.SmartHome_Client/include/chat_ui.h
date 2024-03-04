/*************************************************************************
	> File Name: chat_ui.h
	> Author: 
	> Mail: 
	> Created Time: Sun 03 Mar 2024 08:53:29 PM CST
 ************************************************************************/

#ifndef _CHAT_UI_H
#define _CHAT_UI_H
#include <ncurses.h>
#include <locale.h>
#include "datatype.h"

#define MSG_WIDTH 100
#define INFO_WIDTH 15
#define MSG_HEIGHT 25
#define INPUT_HEIGHT 5
void destroy_win(WINDOW *win);
void gotoxy(int x, int y);
void gotoxy_putc(int x, int y, int c);
void gotoxy_puts(int x, int y, char* s);
void w_gotoxy_putc(WINDOW *win, int x, int y, int c);
void w_gotoxy_puts(WINDOW *win, int x, int y, char *s);
void init_ui();
void show_info();
void show_message(WINDOW *win, struct User *user, char *msg, int type);

#define DEFARG(name, default_value) ((#name[0]) ? (name + 0) : default_value)
extern WINDOW *message_sub;
#define Show_Message(arg0, arg1, arg2, arg3) show_message(DEFARG(arg0, message_sub), DEFARG(arg1, NULL), arg2, DEFARG(arg3, 0))

#endif  // _CHAT_UI_H
