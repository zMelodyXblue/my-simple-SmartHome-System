/*************************************************************************
	> File Name: client_recv.c
	> Author: 
	> Mail: 
	> Created Time: Sun 03 Mar 2024 05:22:44 PM CST
 ************************************************************************/

#include <stdio.h>
#include <head.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <chat_ui.h>
extern int global_sockfd; //该变量在client.c中被定义为全局变量
extern char global_user_name[];

#ifndef _D
void *client_recv(void *arg) {
    char recv_buff[2048] = {0};
	struct SmhMsg msg;
    //struct User Username;
    //memset(&Username, 0, sizeof(Username));
	while (1) {
	    memset(&msg, 0, sizeof(msg));
        int rsize = recv(global_sockfd, (char *)&msg, sizeof(msg), 0);
        if (rsize < 0) {
            perror("client_recv:recv");
            exit(1);
        } else if (rsize == 0) {
            DBG("recv SmhMsg null");
            break;
        }

        //待补充: 粘包、拆包 
        
        //struct User0 user0 = {0};
        //strncpy(user0.name, msg.user.name, 19);
        
	    if (msg.type & SMH_HEART) {
            char *temp_msg = "来自服务器的心跳 ������";
            strncpy(recv_buff, temp_msg, MAX_MSG);
	        Show_Message( , &msg.user, recv_buff, 0);
            struct SmhMsg ack_msg;
            ack_msg.type = SMH_ACK;
            if (send(global_sockfd, (char *)&ack_msg, sizeof(ack_msg), 0) < 0) {
                perror("client_recv: send_ack");
            }
	    } else if (msg.type & SMH_MSG) {
            sprintf(recv_buff, "rsize=%d Server Msg: %s\n", rsize, msg.msg);
            //Show_Message( , &msg.user, recv_buff, 0);
	    } else if (msg.type & SMH_WALL) {
            sprintf(recv_buff, "rsize=%d Server Msg To All: %s\n", rsize, msg.msg);
            //Show_Message( , &msg.user, recv_buff, 0);
        } else if (msg.type & SMH_CTL) {
            sprintf(recv_buff, "CTL: %s", msg.msg);
            //Show_Message( , &msg.user, recv_buff, 0);
        } else if (msg.type & SMH_ACK) {
            sprintf(recv_buff, "ACK: %s", msg.msg);
            //Show_Message( , &msg.user, recv_buff, 0);
        } else if (msg.type & SMH_FIN) {
            sprintf(recv_buff, "服务器正要停止\n");
            Show_Message( , &msg.user, recv_buff, 0);
	        close(global_sockfd);
            endwin();
	        exit(0);
	    } else {
            Show_Message( , &msg.user, "Msg Unsupport\n", 0);
            continue;
	    }
        /*sprintf(recv_buff, " %d :msg.type=%d, msg.size=%d\n user: name: %s, type=%d\n  msg: %s\n",
            global_sockfd, msg.type, msg.size, msg.user.name, msg.user.type, msg.msg
        );*/
        Show_Message( , &msg.user, recv_buff, 0);

	}
    return NULL;
}

#else
void *client_recv(void *arg) {
	struct SmhMsg msg;
	while (1) {
	    memset(&msg, 0, sizeof(msg));
        int rsize = recv(global_sockfd, (char *)&msg, sizeof(msg), 0);
        if (rsize < 0) {
            perror("client_recv:recv");
            exit(1);
        } else if (rsize == 0) {
            DBG("recv SmhMsg null");
            break;
        }

        //粘包、拆包
        
	    if (msg.type & SMH_HEART) {
	        DBG("来自服务器的心跳 ������\n");
            struct SmhMsg ack_msg;
            ack_msg.type = SMH_ACK;
            if (send(global_sockfd, (char *)&ack_msg, sizeof(ack_msg), 0) < 0) {
                perror("client_recv: send_ack");
            }
	    } else if (msg.type & SMH_MSG) {
	        DBG("Server Msg : %s\n", msg.msg);
	    } else if (msg.type & SMH_WALL) {
	        DBG("Server Msg To All: %s\n", msg.msg);
	    } else if (msg.type & SMH_FIN) {
	        DBG("服务器正要停止。\n");
	        close(global_sockfd);
	        exit(0);
	    } else {
	        DBG("Msg Unsupport\n");
	    }
        DBG(L_GREEN"recv_SmhMsg: type = %d\n  msg: %s" NONE"\n", msg.type, msg.msg);
	}
    return NULL;
}
#endif
