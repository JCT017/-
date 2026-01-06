// Timer.c
#include "Timer.h"
#include "Key.h"
#include "OLED_Display.h"
#include "Delay.h"

// 按键计时变量
 uint32_t g_TimerTick=0;

// 定时器2初始化（1ms中断）
void TIM2_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_InternalClockConfig(TIM2);//
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_TimeBaseStruct.TIM_Period = 999;         // 自动重装值
    TIM_TimeBaseStruct.TIM_Prescaler = 71;       // 预分频器（72MHz→1kHz，即1ms）
    TIM_TimeBaseStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);//时基单元
    
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);   // 开启更新中断 中断输出控制   时钟源选择默认内部
    
    // 中断优先级配置
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    TIM_Cmd(TIM2, ENABLE); // 启动定时器  运行控制
}

// 定时器2中断服务函数（核心逻辑：按键检测+计时）
// TIM2中断服务函数（只做一件事：累加毫秒数）
void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
        g_TimerTick++; // 仅累加毫秒数
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
