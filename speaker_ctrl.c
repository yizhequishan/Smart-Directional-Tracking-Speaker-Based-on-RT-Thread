#include "speaker_ctrl.h"
#include "stm32f4xx_hal.h"

/* --------------------------------------------------------------------------
 * 外设句柄 — 由 CubeMX 在别处定义，这里只需 extern。
 * --------------------------------------------------------------------------*/
extern ADC_HandleTypeDef  hadc1;
extern TIM_HandleTypeDef  htim1;   /* TIM1 – 40 kHz PWM */
extern TIM_HandleTypeDef  htim3;   /* TIM3 – ADC 触发源 */
#define PWM_PERIOD 4199
/* 单元素 DMA 循环缓冲，外部可用 extern 引用 */
//volatile uint32_t adc_sample __attribute__((section(".sram_dma")));
volatile uint32_t adc_sample;
/* 内联函数：根据 ADC 值更新互补 PWM */
/* 每次调用把 raw (0-4095) 按原比例拉伸到 0-(PWM/2-1) */
static inline void update_pwm(uint16_t raw)
{
    const uint16_t mid = PWM_PERIOD / 2;            /* 50 % 占空点 */

    /* scale = (mid-1)/4095  →  用 32-bit 乘，再右移 12 位实现除 4095 */
    uint32_t delta = (uint32_t)raw * (mid - 1);
    delta = delta / 4095U;                          /* 或 (delta >> 12) 近似 */

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, mid + (uint16_t)delta);
}

/* TIM1 Update 中断回调：保持实时调制 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        update_pwm(adc_sample);
        //rt_kprintf("TIM_PeriodE ADC DATA:%x  \n",adc_sample);
    }
}

/* 统一启动函数 —— 在 main() 里调用一次即可 */
void speaker_ctrl_start(void)
{
    /* 互补 PWM */
    HAL_TIM_PWM_Start(&htim1,     TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1,  TIM_CHANNEL_1);
    __HAL_TIM_ENABLE_IT(&htim1,   TIM_IT_UPDATE);

    /* ADC + DMA 循环 */
    HAL_ADC_Start_DMA(&hadc1, &adc_sample, 1);

    /* TIM3 触发 ADC */
    HAL_TIM_Base_Start(&htim3);
    //HAL_TIM_Base_Start_IT(&htim3);
}


void adc_print_once(void)
{
    /* 取当前值 —— 因为 adc_sample 是 volatile，编译器会从 SRAM 重新读取 */
    uint16_t val = adc_sample;

    /* 打印：0xXXXX  (unsigned) */
    rt_kprintf("ADC: 0x%04X  (%u)\r\n",
               (unsigned int)val,       /* 16 进制 */
               (unsigned int)val);      /* 10 进制 */
}
