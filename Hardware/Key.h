#ifndef __KEY_H
#define __KEY_H
#include "stm32f10x.h"                  // Device header
#include "Delay.h"
typedef enum {
    KEY_NONE = 0,    // 无操作
    KEY1_SHORT = 1,  // 按键1短按
    KEY1_LONG = 2,   // 按键1长按
    KEY2_LONG = 3    // 按键2长按
} Key_State;// 给枚举类型起别名Key_State

// 函数声明：返回值为Key_State类型
Key_State Key_GetState(void); // 按键状态读取（返回短按/长按）
 

void Key_Init(void);

#endif
