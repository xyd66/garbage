#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "oled.h"
#include "font.h"
#include "myoled.h"

// 定义OLED设备文件路径
#define FILENAME  "/dev/i2c-3"

// 定义一个display_info结构体类型的静态变量disp，用于存储OLED显示屏的相关信息
static struct display_info disp;

// oled_show函数接收一个void指针类型的参数arg，实则是指向一串字节数据的指针，其中第二个字节（buffer[2]）决定了显示的文本内容
int oled_show(void *arg) 
{
    unsigned char *buffer = (unsigned char *)arg;

    
    
    oled_putstrto(&disp, 0, 9+1, buffer);
  

   
    // 切换到第二种字体
    disp.font = font2;
    // 根据buffer[2]的值显示不同的垃圾分类信息
    switch(buffer[2])
    {
        case 0x41: // 干垃圾
            oled_putstrto(&disp, 0, 20, "dry waste");
            break;
        case 0x42: // 湿垃圾
            oled_putstrto(&disp, 0, 20, "wet waste");
            break;
        case 0x43: // 可回收垃圾
            oled_putstrto(&disp, 0, 20, "recyclable waste");
            break;
        case 0x44: // 有害垃圾
            oled_putstrto(&disp, 0, 20, "hazardous waste");
            break;
        case 0x45: // 识别失败
            oled_putstrto(&disp, 0, 20, "recognition failed");
            break;
    }

    // 确保字体设置正确，尽管之前已经设置过，这里再次设置可能是为了保持清晰或防止意外更改
    disp.font = font2;

    // 将缓存中的内容发送到OLED显示屏进行显示
    oled_send_buffer(&disp);

    return 0; // 函数成功执行返回0
}

// 初始化OLED显示屏
int myoled_init(void)
{
    int e;
    // 设置OLED的I2C地址
    disp.address = OLED_I2C_ADDR;
    // 设置默认字体为第二种字体
    disp.font = font2;

    // 打开与OLED显示屏的I2C连接
    e = oled_open(&disp, FILENAME);
    // 初始化OLED显示屏
    e = oled_init(&disp);

    
    
    // 返回初始化操作的结果状态
    return e;
}