#ifndef __Trace_H
#define __Trace_H
////#include "Trace.h"
//#define X1() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10)//读单个引脚，换个名，,把函数返回值给Xx x1=1(高电平)x1=0（低电平）
//#define X2() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)
//#define X3() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12)
//#define X4() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15)
//#define X5() GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)

#include "stm32f10x.h" // 根据你的单片机型号修改
//#include "buzzer.h"    // 蜂鸣器驱动头文件
//#include "Delay.h"     // 延时函数头文件

//_Bool is_start = 0;     // 起点检测标志
//_Bool is_end = 0;       // 终点检测标志


#include "Motor.h"

// 5路灰度传感器ADC通道定义（根据硬件接线修改）
#define ADC_CH_X1    ADC_Channel_0  // X1对应PA0
#define ADC_CH_X2    ADC_Channel_1  // X2对应PA1
#define ADC_CH_X3    ADC_Channel_2  // X3对应PA2
#define ADC_CH_X4    ADC_Channel_3  // X4对应PA3
#define ADC_CH_X5    ADC_Channel_4  // X5对应PA4

// 灰度阈值（需实际调试，黑线为高电平，阈值区分黑白）
#define GRAY_THRESHOLD  2000  // ADC值>2000判定为黑线，否则为白

// 循迹相关全局变量
extern uint16_t g_GrayValue[5];  // 存储5路灰度ADC值 [X1,X2,X3,X4,X5]
//extern int16_t  g_CurrentAngle;  // 当前角度（适配OLED第四行显示）

// 函数声明
void ADCx_Init(void);            // ADC初始化
uint16_t ADC_GetValue(uint8_t ch); // 读取单通道ADC值
void Gray_Trace_Task(void);      // 循迹核心任务
extern uint8_t Speed;
extern uint8_t adjustSpeed;
extern uint8_t g_Circlenum;
extern uint8_t g_ReturnFlag;
// PID参数
 #define KP  0.2f   // 比例系数（0.1~0.5，越大纠偏越狠）
 #define KI  0.02f  // 积分系数（极小，抑制静态偏差）
 #define KD  0.1f   // 微分系数（0.05~0.2，抑制过冲）
 #define PID_OUT_LIMIT 15 // PID输出限幅，防止纠偏过度
 #define KP_LOCK 0.5f
extern float g_Error;
extern float g_LastError;
extern float g_ErrorSum;
extern float g_MPU6050_Yaw;
extern float g_MPU6050_Pitch;
extern float g_MPU6050_Roll;
extern uint8_t g_IsLockMode;
extern float g_LockAngle;

 void MPU6050_Calc_Angle(void);
  void MPU6050_AngleLock(void);
// 全局标志位：记录起点/终点状态
extern uint8_t is_start ;     // 1=已检测到起点，0=未检测
extern uint8_t is_end ;       // 1=已检测到终点，0=未检测//is_start = 0;     // 起点检测标志
extern uint8_t i;
//is_end = 0;  
// 蜂鸣器响铃函数声明（需和你已实现的蜂鸣器函数一致）
//void Buzzer_Beep(uint16_t ms);
//void Trace_Init(void);
//void Trace_1(void);
//void Trace_2(void);
//void Trace_3(void);
//void Trace_task(void);
void Buzzer(void);
void yaw(void);
#endif
