#include "stm32f10x.h"                  // Device header
#include "PWM.h"

/**
  * 函    数：直流电机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11| GPIO_Pin_1|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//将引脚初始化为推挽输出	
	
	PWM_Init();													//初始化直流电机的底层PWM
}

/**
  * 函    数：直流电机设置速度
  * 参    数：Speed 要设置的速度，范围：-100~100
  * 返 回 值：无
  */


void MotorL_SetSpeed(int8_t Speed)			//左路两个电机
{
	if (Speed > 0)							//如果设置正转的速度值
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);	//PA4置低电平
		GPIO_SetBits(GPIOB, GPIO_Pin_11);	//PA5置高电平，设置方向为反转
		PWM_SetCompare3(Speed);				//PWM设置为速度值
	}
	else if(Speed<0)									//否则，即设置反转的速度值
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_1);	//PA4置高电平
		GPIO_ResetBits(GPIOB, GPIO_Pin_11);	//PA5置低电平，设置方向为正转
//		GPIO_ResetBits(GPIOB, GPIO_Pin_1);	//PA4置低电平
//		GPIO_SetBits(GPIOB, GPIO_Pin_11);	//PA5置高电平，设置方向为反转
		PWM_SetCompare3(-Speed);			//PWM设置为负的速度值，因为此时速度值为负数，而PWM只能给正数
	}
	else
	{
		TIM_SetCompare3(TIM3,0);            //PA7
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
		GPIO_SetBits(GPIOB, GPIO_Pin_11);
	}
}

void MotorR_SetSpeed(int8_t Speed)			//右路两个电机
{
	if (Speed > 0)							//如果设置正转的速度值
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_7);	//PA4置低电平
		GPIO_SetBits(GPIOB, GPIO_Pin_6);	//PA5置高电平，设置方向为反转
		PWM_SetCompare2(Speed);				//PWM设置为速度值
	}
	else if(Speed<0)										//否则，即设置反转的速度值
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_7);	//PA4置高电平
		GPIO_ResetBits(GPIOB, GPIO_Pin_6);	//PA5置低电平，设置方向为正转
//		GPIO_ResetBits(GPIOB, GPIO_Pin_7);	//PA4置低电平
//		GPIO_SetBits(GPIOB, GPIO_Pin_6);	//PA5置高电平，设置方向为反转
		PWM_SetCompare2(-Speed);			//PWM设置为负的速度值，因为此时速度值为负数，而PWM只能给正数
	}
	else
	{
		TIM_SetCompare3(TIM2,0);            //PA7
		GPIO_SetBits(GPIOB, GPIO_Pin_6);
		GPIO_SetBits(GPIOB, GPIO_Pin_7);
	}
}

//void Motor_SetSpeed(int Left_speed ,int Right_speed)
//{
//	//右边侧电机
//	if(Right_speed>0)						//如果设置正转的速度值
//	{
//    	GPIO_SetBits(GPIOB, GPIO_Pin_6);	//PA4置高电平
//		GPIO_ResetBits(GPIOB, GPIO_Pin_7);	//PA5置低电平，设置方向为正转
//		PWM_SetCompare2(Right_speed);				//PWM设置为速度值
//	}
//	else									//否则，即设置反转的速度值
//	{

//		GPIO_SetBits(GPIOB, GPIO_Pin_6);	//PA4置高电平
//		GPIO_ResetBits(GPIOB, GPIO_Pin_7);	//PA5置低电平，设置方向为正转
//		PWM_SetCompare2(-Right_speed);				//PWM设置为速度值
//	}
//	
//	//左侧电机
//	if(Left_speed>0)
//	{
//		PWM_SetCompare3(Left_speed);
//		GPIO_SetBits(GPIOB, GPIO_Pin_11);
//		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
//	}
//	else if(Left_speed<0)
//	{
//		PWM_SetCompare3(-Left_speed);
//		GPIO_ResetBits(GPIOB, GPIO_Pin_11);
//		GPIO_SetBits(GPIOB, GPIO_Pin_1);
//	}
////	else
////	{
////		TIM_SetCompare2(TIM3,0);            //PA7
////		GPIO_SetBits(GPIOB, GPIO_Pin_7);
////		GPIO_SetBits(GPIOB, GPIO_Pin_8);
////	}
//	
//}
