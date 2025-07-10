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
#include "gimbal.h"



/* 全局变量 */
static struct rt_device_pwm *servo_pitch_dev; // 垂直舵机设备句柄
static struct rt_device_pwm *servo_yaw_dev;   // 水平舵机设备句柄
static float current_pitch_angle = 180.0f;     // 假设初始位置在90度
static float current_yaw_angle = 90.0f;

/**
 * @brief  将角度转换为对应的PWM脉冲宽度 (单位: ns)
 */
static rt_uint32_t angle_to_pulse_ns(float angle)
{
    // 限制角度在有效范围内
    if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

    // 您的裸机代码中的映射关系: Angle / 180 * 2000 + 500 (us)
    // 转换为ns: (Angle / 180 * 2000 + 500) * 1000
    // 这等同于从 500,000 ns (0.5ms) 到 2,500,000 ns (2.5ms) 的线性映射
    return (rt_uint32_t)(SERVO_MIN_PULSE_NS + (angle / SERVO_MAX_ANGLE) * (SERVO_MAX_PULSE_NS - SERVO_MIN_PULSE_NS));
}

int gimbal_init(void)
{
    rt_err_t ret = RT_EOK;

    /* 查找PWM设备 */
    servo_pitch_dev = (struct rt_device_pwm *)rt_device_find(SERVO_PITCH_PWM_DEVICE);
    if (servo_pitch_dev == RT_NULL)
    {

        return -RT_ERROR;
    }

    servo_yaw_dev = (struct rt_device_pwm *)rt_device_find(SERVO_YAW_PWM_DEVICE);
    if (servo_yaw_dev == RT_NULL)
    {
        // 如果俯仰和偏航使用同一个PWM设备，这里不会是NULL

        return -RT_ERROR;
    }

    /* 初始化舵机到一个中间位置 (90度) */
    current_pitch_angle = 120.0f;
    current_yaw_angle = 180.0f;
    gimbal_set_angle(current_pitch_angle, current_yaw_angle);


    /* 使能PWM通道 */
    ret = rt_pwm_enable(servo_pitch_dev, SERVO_PITCH_PWM_CHANNEL);
    if (ret != RT_EOK)
    {

        return ret;
    }

    ret = rt_pwm_enable(servo_yaw_dev, SERVO_YAW_PWM_CHANNEL);
    if (ret != RT_EOK)
    {

        return ret;
    }


    return RT_EOK;
}

void gimbal_set_angle(float pitch_angle, float yaw_angle)
{
    // 限制角度范围
    if (pitch_angle < SERVO_MIN_ANGLE) pitch_angle = SERVO_MIN_ANGLE;
    if (pitch_angle > SERVO_MAX_ANGLE) pitch_angle = SERVO_MAX_ANGLE;
    if (yaw_angle < SERVO_MIN_ANGLE) yaw_angle = SERVO_MIN_ANGLE;
    if (yaw_angle > SERVO_MAX_ANGLE) yaw_angle = SERVO_MAX_ANGLE;

    rt_uint32_t pitch_pulse = angle_to_pulse_ns(pitch_angle);
    rt_uint32_t yaw_pulse   = angle_to_pulse_ns(yaw_angle);

    rt_pwm_set(servo_pitch_dev, SERVO_PITCH_PWM_CHANNEL, SERVO_PERIOD_NS, pitch_pulse);
    rt_pwm_set(servo_yaw_dev, SERVO_YAW_PWM_CHANNEL, SERVO_PERIOD_NS, yaw_pulse);

    // 更新当前角度
    current_pitch_angle = pitch_angle;
    current_yaw_angle = yaw_angle;

    rt_kprintf("NEW ANGLE :%d %d\n",(int)current_pitch_angle,(int)current_pitch_angle);
}

float gimbal_get_pitch_angle(void)
{
    // 直接返回静态变量中记录的当前角度值
    return current_pitch_angle;
}

float gimbal_get_yaw_angle(void)
{
    // 直接返回静态变量中记录的当前角度值
    return current_yaw_angle;
}
