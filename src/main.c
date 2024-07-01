#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <wiringPi.h>
#include <pthread.h>

#include "uart.h"
#include "garbage.h"
#include "pwm.h"
#include "myoled.h"
#include "socket.h"

 
int serial_fd = -1;
 pthread_cond_t cond;
 pthread_mutex_t mutex; 

static int detect_process(const char * process_name)//检查进程是否在运行
{  
        int n = -1;     
        FILE *strm;  
        char buf[128]={0};  
        sprintf(buf,"ps -ax | grep  %s|grep -v grep", process_name);  
        
        if((strm = popen(buf, "r")) != NULL)  
        {  
            if(fgets(buf, sizeof(buf), strm) != NULL)  
            {
                printf("buf=%s\n",buf);
                n = atoi(buf); 
                printf("n=%d\n",n);
            }
        }
        else
        {
            return -1;
        }       
        
        pclose(strm);   
        return n;  
} 
   
  


void *pget_void(void *arg) //接收语音模块的数据
{
    unsigned char buffer[6] = {0xAA,0x55, 0x00,0x00,0x55,0xAA};
    int len =0;

    printf("%s|%s|%d \n", __FILE__, __func__, __LINE__);
    if (-1 == serial_fd)
    {
        printf("%s|%s|%d : open serial failed\n", __FILE__, __func__, __LINE__);
        pthread_exit(0);
    }

    printf("%s|%s|%d \n", __FILE__, __func__, __LINE__);
    while(1)
    {
        len = serialGetstring(serial_fd,buffer);
         printf("%s|%s|%d \n", __FILE__, __func__, __LINE__);
        if(len > 0 && buffer[2] == 0x46)
        {
            pthread_mutex_lock(&mutex);//加锁
            buffer[2] = 0x00;
            pthread_cond_signal(&cond);//触发
            pthread_mutex_unlock(&mutex);//解锁

        }
    }
        pthread_exit(0);
}

void *psend_voice(void *arg) //向语音模块发送数据
{
    pthread_detach(pthread_self());
    unsigned char *buffer = (unsigned char *)arg;

    if (-1 == serial_fd)
    {
        printf("%s|%s|%d : open serial failed\n", __FILE__, __func__, __LINE__);
        pthread_exit(0);
    }

    
    if(NULL != buffer)
    {
         serialSendstring(serial_fd,buffer,6);
    }
       
    pthread_exit(0);
}

void *popen_trash_can(void *arg)//垃圾桶开盖并判断垃圾类别
{
    
    pthread_detach(pthread_self());
    unsigned char *buffer = (unsigned char*)arg;

    if(buffer[2] == 0x43)
    {
    	printf("%s|%s|%d: buffer[2]=0x%x \n", __FILE__, __func__, __LINE__,buffer[2]);
        pwm_write(PWM_RECOVERABLE_GARBAGE);
        delay(2000);
        pwm_stop(PWM_RECOVERABLE_GARBAGE);
    }
    else if (buffer[2] != 0x45)
    {
    	printf("%s|%s|%d: buffer[2]=0x%x \n", __FILE__, __func__, __LINE__,buffer[2]);
        printf("start\n");
        pwm_write(PWM_GARBAGE);
        delay(2000);
        pwm_stop(PWM_GARBAGE);
    }
  
    pthread_exit(0);
}

 void *poled_show(void *arg) //oled 显示数据
 {
    pthread_detach(pthread_self());//线程脱离
    myoled_init();
    oled_show(arg);

    pthread_exit(0); 
}

void *pcategory(void *arg)
{
    unsigned char buffer[6] = {0xAA,0x55, 0x00,0x00,0x55,0xAA};
    char *category = NULL;

    pthread_t send_voice_tid, trash_tid,oled_tid;
    printf("%s|%s|%d \n", __FILE__, __func__, __LINE__);

    while(1)
    {
        printf("%s|%s|%d \n", __FILE__, __func__, __LINE__);
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond,&mutex);
        pthread_mutex_unlock(&mutex); 
		
        printf("%s|%s|%d \n", __FILE__, __func__, __LINE__);

		buffer[2] = 0x00;
        system(WGET_CMD);//拍照

        if(0 == access(GARBAGE_FILE,F_OK))
        {
            category = garbage_category(category);
            if(strstr(category,"干垃圾"))
            {
                buffer[2] = 0x41;
            }
            else if (strstr(category,"湿垃圾"))
            {
                buffer[2] = 0x42;
            }
            else if (strstr(category,"可回收垃圾"))
            {
                buffer[2] = 0x43;
            }
            else if (strstr(category,"有害垃圾"))
            {
                 buffer[2] = 0x44;
            }
            else
            {
                buffer[2] = 0x45;
            }

            }
            else
            {
                buffer[2] = 0x45;
            }

            //开垃圾桶开关
            pthread_create(&trash_tid,NULL,psend_voice ,(void*)buffer);
           
            //开语音播报线程
            pthread_create(&send_voice_tid,NULL,popen_trash_can,(void*)buffer);

            //oled显示线程 
            pthread_create(&oled_tid, NULL, poled_show, (void *)buffer);
            
           // buffer[2]= 0x00;
            remove(GARBAGE_FILE);

    
	}

	pthread_exit(0);

}


