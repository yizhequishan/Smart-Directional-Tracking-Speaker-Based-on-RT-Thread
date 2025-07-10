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
#ifndef __UART_HANDLER_H__
#define __UART_HANDLER_H__

#include <rtthread.h>

// 根据您的硬件连接，选择摄像头数据输入的串口
#define CAM_UART_DEVICE_NAME "uart3"

/**
 * @brief 初始化串口处理器
 * @return 0: 成功, -1: 失败
 */
int uart_handler_init(void);

/**
 * @brief 获取解析后的坐标数据
 * @param x: 用于存储x坐标的指针
 * @param y: 用于存储y坐标的指针
 * @return RT_TRUE: 获取到新数据, RT_FALSE: 没有新数据
 */
rt_bool_t uart_handler_get_coordinates(int *x, int *y);

/**
 * @brief 检查目标是否丢失 (通过"Sweeping"指令判断)
 * @return RT_TRUE: 目标丢失, RT_FALSE: 目标未丢失
 */
rt_bool_t uart_handler_is_target_lost(void);


#endif //__UART_HANDLER_H__
