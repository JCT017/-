#include "stm32f10x.h"                  // Device header
#include "MPU5060PID.h"
#include "Delay.h"
#include "math.h"
// 全局变量：左右轮编码器脉冲增量
int16_t g_LeftEncPulse = 0;
int16_t g_RightEncPulse = 0;

/**
 * @brief  编码器初始化（左轮TIM4(PB3/PB4)，右轮TIM1(PA10/PA11)）
 
 */
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    // 1. 使能时钟：GPIOA/GPIOB + TIM1/TIM4 + 复用功能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    // 2. 左编码器（PB3/TIM4_CH2、PB4/TIM4_CH1）上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 3. 右编码器（PA10/TIM1_CH3、PA11/TIM1_CH4）上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 4. TIM4初始化（左轮）：编码器模式，不分频+最大计数
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_Period = 65535;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

    // 5. 左轮编码器模式配置（TI1+TI2，4倍频+滤波）
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 0x0F;
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    // 6. TIM1初始化（右轮）：高级定时器，编码器模式
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_Period = 65535;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    // 7. 右轮编码器模式配置（TI3+TI4，4倍频+滤波）
    TIM_EncoderInterfaceConfig(TIM1, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 0x0F;
    // TIM1_CH3配置
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
    TIM_ICInit(TIM1, &TIM_ICInitStructure);
    // TIM1_CH4配置
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
    TIM_ICInit(TIM1, &TIM_ICInitStructure);

    // 8. 使能定时器（TIM1需额外使能主输出）
    TIM_Cmd(TIM4, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE); // TIM1高级定时器必须开启主输出
}

/**
 * @brief  读取编码器脉冲增量（读取后清零）
 */
void Encoder_Read_Pulse(void)
{
    g_LeftEncPulse = (int16_t)TIM_GetCounter(TIM4);  // 左轮TIM4计数
    g_RightEncPulse = (int16_t)TIM_GetCounter(TIM1); // 右轮TIM1计数
    TIM_SetCounter(TIM4, 0);
    TIM_SetCounter(TIM1, 0);
}

/**
 * @brief  编码电机测速（返回0~100相对转速）
 */
int8_t Encoder_Get_Speed(void)
{
    Encoder_Read_Pulse();
    int8_t leftSpeed = g_LeftEncPulse / 5;  // 缩放系数按电机微调
    int8_t rightSpeed = g_RightEncPulse / 5;

    leftSpeed = leftSpeed > 100 ? 100 : (leftSpeed < 0 ? 0 : leftSpeed);
    rightSpeed = rightSpeed > 100 ? 100 : (rightSpeed < 0 ? 0 : rightSpeed);

    return (leftSpeed + rightSpeed) / 2;
}





