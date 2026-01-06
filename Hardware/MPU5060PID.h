#ifndef __MPU5060PID_H
#define __MPU5060PID_H



extern int16_t L_EncPulse;
extern int16_t R_EncPulse;
void Encoder_Init(void);
void Encoder_Read_pluse(void);
int8_t Encoder_Get_Speed(void);
#endif
