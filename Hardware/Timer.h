// Timer.h
#ifndef TIMER_H
#define TIMER_H

#include "stm32f10x.h"
extern uint32_t g_TimerTick;//全局毫秒计数

void TIM2_Init(void); // 1ms定时器初始化

#endif
