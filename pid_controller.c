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
#include "pid_controller.h"

void pid_init(pid_controller_t *pid, float Kp, float Ki, float Kd, float min, float max)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->output_min = min;
    pid->output_max = max;
    pid_reset(pid);
}

void pid_reset(pid_controller_t *pid)
{
    pid->target = 0.0f;
    pid->actual = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
}

float pid_compute(pid_controller_t *pid, float target, float actual)
{
    pid->target = target;
    pid->actual = actual;
    pid->error = pid->target - pid->actual;

    // 积分项
    pid->integral += pid->error;
    // 积分限幅
    if (pid->integral > pid->output_max) pid->integral = pid->output_max;
    if (pid->integral < pid->output_min) pid->integral = pid->output_min;


    // PID 计算
    float output = pid->Kp * pid->error +
                   pid->Ki * pid->integral +
                   pid->Kd * (pid->error - pid->last_error);

    // 更新上次误差
    pid->last_error = pid->error;

    // 输出限幅
    if (output > pid->output_max) output = pid->output_max;
    if (output < pid->output_min) output = pid->output_min;

    return output;
}
