#include <rtthread.h>
#include "speaker_ctrl.h"
#include "stm32f4xx_hal.h"
#include "cubemx/Inc/main.h"
#include "main.h"
/* 由 CubeMX 生成的外设初始化函数（在 Core/Src 或 board.c 中） */
//extern void SystemClock_Config(void);
//extern void MX_GPIO_Init(void);
//extern void MX_DMA_Init(void);
//extern void MX_ADC1_Init(void);
//extern void MX_TIM1_Init(void);
//extern void MX_TIM3_Init(void);

extern TIM_HandleTypeDef htim1;
/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-06-24     Gemini       the first version for camera gimbal system
 */
#include <rtthread.h>
#include <stdlib.h> // For abs() and atof()
#include "gimbal.h"
#include "uart_handler.h"
#include "pid_controller.h"

/* 系统状态机 */
typedef enum {
    STATE_IDLE,
    STATE_TRACKING,
    STATE_SEARCHING,
    STATE_MANUAL
} system_state_t;

/* 将状态变量定义为文件内全局，以便命令函数可以访问 */
static volatile system_state_t current_state = STATE_IDLE;

/* 控制参数 */
#define TARGET_CENTER_X      0.0f
#define TARGET_CENTER_Y      0.0f
#define DEAD_ZONE_X          20
#define DEAD_ZONE_Y          20
#define TARGET_LOSS_TIMEOUT  3000

/* PID 控制器实例 */
static pid_controller_t pid_pitch;
static pid_controller_t pid_yaw;

/* 搜索模式参数 */
#define SEARCH_SWEEP_ANGLE   60.0f
#define SEARCH_SWEEP_SPEED   0.5f

/**
 * @brief 核心控制线程
 */
void control_thread_entry(void* parameter)
{

    if (gimbal_init() != RT_EOK) {
        rt_kprintf("Gimbal init failed!\n");
        return;
    }
    if (uart_handler_init() != RT_EOK) {
        rt_kprintf("UART handler init failed!\n");
        return;
    }

    pid_init(&pid_pitch, 0.008f, -0.001f, -0.01f, -10.0f, 10.0f);
    pid_init(&pid_yaw,   0.008f, -0.001f, -0.01f, -10.0f, 10.0f);

    system_state_t previous_state = STATE_IDLE;
    rt_tick_t last_coord_tick = rt_tick_get();
    int target_x = 0, target_y = 0;
    float search_direction = 1.0f;

    gimbal_set_angle(120.0, 180.0);

    while(1)
    {
        if (current_state != STATE_MANUAL) {
            previous_state = current_state;
            if (uart_handler_get_coordinates(&target_x, &target_y)) {
                current_state =STATE_TRACKING;
                last_coord_tick = rt_tick_get();
            } else if (uart_handler_is_target_lost()) {
                current_state =  STATE_SEARCHING;
            } else if (current_state == STATE_TRACKING && rt_tick_get() - last_coord_tick > rt_tick_from_millisecond(TARGET_LOSS_TIMEOUT)) {
                current_state = STATE_SEARCHING;
            }
        }
        if(current_state != STATE_IDLE){rt_kprintf("STATE:%d \n",current_state);}


        if (current_state != previous_state) {
            switch (current_state) {
                case STATE_IDLE:  break;
                case STATE_TRACKING: pid_reset(&pid_pitch); pid_reset(&pid_yaw); break;
                case STATE_SEARCHING: gimbal_set_angle(90.0f, gimbal_get_yaw_angle()); break;
                case STATE_MANUAL: /* 提示信息已移至命令函数中，这里无需操作 */ break;
            }
        }
        previous_state = current_state;

        switch (current_state) {
            case STATE_IDLE:
            case STATE_MANUAL:
                break;
            case STATE_TRACKING:
                rt_kprintf("Tracking... Coords(x,y): (%d, %d)\n", target_x, target_y);
                if (abs(target_x) < DEAD_ZONE_X && abs(target_y) < DEAD_ZONE_Y) {
                    // In dead zone
                } else {
                    float pitch_output = pid_compute(&pid_pitch, TARGET_CENTER_Y, (float)target_y);
                    float yaw_output   = pid_compute(&pid_yaw,   TARGET_CENTER_X, (float)target_x);
                    float new_pitch = gimbal_get_pitch_angle() + pitch_output;
                    float new_yaw   = gimbal_get_yaw_angle() + yaw_output;
                    rt_kprintf("  -> PID Out(p,y): (%d, %d) | New Angles(p,y): (%d, %d)\n",
                                   (int)pitch_output, (int)yaw_output, (int)new_pitch, (int)new_yaw);
                    gimbal_set_angle(new_pitch, new_yaw);
                }
                break;
            case STATE_SEARCHING:
                {
                    float current_yaw = gimbal_get_yaw_angle();
                    float next_yaw = current_yaw + (SEARCH_SWEEP_SPEED * search_direction);
                    if (next_yaw >= (90.0f + SEARCH_SWEEP_ANGLE) || next_yaw <= (90.0f - SEARCH_SWEEP_ANGLE)) {
                        search_direction *= -1.0f;
                    }
                    gimbal_set_angle(90.0f, next_yaw);
                }
                break;
        }


        rt_thread_mdelay(20);
    }
}

/************************************************************************
 * MSH 自定义命令部分
 ************************************************************************/

static void gimbal_move(int argc, char **argv)
{
    if (argc != 3) {
        rt_kprintf("Usage: gimbal_move <pitch> <yaw>\n");
        rt_kprintf("  pitch: 0.0 - 180.0\n");
        rt_kprintf("  yaw:   0.0 - 180.0\n");
        return;
    }

    float pitch = atof(argv[1]);
    float yaw = atof(argv[2]);

    // 只有在非手动模式时才打印提示信息
    if (current_state != STATE_MANUAL) {
        rt_kprintf("Switching to MANUAL control mode.\n");
    }
    current_state = STATE_MANUAL;

    gimbal_set_angle(pitch, yaw);

    // *** FIX: Print floats as integers to avoid printf issue ***
    rt_kprintf("Gimbal set to -> Pitch: %d, Yaw: %d\n", (int)pitch, (int)yaw);
}
MSH_CMD_EXPORT(gimbal_move, Move gimbal to a specific angle. Usage: gimbal_move <pitch> <yaw>);

static void gimbal_resume(int argc, char **argv)
{
    if (current_state == STATE_MANUAL) {
        current_state = STATE_IDLE;
        rt_kprintf("Gimbal control resumed to automatic mode (IDLE).\n");
    } else {
        rt_kprintf("Gimbal is already in automatic mode.\n");
    }
}

MSH_CMD_EXPORT(gimbal_resume, Resume automatic gimbal control.);

int main(void)
{
    /* HAL & RT-Thread 基础初始化 */
    cubemx_init();

//    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);      /* 主路 PA8 */
//    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);   /* 互补 PA7 */
    /* 应用层初始化：启动 PWM、ADC+DMA、TIM3 触发 */
    speaker_ctrl_start();


    rt_thread_t tid = rt_thread_create("control_thread",
                                       control_thread_entry,
                                       RT_NULL,
                                       2048,
                                       15,
                                       10);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    } else {
        rt_kprintf("Failed to create control_thread\n");
        return -1;
    }
    return 0;
    /* LED 心跳 (板载 PC13) */
//    while (1)
//    {
//        extern uint32_t adc_sample;
//        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//        rt_kprintf("ADC DATA: %u  \n",adc_sample);
//        rt_thread_mdelay(100);
//    }
}
