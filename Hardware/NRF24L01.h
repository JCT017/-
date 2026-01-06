#ifndef __nRF24L01_API_H
#define __nRF24L01_API_H

#include "stm32f10x.h"                  // Device header

uint8_t SPI_SwapByte(uint8_t byte);
uint8_t NRF24L01_Write_Reg(uint8_t reg,uint8_t value);
uint8_t NRF24L01_Read_Reg(uint8_t reg);
uint8_t NRF24L01_Read_Buf(uint8_t reg,uint8_t *pBuf, uint8_t len);
uint8_t NRF24L01_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len);
uint8_t NRF24L01_GetRxBuf(uint8_t *rxbuf);
uint8_t NRF24L01_SendTxBuf(uint8_t *txbuf);
uint8_t NRF24L01_Check(void);
void NRF24L01_RT_Init(void);
void NRF24L01_Init(void);
void NRF24L01_SendBuf(uint8_t *Buf);
void NRF24L01_Pin_Init(void);
uint8_t NRF24L01_Get_Value_Flag(void);

#endif
////int main(void)
////{
////	
////	   OLED_Init();		//OLED初始化
////	   NRF24L01_Init(); 	//NRF24L01的初始化
////	   Motor_Init();		//左路两个电机
////	        
////	    MotorL_SetSpeed(0);
////        MotorR_SetSpeed(0);
////	uint8_t Buf[32] = {0};	

//// // OLED显示初始化字符串
////    OLED_ShowString(1, 1, "Speed:");
////    OLED_ShowString(2, 1, "Dir:");
////    while (1)
////    {
////		
////        // 1. 接收NRF24L01数据
////        if (NRF24L01_Get_Value_Flag() == 0)
////        {
////            NRF24L01_GetRxBuf(Buf);
////        

////        // 2. 解析速度和方向（抵消50偏移）
////        Speed = (-(Buf[1] - 50))/2;
////        Direction = ((Buf[2] - 50))/2;}

////        // 3. OLED显示
////        OLED_ShowSignedNum(1, 6, Speed, 3);
////        OLED_ShowSignedNum(2, 6, Direction, 3);

////        // 4. 电机控制（过滤微小抖动，速度变化更平缓）
////        if (Speed > 3 || Speed < -3)        ///
////        {
//////            // 差速转向：左电机=速度-方向/2，右电机=速度+方向/2//转圈圈
//////            MotorL_SetSpeed(Speed);				

////			MotorL_SetSpeed(Speed - Direction);
////            MotorR_SetSpeed(Speed + Direction);
////        }
////        else
////        {
////            // 速度过小时停止电机
////            MotorL_SetSpeed(0);
////            MotorR_SetSpeed(0);
////        }
