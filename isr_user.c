#include <rtthread.h>
#include "main.h"      /* 带 htim1 / hdma_adc1 句柄 */

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;


void TIM1_UP_TIM10_IRQHandler(void)
{
    /* RT-Thread 进入中断 */
    rt_interrupt_enter();

    HAL_TIM_IRQHandler(&htim1);   /* 调用 HAL 内部 */
    //rt_kprintf("TIM1  \n");
    /* RT-Thread 退出中断 */
    rt_interrupt_leave();
}

void DMA2_Stream0_IRQHandler(void)
{
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&hdma_adc1);
    //rt_kprintf("DMA2  \n");
    rt_interrupt_leave();
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */
  rt_kprintf("TIM3  \n");
  /* USER CODE END TIM3_IRQn 1 */
}
