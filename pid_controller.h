/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-06-24     Gemini       the first version
 */
#ifndef __PID_CONTROLLER_H__
#define __PID_CONTROLLER_H__

#include <rtthread.h>

typedef struct {
    // PID 参数
    float Kp;
    float Ki;
    float Kd;

    // 目标值和当前值
    float target;
    float actual;

    // 误差项
    float error;
    float last_error;
    float integral;

    // 输出限制
    float output_min;
    float output_max;

} pid_controller_t;


/**
 * @brief 初始化PID控制器
 * @param pid: PID控制器实例指针
 * @param Kp, Ki, Kd: PID参数
 * @param min, max: 输出限制
 */
void pid_init(pid_controller_t *pid, float Kp, float Ki, float Kd, float min, float max);

/**
 * @brief 计算PID输出
 * @param pid: PID控制器实例指针
 * @param target: 目标值
 * @param actual: 当前实际值
 * @return PID计算后的输出值
 */
float pid_compute(pid_controller_t *pid, float target, float actual);

/**
 * @brief 重置PID控制器的内部状态 (如积分项)
 * @param pid: PID控制器实例指针
 */
void pid_reset(pid_controller_t *pid);


#endif //__PID_CONTROLLER_H__
