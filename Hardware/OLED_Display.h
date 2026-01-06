// OLED_Display.h（声明显示函数和全局变量）
#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "stm32f10x.h"
#include "OLED.h"
#include "Trace.h"


// 全局变量（后续定时器会修改这些值）
extern uint8_t g_CurrentMode;    // 当前模式（1/2/3）
extern uint8_t g_IsRunning;      // 是否运行（0=停止，1=运行）
extern uint32_t g_TotalRunTime;  // 总运行时间（ms）
extern uint32_t g_ModeRunTime;   // 当前模式运行时间（ms）
extern int16_t g_CurrentAngle;   // 当前角度（初始0°）
extern uint32_t g_ModeStartTick; // 当前模式启动时的TIM2毫秒数
extern uint8_t g_RtcShowInit ; // 标记RTC是否已显示
extern uint8_t g_IsRtcMode ; // 1:开机RTC模式
//void OLED_Init(void);     // OLED初始化+初始显示
void OLED_ShowRTC_Startup(void);
void OLED_UpdateDisplay(void);   // 更新OLED显示（考核4行）

#endif
