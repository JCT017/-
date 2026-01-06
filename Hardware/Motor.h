#ifndef __MOTOR_H
#define __MOTOR_H

void Motor_Init(void);
//void Motor_SetSpeed(int Left_speed ,int Right_speed);
void MotorL_SetSpeed(int8_t Speed);			//左路两个电机
void MotorR_SetSpeed(int8_t Speed);			//右路两个电机

#endif
