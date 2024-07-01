#ifndef __SOCKET_H
#define __SOCKET_H

#include <stdio.h>      // 标准输入输出库
#include <sys/types.h>  // 数据类型定义，如pid_t
#include <sys/socket.h> // 套接字编程相关函数声明
#include <stdlib.h>     // 通用函数库，如exit()
#include <arpa/inet.h>  // 网络地址转换等函数声明
#include <netinet/in.h> // INET 地址族定义
#include <string.h>     // 字符串操作函数库
#include <unistd.h>     // 与进程控制相关的函数声明
#include <errno.h>
#include <netinet/tcp.h>

#define IPADDR "192.168.1.105"
#define IPPORT "8192"//一定不要是8080，摄像头服务已经用了

int socket_init(const char *ipaddr, const char *ipport);

#endif