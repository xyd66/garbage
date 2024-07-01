#include "socket.h"



int socket_init(const char *ipaddr, const char *ipport)
{
    int s_fd = -1;

    struct sockaddr_in s_addr;  // 服务器地址结构
   
    memset(&s_addr, 0, sizeof(struct sockaddr_in)); // 清零s_addr

    printf("%s|%s|%d: \n",__FILE__,__func__,__LINE__);
        // 1. 创建套接字
    s_fd = socket(AF_INET, SOCK_STREAM, 0);
    printf("%s|%s|%d: s_fd=%d\n",__FILE__,__func__,__LINE__,s_fd);
    if(s_fd == -1){
        perror("socket creation failed");
        return -1;
    }

    // 配置服务器地址结构体
    s_addr.sin_family = AF_INET; // 使用IPv4
    s_addr.sin_port = htons(atoi(ipport)); // 设置端口号，htons用于网络字节序转换
    inet_aton(ipaddr, &s_addr.sin_addr); // 将字符串形式的IP地址转换为网络字节序的二进制形式
    printf("%s|%s|%d: \n",__FILE__,__func__,__LINE__);
    // 2. 绑定地址到套接字
    if(bind(s_fd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr_in)) == -1){
        perror("bind failed");
        return -1;
    }
    printf("%s|%s|%d: \n",__FILE__,__func__,__LINE__);
    // 3. 监听连接请求
    if(listen(s_fd, 1) == -1){ // 最多挂起1个连接请求
        perror("listen failed");
        return -1;
    }
    printf("%s|%s|%d: \n",__FILE__,__func__,__LINE__);
    return s_fd;
}