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
#ifndef __GIMBAL_H__
#define __GIMBAL_H__

#include <rtthread.h>

/* 根据您的硬件配置，PWM设备名称可能不同，通常为 "pwmx" */
#define SERVO_PITCH_PWM_DEVICE   "pwm2"     // 控制垂直方向（俯仰）的舵机
#define SERVO_PITCH_PWM_CHANNEL  1          // 使用 TIM2_CH2 (PA1)

#define SERVO_YAW_PWM_DEVICE     "pwm2"     // 控制水平方向（偏航）的舵机
#define SERVO_YAW_PWM_CHANNEL    4          // 使用 TIM2_CH3 (PA2)


/* 舵机参数 (单位: 纳秒 ns), 与您裸机代码一致 */
#define SERVO_PERIOD_NS      20000000  // 周期 20ms
#define SERVO_MIN_PULSE_NS   500000    // 最小脉宽 0.5ms, 对应 0°
#define SERVO_MAX_PULSE_NS   2500000   // 最大脉宽 2.5ms, 对应 180°

/* 舵机可旋转的角度范围 */
#define SERVO_MIN_ANGLE      0
#define SERVO_MAX_ANGLE      180

/**
 * @brief 初始化云台
 * @return 0: 成功, -1: 失败
 */
int gimbal_init(void);

/**
 * @brief 设置云台的角度
 * @param pitch_angle: 垂直舵机的目标角度 (0-180)
 * @param yaw_angle:   水平舵机的目标角度 (0-180)
 */
void gimbal_set_angle(float pitch_angle, float yaw_angle);

/**
 * @brief 获取当前垂直舵机角度
 * @return 角度值
 */
float gimbal_get_pitch_angle(void);

/**
 * @brief 获取当前水平舵机角度
 * @return 角度值
 */
float gimbal_get_yaw_angle(void);


#endif //__GIMBAL_H__



