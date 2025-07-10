/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-06-24     yizhe       the first version
 */
/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-06-24     Gemini       the first version
 */
#include <rtdevice.h>
#include "uart_handler.h"
#include <string.h>
#include <stdlib.h>



/* 数据缓冲区大小 */
#define UART_RX_BUFFER_SIZE 64

/* 全局变量 */
static rt_device_t serial_cam;
static struct rt_semaphore rx_sem;

// 串口接收缓冲区
static char rx_buffer[UART_RX_BUFFER_SIZE];
static int rx_len = 0;

// 解析后的坐标数据
static volatile int target_x = 0;
static volatile int target_y = 0;
static volatile rt_bool_t new_data_flag = RT_FALSE;
static volatile rt_bool_t target_lost_flag = RT_FALSE;

/**
 * @brief 接收数据回调函数
 */
static rt_err_t uart_input_callback(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&rx_sem);
    return RT_EOK;
}


/**
 * @brief 解析接收到的数据帧
 * 仿照您的 chuanko.c 和数据示例进行解析
 */
static void parse_uart_data(char *data, int len)
{
    rt_kprintf("parse_uart_data");
    // 检查是否为 "Sweeping" 指令
    if (strstr(data, "Sweeping") != NULL)
    {
        target_lost_flag = RT_TRUE;

        return;
    }

    // 解析 "x,y" 格式的坐标
    char *comma_ptr = strchr(data, ',');
    if (comma_ptr != NULL)
    {
        *comma_ptr = '\0'; // 将逗号替换为字符串结束符
        int x_val = atoi(data);
        int y_val = atoi(comma_ptr + 1);
        rt_kprintf("\n parse_NEW_data \n");
        // 更新坐标并设置新数据标志
        //rt_enter_critical();
        target_x = x_val;
        target_y = y_val;
        new_data_flag = RT_TRUE;
        target_lost_flag = RT_FALSE; // 收到坐标，说明目标没丢
        //rt_exit_critical();


    }
}


/**
 * @brief 串口数据处理线程
 */
// file: uart_handler.c (推荐的修改)
static void uart_thread_entry(void *parameter)
{
    char ch;
    while (1)
    {
        // 等待回调函数发出的“有数据到达”的信号
//        if (rt_sem_take(&rx_sem, RT_WAITING_FOREVER) != RT_EOK)
//        {
//            continue;
//        }
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);

        //rt_kprintf("UART thread woke up!\n");

        // 循环读取串口缓冲区中的所有字符
        while (rt_device_read(serial_cam, 0, &ch, 1) == 1)
        {
            //rt_kprintf("raw char: 0x%02X\n", ch);
            if (ch == '\n' || ch == '\r')
            {   rt_kprintf("good char: 0x%02X\n", ch);
                if (rx_len > 0)
                {
                    rx_buffer[rx_len] = '\0';
                    parse_uart_data(rx_buffer, rx_len);
                    rx_len = 0;
                }
            }
            else if (rx_len < (UART_RX_BUFFER_SIZE - 1))
            {
                rx_buffer[rx_len++] = ch;
            }
            else
            {
                rx_len = 0; // 缓冲区溢出，清空
            }
        }
    }
}

int uart_handler_init(void)
{
    rt_err_t ret = RT_EOK;

    serial_cam = rt_device_find(CAM_UART_DEVICE_NAME);
    if (!serial_cam)
    {
        rt_kprintf("Failed to find %s device!\n", CAM_UART_DEVICE_NAME);
        return RT_ERROR;
    }

    rt_kprintf("%s device found.\n", CAM_UART_DEVICE_NAME);

    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);

    // 以中断接收模式打开设备
    ret = rt_device_open(serial_cam, RT_DEVICE_FLAG_INT_RX);
    if (ret < 0)
    {
        rt_kprintf("Failed to open %s device with INT_RX flag! Error code: %d\n", CAM_UART_DEVICE_NAME, ret);
        return ret;
    }

    rt_kprintf("%s device opened successfully.\n", CAM_UART_DEVICE_NAME);
    // 设置接收回调
    rt_device_set_rx_indicate(serial_cam, uart_input_callback);

    // 创建处理线程
    rt_thread_t thread = rt_thread_create("uart_thread",
                                          uart_thread_entry,
                                          RT_NULL,
                                          1024,
                                          25,
                                          10);

    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {

        return RT_ERROR;
    }


    return ret;
}


rt_bool_t uart_handler_get_coordinates(int *x, int *y)
{
    if (new_data_flag)
    {
        //rt_enter_critical();
        rt_kprintf("newdata");
        *x = target_x;
        *y = target_y;
        new_data_flag = RT_FALSE; // 标志被清除，表示数据已被取走
        //rt_exit_critical();
        return RT_TRUE;
    }
    return RT_FALSE;
}

rt_bool_t uart_handler_is_target_lost(void)
{
    if (target_lost_flag) {
        target_lost_flag = RT_FALSE; // 清除标志
        return RT_TRUE;
    }
    return RT_FALSE;
}