void *pget_socket(void *arg)
{
    int s_fd = -1;
    int c_fd = -1;
    char buffer[6];
    int nread = -1;
    struct sockaddr_in c_addr; // 客户端地址结构 
    memset(&c_addr,0,sizeof(struct sockaddr_in));   

    s_fd = socket_init(IPADDR,IPPORT);//获取IP地址和端口号
    printf("%s|%s|%d: s_fd=%d\n",__FILE__,__func__,__LINE__,s_fd);
    if (-1 == s_fd)
    {
        
        pthread_exit(0);
    }
    sleep(3);
    int clen = sizeof(struct sockaddr_in);

    while(1){

        c_fd = accept(s_fd,(struct sockaddr *)&c_addr,&clen); 

        int keepalive = 1;        // 开启TCP KeepAlive功能   
        int keepidle = 5;        // tcp_keepalive_time 3s内没收到数据开始发送心跳包
        int keepcnt = 3;        // tcp_keepalive_probes 每次发送心跳包的时间间隔,单位秒   
        int keepintvl = 3;      // tcp_keepalive_intvl 每3s发送一次心跳包 
        
        
       
        setsockopt( c_fd , SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));

        setsockopt( c_fd , SOL_TCP, TCP_KEEPIDLE, (void *) &keepidle, sizeof(keepidle));

        setsockopt( c_fd , SOL_TCP, TCP_KEEPCNT, (void *)&keepcnt, sizeof(keepcnt));

        setsockopt( c_fd , SOL_TCP, TCP_KEEPINTVL, (void *)&keepintvl, sizeof(keepintvl));

        printf("%s|%s|%d: Accept a connection from %s:%d\n",__FILE__,__func__,__LINE__,inet_ntoa(c_addr.sin_addr),ntohs(c_addr.sin_port));
        
        if(c_fd ==-1){  
            perror("accept");  
            continue; // 如果接受失败，继续等待下一个连接  
        }  

        while(1)
        {
            memset(buffer,0,sizeof(buffer));

            //等价于n_read = read(c_fd,readBuf,sizeof(buffer)); 
            nread = recv(c_fd,buffer,sizeof(buffer),0);//读取客户端数据

            printf("%s|%s|%d: nread=%d,buffer=%s\n",__FILE__,__func__,__LINE__,nread,buffer);

            if(nread > 0)
            {
                if(strstr(buffer,"open"))
                {
                    pthread_mutex_lock(&mutex);//加锁
                    
                    pthread_cond_signal(&cond);//触发
                    pthread_mutex_unlock(&mutex);//解锁
                }
            }
            else if(0 == nread || -1 == nread)//读取成功或者失败退出循环
            {
                break;
            }
        }
        close(c_fd);


    }
    pthread_exit(0);
}

int main(int argc,char *argv[])
{
   
    int len =0;
    int ret = -1;
    int oa;

    char *category = NULL;
    pthread_t get_voice_tid , category_tid,get_socket_tid; 

    wiringPiSetup();//初始化wiringpi库

    garbage_init();//初始化阿里云接口

    ret = detect_process("mjpg_streamer");//检测服务是否开启
      
    if ( -1 == ret)
    {
        printf("detect process failed\n");
        goto END;
    }
    
    serial_fd = myserialOpen(SERIAL_DEV,BAUD);//打开串口设置波特率

    if (-1 == serial_fd)
    {
        printf("open serial failed\n");
        goto END;
    }
    
    //开语音线程
   
    pthread_create(&get_voice_tid,NULL,pget_void ,NULL);
    printf("%s|%s|%d\n", __FILE__, __func__, __LINE__);

    //开网络线程
    printf("%s|%s|%d\n", __FILE__, __func__, __LINE__);
    oa=pthread_create(&get_socket_tid,NULL,pget_socket ,NULL);
    if(oa!=0)
    {
        printf("fialed 66\n");
    }


    //开阿里云交互线程
    printf("%s|%s|%d\n", __FILE__, __func__, __LINE__);
    pthread_create(&category_tid,NULL,pcategory,NULL);

    pthread_join(get_voice_tid,NULL);
    pthread_join(get_socket_tid,NULL);
    pthread_join(category_tid,NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    close(serial_fd);
END:
    garbage_final();
    return 0;
}
