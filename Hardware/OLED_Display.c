#include "stm32f10x.h"// Device header
#include "OLED.h"
#include "OLED_Display.h"
#include "MyRTC.h"
#include "Delay.h"
#include "timer.h"
// 全局变量定义
uint8_t g_CurrentMode = 1;    // 默认模式1 0为rtc
uint8_t g_IsRunning = 0;      // 默认停止
uint32_t g_TotalRunTime = 0;  // 总时间初始0
uint32_t g_ModeRunTime = 0;   // 模式时间初始0
//int16_t g_CurrentAngle = 0;   // 角度初始0°
uint32_t g_ModeStartTick = 0; // 当前模式启动时的TIM2毫秒数
uint8_t g_RtcShowInit = 0; // 标记RTC是否已显示
uint8_t g_IsRtcMode = 1; // 1:开机RTC模式
void OLED_ShowRTC_Startup(void){

   OLED_Clear(); // 清屏
//     

 // 第1行：开机提示
OLED_ShowString(1, 1, "Startup RTC");
    // 第2行：显示时间（时:分:秒）
OLED_ShowString(2, 1, "Time: ");

		MyRTC_ReadTime(); // 读取RTC时间
        OLED_ShowNum(2, 7, MyRTC_Time[0], 2);  // 显示小时（2位）
        OLED_ShowChar(2, 9, ':');              // 显示冒号
        OLED_ShowNum(2, 10, MyRTC_Time[1], 2); // 显示分钟（2位）
        OLED_ShowChar(2, 12, ':');
        OLED_ShowNum(2, 13, MyRTC_Time[2], 2); // 显示秒（2位）
	    OLED_ShowNum(3, 6, RTC_GetCounter(), 10);	//显示32位的秒计数器
		OLED_ShowNum(4, 6, RTC_GetDivider(), 10);	//显示余数寄存器
//	   
//	static uint32_t lastOledUpdateTick = 0;
//   
//		if (g_TimerTick - lastOledUpdateTick >= 1000) { // 时间差≥1000ms（1秒）
//  
//		lastOledUpdateTick = g_TimerTick; // 更新上次刷新的时间

//		OLED_ShowRTC_Startup(); // 刷新OLED
//    }

	}
  
// 更新OLED显示（严格对应考核的4行）
void OLED_UpdateDisplay(void) {
    OLED_Clear(); // 清屏
    
    // 第1行：当前运行模式
    OLED_ShowString(1, 1, "Mode: ");
    OLED_ShowNum(1, 7, g_CurrentMode, 1); // 显示模式号
    OLED_ShowString(1, 9, g_IsRunning ? "(start)" : "(Stop)"); // 运行状态
    
    // 第2行：总运行时间（转成秒）
    OLED_ShowString(2, 1, "TotalTime: ");
    OLED_ShowNum(2, 11, g_TotalRunTime , 4); // 毫秒转秒
    OLED_ShowString(2, 15, "s");
    
    // 第3行：当前模式运行时间
    OLED_ShowString(3, 1, "ModeTime: ");
    OLED_ShowNum(3, 11, g_ModeRunTime , 4);
    OLED_ShowString(3, 15, "s");
    
    // 第4行：当前角度
    OLED_ShowString(4, 1, "Angle: ");
    OLED_ShowNum(4, 7, g_CurrentAngle, 4);
    OLED_ShowString(4, 11, "deg");
	
	
}
