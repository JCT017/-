#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Key.h"
//#include "OLED_Display.h"
#include "Timer.h" // 包含g_TimerTick
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

Key_State  Key_GetState(void){     // 按键状态读取（返回短按/长按）

    static uint32_t key1_down_tick = 0;  // 按键1按下时的毫秒数
    static uint8_t key1_is_pressed = 0;  // 按键1是否处于按下状态
    static uint32_t key2_down_tick = 0;  // 按键2按下时的毫秒数
    static uint8_t key2_is_pressed = 0;  // 按键2是否处于按下状态
    Key_State state = KEY_NONE;


    // -------- 按键1检测 --------
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0) { // 按键按下
        Delay_ms(20); // 消抖
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0 && key1_is_pressed == 0) {
            key1_down_tick = g_TimerTick; // 记录按下时刻的毫秒数
            key1_is_pressed = 1;          // 标记按键已按下
        }
    } else { // 按键松开
        if (key1_is_pressed == 1) {
            uint32_t press_duration = g_TimerTick - key1_down_tick; // 计算按压时长
            if (press_duration >= 3000) {
                state = KEY1_LONG;  // 长按（≥3秒）
            } else if (press_duration >= 50) {
                state = KEY1_SHORT; // 短按（50ms~3秒）
            }
            key1_is_pressed = 0; // 重置状态
        }
  }
	

    // -------- 按键2检测（同理）--------
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == 0) { // 按键按下
        Delay_ms(20); // 消抖
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == 0 && key2_is_pressed == 0) {
            key2_down_tick = g_TimerTick;
            key2_is_pressed = 1;
        }
    } else { // 按键松开
        if (key2_is_pressed == 1) {
            uint32_t press_duration = g_TimerTick - key2_down_tick;
            if (press_duration >= 3000) {
                state = KEY2_LONG; // 按键2长按
            }
            key2_is_pressed = 0;
        }
    }


    return state;
}


