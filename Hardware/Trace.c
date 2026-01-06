#include "stm32f10x.h"
#include "Delay.h"
#include "Trace.h"
#include "Motor.h"
//#include "mycontrol.h"
#include "OLED_Display.h"
#include "Timer.h"
#include "MPU5060PID.h"
#include "math.h"
#include "stdlib.h"
uint8_t is_end=0;
uint8_t is_start=0;
	// 全局变量
uint16_t g_GrayValue[5] = {0};
int16_t  g_CurrentAngle = 0;     // 角度初始化为0（起点角度）
uint8_t Speed=25;	//使用各个外设前必须开启时钟，否则对外设的操作无效extern float g_Error;
 float g_LastError;
 float g_ErrorSum;
float g_Error;

   void Buzzer(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIO的时钟

	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;					//定义结构体变量
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//GPIO模式，赋值为推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				//GPIO引脚，赋值为第12号引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//GPIO速度，赋值为50MHz
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将赋值后的构体变量传递给GPIO_Init函数
															//函数内部会自动根据结构体的参数配置相应寄存器
															//实现GPIOB的初始化
	   GPIO_ResetBits(GPIOB, GPIO_Pin_5);		//将PB12引脚设置为低电平，蜂鸣器鸣叫
		Delay_ms(300);							//延时100ms
		GPIO_SetBits(GPIOB, GPIO_Pin_5);		//将PB12引脚设置为高电平，蜂鸣器停止

}
void yaw(void){


}




/**
 * @brief  ADC初始化（PA0-PA4，ADC1，单次转换模式）
 */
void ADCx_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef  ADC_InitStructure;

    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);  // ADC时钟分频6（12MHz）

    // GPIO配置（PA0-PA4 模拟输入）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 模拟输入模式
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ADC配置
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  // 独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;       // 单次扫描（单通道）
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // 单次转换
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // 右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1; // 单次转换1个通道
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE); // 使能ADC1

    // ADC校准
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

/**
 * @brief  读取指定通道ADC值（软件触发）
 * @param  ch: ADC通道（ADC_Channel_0 ~ ADC_Channel_4）
 * @retval 12位ADC转换值
 */
uint16_t ADC_GetValue(uint8_t ch)
{
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_55Cycles5); // 配置通道
    ADC_SoftwareStartConvCmd(ADC1, ENABLE); // 软件触发转换
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)); // 等待转换完成
    return ADC_GetConversionValue(ADC1); // 返回转换值
}

/**
 * @brief  5路灰度循迹核心任务（需在main的while(1)中调用）
 * @note   黑线为高电平，根据5路灰度值控制电机转向
 */
/**
  * @brief  灰度偏差计算（PID输入，加权求和精准纠偏）
  * @retval 偏差值：>0偏左，<0偏右，=0居中
  */
 float Gray_Calc_Error(void)
 {
     float error = 0;
     int weight[5] = {20, 10, 0, -10, -20}; // 0最左，4最右权重
     for(uint8_t i=0; i<5; i++)
     {
         if(g_GrayValue[i] > GRAY_THRESHOLD)
         {
             error += weight[i];
         }
     }
     return error;
 }
 /**
  * @brief  PID核心计算（比例+积分+微分，精准纠偏）
  */
 /**
 * @brief  PID核心计算（比例+积分+微分，修复语法错误）
 */
float PID_Calc(float error)
{
    // 1. 比例环节（确保KP是浮点常量，非指针）
    float pOut = KP * error;

    // 2. 积分环节（修复括号匹配，限幅防止饱和）
    g_ErrorSum += error;
    if (g_ErrorSum > (100.0f / KI))  // 补全右括号，加.0f确保浮点运算
    {
        g_ErrorSum = 100.0f / KI;
    }
    if (g_ErrorSum < (-100.0f / KI)) // 补全右括号
    {
        g_ErrorSum = -100.0f / KI;
    }
    float iOut = KI * g_ErrorSum;

    // 3. 微分环节（修复乘法语法）
    float dOut = KD * (error - g_LastError);
    g_LastError = error;

    // 4. PID总输出
    float pidOut = pOut + iOut + dOut;
    // 限幅：
    if (pidOut > PID_OUT_LIMIT)
    {
        pidOut = PID_OUT_LIMIT;
    }
    else if (pidOut < -PID_OUT_LIMIT)
    {
        pidOut = -PID_OUT_LIMIT;
    }

    return pidOut;
}
 /**
  * @brief  起点/终点检测（保留你的逻辑，新增陀螺仪角度校准）
  */
 void Start_End_Check(void)
 {
     static uint32_t start_tick = 0, end_tick = 0;
     // 起点检测：中路黑线，持续200ms防误判
     if (g_GrayValue[2] > GRAY_THRESHOLD && is_start == 0 && g_IsRunning == 1)
     {
         if (g_TimerTick - start_tick > 200)
         {
             Buzzer();
             is_start = 1;
             is_end = 0;
             g_MPU6050_Yaw = 0; // 起点重置陀螺仪角度为0°
             start_tick = g_TimerTick;
         }
     }
     else start_tick = g_TimerTick;
     // 终点检测：左右两路黑线，持续200ms防误判
     if (g_GrayValue[0] > GRAY_THRESHOLD && g_GrayValue[4] > GRAY_THRESHOLD && is_end == 0 && g_IsRunning == 1)
     {
         if (g_TimerTick - end_tick > 200)
         {
             Buzzer();
             is_end = 1;
             is_start = 0;
             end_tick = g_TimerTick;
         }
     }
     else end_tick = g_TimerTick;
 }
 /**
  * @brief  灰度循迹核心任务（整合PID+编码器+陀螺仪，保留你的直角转向）
  */
 void Gray_Trace_Task(void)
 {
     // 1. 读取5路灰度值
     g_GrayValue[4] = ADC_GetValue(ADC_CH_X1);
     g_GrayValue[3] = ADC_GetValue(ADC_CH_X2);
     g_GrayValue[2] = ADC_GetValue(ADC_CH_X3);
     g_GrayValue[1] = ADC_GetValue(ADC_CH_X4);
     g_GrayValue[0] = ADC_GetValue(ADC_CH_X5);
     // 2. 起点/终点检测
     Start_End_Check();
     // 3. 停止状态直接返回
     if (g_IsRunning == 0)
     {
         MotorL_SetSpeed(0);
         MotorR_SetSpeed(0);
         return;
     }
     uint8_t baseSpeed = Speed;
     float pidOut = 0;
     int8_t encSpeed = Encoder_Get_Speed(); // 编码电机测速稳速
     g_Error = Gray_Calc_Error();           // 计算循迹偏差
     pidOut = PID_Calc(g_Error);            // PID计算修正量
    // MPU6050_Calc_Angle();                  // 更新陀螺仪角度
     // 4. 模式三返程逻辑（保留你的原有逻辑）
     if (g_ReturnFlag == 1)
     {
         if (g_GrayValue[2] > GRAY_THRESHOLD)
         {
             MotorL_SetSpeed(baseSpeed - pidOut);
             MotorR_SetSpeed(baseSpeed + pidOut);
             g_CurrentAngle = 0;
         }
         else if (g_GrayValue[1] > GRAY_THRESHOLD)
         {
             MotorL_SetSpeed(baseSpeed + 15 - pidOut);
             MotorR_SetSpeed(baseSpeed - 15 + pidOut);
             //g_CurrentAngle = 10;
         }
         else if (g_GrayValue[0] > GRAY_THRESHOLD)
         {
             MotorL_SetSpeed(baseSpeed + 30 - pidOut);
             MotorR_SetSpeed(baseSpeed - 30 + pidOut);
             //g_CurrentAngle = 20;
         }
         else if (g_GrayValue[3] > GRAY_THRESHOLD)
         {
             MotorL_SetSpeed(baseSpeed - 15 - pidOut);
             MotorR_SetSpeed(baseSpeed + 15 + pidOut);
             //g_CurrentAngle = -10;
         }
         else if (g_GrayValue[4] > GRAY_THRESHOLD)
         {
             MotorL_SetSpeed(baseSpeed - 30 - pidOut);
             MotorR_SetSpeed(baseSpeed + 30 + pidOut);
             //g_CurrentAngle = -20;
         }
         else
         {
             MotorL_SetSpeed(0);
             MotorR_SetSpeed(0);
            // g_CurrentAngle = 999;
         }
     }
     else
     {
		 if (g_GrayValue[1] < GRAY_THRESHOLD&&g_GrayValue[0] < GRAY_THRESHOLD&&g_GrayValue[2] < GRAY_THRESHOLD&&g_GrayValue[3] < GRAY_THRESHOLD&&g_GrayValue[4] < GRAY_THRESHOLD){
		 MotorL_SetSpeed(0);
		 MotorR_SetSpeed(0);
		 
		 
		 }
         // 5. 左直角转向（原有逻辑+陀螺仪角度校准）
        else  if((g_GrayValue[0] > GRAY_THRESHOLD&&g_GrayValue[1] > GRAY_THRESHOLD))
         {
             uint16_t left_timeout = 0;
             do{
                 g_GrayValue[2] = ADC_GetValue(ADC_CH_X3);
                // MPU6050_Calc_Angle();
                 MotorL_SetSpeed(0);
                 MotorR_SetSpeed(baseSpeed);
                 left_timeout++;
                 Delay_ms(1);
             }while(g_GrayValue[2] < GRAY_THRESHOLD && fabs(g_MPU6050_Yaw) < 88 && left_timeout < 300);
             g_CurrentAngle = (int16_t)g_MPU6050_Yaw;
         }
         // 6. 右直角转向（原有逻辑+陀螺仪角度校准）
         else if((g_GrayValue[3] > GRAY_THRESHOLD&&g_GrayValue[4] > GRAY_THRESHOLD))
         {
             uint16_t right_timeout = 0;
             do{
                 g_GrayValue[2] = ADC_GetValue(ADC_CH_X3);
                 //MPU6050_Calc_Angle();
                 MotorL_SetSpeed(baseSpeed);
                 MotorR_SetSpeed(0);
                 right_timeout++;
                 Delay_ms(1);
             }while(g_GrayValue[2] < GRAY_THRESHOLD &&fabs(g_MPU6050_Yaw) < 88 && right_timeout < 300);
             g_CurrentAngle = (int16_t)g_MPU6050_Yaw;
         }
         
		 // 7. 常规PID循迹（编码器稳速+PID纠偏）
         else
         {
			 
             int8_t leftSpeed = baseSpeed + pidOut;
             int8_t rightSpeed = baseSpeed - pidOut;
             // 编码器稳速校准
             if(encSpeed > leftSpeed) leftSpeed += 2;
             if(encSpeed < leftSpeed) leftSpeed -= 2;
             if(encSpeed > rightSpeed) rightSpeed += 2;
             if(encSpeed < rightSpeed) rightSpeed -= 2;
             // 转速限幅
             leftSpeed = leftSpeed > 100 ? 100 : (leftSpeed < 0 ? 0 : leftSpeed);
             rightSpeed = rightSpeed > 100 ? 100 : (rightSpeed < 0 ? 0 : rightSpeed);
             // 执行电机速度
             MotorL_SetSpeed(leftSpeed);
             MotorR_SetSpeed(rightSpeed);
             g_CurrentAngle = (int16_t)g_MPU6050_Yaw;
         }
     }
 }

//////void Gray_Trace_Task(void)
//////{
////////	uint8_t is_start ;     // 1=已检测到起点，0=未检测
//////// 
////////	uint8_t is_end ;  
//////	
//////    // 1. 读取5路灰度ADC值
//////    g_GrayValue[4] = ADC_GetValue(ADC_CH_X1); // X1
//////    g_GrayValue[3] = ADC_GetValue(ADC_CH_X2); // X2
//////    g_GrayValue[2] = ADC_GetValue(ADC_CH_X3); // X3（中间路）
//////    g_GrayValue[1] = ADC_GetValue(ADC_CH_X4); // X4
//////    g_GrayValue[0] = ADC_GetValue(ADC_CH_X5); // X5

//////////	
//////////	 // 2. 起点检测：X3单路黑线（持续200ms防误判）
//////////    static uint32_t start_tick = 0;
//////////    if (g_GrayValue[2] > GRAY_THRESHOLD&&g_IsRunning==1)//&&is_start==0
//////////    {
//////////		    
//////////        if (g_TimerTick - start_tick > 200)
//////////        { 
//////////			Buzzer();
//////////            is_start = 1;
//////////            is_end = 0;
//////////			//Delay_ms(3000);
//////////            
//////////            start_tick = g_TimerTick;
//////////        }
//////////    }
//////////    else start_tick = g_TimerTick;

//////////    // 3. 终点检测：X1+X5双路黑线（持续200ms防误判）
//////////    static uint32_t end_tick = 0;
//////////    if (g_GrayValue[0] > GRAY_THRESHOLD && g_GrayValue[4] > GRAY_THRESHOLD&&g_IsRunning==1)//&&is_end==0
//////////    {
//////////		
//////////		    
//////////        if (g_TimerTick - end_tick > 200)
//////////        { 
//////////			Buzzer();
//////////            is_end = 1;
//////////            is_start = 0;
//////////			//Delay_ms(3000);
//////////           
//////////            end_tick = g_TimerTick;
//////////        }
//////////    }
//////////    else end_tick = g_TimerTick;

//////    // 4. 循迹逻辑（仅启动时生效）
//////    if (g_IsRunning == 0)
//////    {
//////        MotorL_SetSpeed(0);
//////        MotorR_SetSpeed(0);
//////        return;
//////    }

//////    uint8_t baseSpeed = Speed;
//////    uint8_t adjustSpeed = 15;

//////    // 正向/反向循迹（模式三返程）
//////    if (g_ReturnFlag == 1)
//////    {
//////        // 反向：左右轮速度互换，实现返程
//////        if (g_GrayValue[2] > GRAY_THRESHOLD)
//////        {
//////            MotorL_SetSpeed(baseSpeed);
//////            MotorR_SetSpeed(baseSpeed);
//////            g_CurrentAngle = 0;
//////        }
//////        else if (g_GrayValue[1] > GRAY_THRESHOLD)
//////        {
//////            MotorL_SetSpeed(baseSpeed + adjustSpeed);
//////            MotorR_SetSpeed(baseSpeed - adjustSpeed);
//////            g_CurrentAngle = 10;
//////        }
//////        else if (g_GrayValue[0] > GRAY_THRESHOLD)
//////        {
//////            MotorL_SetSpeed(baseSpeed + 2*adjustSpeed);
//////            MotorR_SetSpeed(baseSpeed - 2*adjustSpeed);
//////            g_CurrentAngle = 20;
//////        }
//////        else if (g_GrayValue[3] > GRAY_THRESHOLD)
//////        {
//////            MotorL_SetSpeed(baseSpeed - adjustSpeed);
//////            MotorR_SetSpeed(baseSpeed + adjustSpeed);
//////            g_CurrentAngle = -10;
//////        }
//////        else if (g_GrayValue[4] > GRAY_THRESHOLD)
//////        {
//////            MotorL_SetSpeed(baseSpeed - 2*adjustSpeed);
//////            MotorR_SetSpeed(baseSpeed + 2*adjustSpeed);
//////            g_CurrentAngle = -20;
//////        }
//////        else
//////        {
//////            MotorL_SetSpeed(0);
//////            MotorR_SetSpeed(0);
//////            g_CurrentAngle = 999;
//////        }
//////    }
//////    else
//////    {   // Delay_ms(3000);
//////        // 正向循迹（原有逻辑）
//////		 if((g_GrayValue[0] > GRAY_THRESHOLD&&g_GrayValue[1] > GRAY_THRESHOLD&&g_GrayValue[2] < GRAY_THRESHOLD)||(g_GrayValue[0] > GRAY_THRESHOLD&&g_GrayValue[1] > GRAY_THRESHOLD&&g_GrayValue[2] >GRAY_THRESHOLD)){
////////		      MotorL_SetSpeed(0);
////////              MotorR_SetSpeed(0);
//////			do{
//////				g_GrayValue[2] = ADC_GetValue(ADC_CH_X3);
//////				MotorL_SetSpeed(0);//-baseSpeed
//////                MotorR_SetSpeed(baseSpeed);
//////			}while(g_GrayValue[2] < GRAY_THRESHOLD);
//////			 g_CurrentAngle = 90;
//////		//g_GrayValue[0] < GRAY_THRESHOLD&&g_GrayValue[1] < GRAY_THRESHOLD&&g_GrayValue[2] > GRAY_THRESHOLD&&g_GrayValue[3] < GRAY_THRESHOLD&&g_GrayValue[4] < GRAY_THRESHOLD||
//////		
//////		}
//////		
//////		
//////		else  if((g_GrayValue[3] > GRAY_THRESHOLD&&g_GrayValue[4] > GRAY_THRESHOLD&&g_GrayValue[2] < GRAY_THRESHOLD)||(g_GrayValue[3] > GRAY_THRESHOLD&&g_GrayValue[4] > GRAY_THRESHOLD&&g_GrayValue[2] >GRAY_THRESHOLD)){
////////		      MotorL_SetSpeed(0);
////////              MotorR_SetSpeed(0);
//////			do{
//////				g_GrayValue[4] = ADC_GetValue(ADC_CH_X1);
//////				MotorL_SetSpeed(baseSpeed);
//////                MotorR_SetSpeed(0);//-baseSpeed
//////			}while(g_GrayValue[4] > GRAY_THRESHOLD);
//////			 g_CurrentAngle = -90;
//////		//g_GrayValue[0] < GRAY_THRESHOLD&&g_GrayValue[1] < GRAY_THRESHOLD&&g_GrayValue[2] > GRAY_THRESHOLD&&g_GrayValue[3] < GRAY_THRESHOLD&&g_GrayValue[4] < GRAY_THRESHOLD||
//////		
//////		}
//////    
//////		else  if (g_GrayValue[2] > GRAY_THRESHOLD|| 
//////            (g_GrayValue[1] > GRAY_THRESHOLD && g_GrayValue[2] > GRAY_THRESHOLD && g_GrayValue[3] > GRAY_THRESHOLD ))
//////        {
//////            MotorL_SetSpeed(baseSpeed);
//////            MotorR_SetSpeed(baseSpeed);
//////            g_CurrentAngle = 0;
//////        }
//////        else if (g_GrayValue[1] > GRAY_THRESHOLD)
//////        {
//////            MotorL_SetSpeed(baseSpeed - adjustSpeed);
//////            MotorR_SetSpeed(baseSpeed + adjustSpeed);
//////            g_CurrentAngle = -10;
//////        }
//////        else if (g_GrayValue[0] > GRAY_THRESHOLD)
//////        {
//////            MotorL_SetSpeed(baseSpeed - 2*adjustSpeed);
//////            MotorR_SetSpeed(baseSpeed + 2*adjustSpeed);
//////            g_CurrentAngle = -20;
//////        }
//////		
//////		  
//////        else if (g_GrayValue[3] > GRAY_THRESHOLD)
//////        {
//////            MotorL_SetSpeed(baseSpeed + adjustSpeed);
//////            MotorR_SetSpeed(baseSpeed - adjustSpeed);
//////            g_CurrentAngle = 10;
//////        }
//////        else if (g_GrayValue[4] > GRAY_THRESHOLD)
//////        {
//////            MotorL_SetSpeed(baseSpeed + 2*adjustSpeed);
//////            MotorR_SetSpeed(baseSpeed - 2*adjustSpeed);
//////            g_CurrentAngle = 20;
//////        }
//////        else
//////        {
//////            MotorL_SetSpeed(0);
//////            MotorR_SetSpeed(0);
//////            g_CurrentAngle = 999;
//////        }
//////    }
//////}
////////	if((g_GrayValue[1]>GRAY_THRESHOLD)&&(g_GrayValue[2]>GRAY_THRESHOLD)&&(g_GrayValue[3]>GRAY_THRESHOLD))
////////	{
////////		
////////		is_end=0;
////////		is_start=1;
////////		Buzzer();
////////	
////////		}
////////	if((g_GrayValue[1]>GRAY_THRESHOLD)&&(g_GrayValue[2]>GRAY_THRESHOLD)&&(g_GrayValue[3]>GRAY_THRESHOLD))
////////	//if((g_GrayValue[0]>GRAY_THRESHOLD)&&(g_GrayValue[4]>GRAY_THRESHOLD))
////////	{
////////		
////////		is_end=1;
////////		is_start=0;
////////		Buzzer();
////////	
////////		}
////////	
////////    // 2. 循迹逻辑（核心：根据黑线位置调整电机转向）
////////    if(g_IsRunning == 0) // 未启动模式时，电机停止
////////    {
////////        MotorL_SetSpeed(0);
////////        MotorR_SetSpeed(0);
////////        return;
////////    }

////////    // 定义电机基础速度（可根据实际修改）
////////    uint8_t baseSpeed = Speed; // 复用你代码中的speed变量（初始25）
////////    uint8_t adjustSpeed = 10;  // 转向补偿速度

////////    // 3. 5路灰度判断（黑线=高电平，ADC值>阈值）
////////    if(g_GrayValue[2] > GRAY_THRESHOLD) // 中间X3检测到黑线：直行
////////    {
////////        MotorL_SetSpeed(baseSpeed);
////////        MotorR_SetSpeed(baseSpeed);
////////        g_CurrentAngle = 0; // 中间线，角度0
////////    }
////////    else if(g_GrayValue[1] > GRAY_THRESHOLD) // X2（左偏1级）：轻微左转
////////    {
////////        MotorL_SetSpeed(baseSpeed - adjustSpeed);
////////        MotorR_SetSpeed(baseSpeed + adjustSpeed);
////////        g_CurrentAngle = -10; // 角度左偏10°（适配OLED显示）
////////    }
////////    else if(g_GrayValue[0] > GRAY_THRESHOLD) // X1（左偏2级）：大幅左转
////////    {
////////        MotorL_SetSpeed(baseSpeed - 2*adjustSpeed);
////////        MotorR_SetSpeed(baseSpeed + 2*adjustSpeed);
////////        g_CurrentAngle = -20; // 角度左偏20°
////////    }
////////    else if(g_GrayValue[3] > GRAY_THRESHOLD) // X4（右偏1级）：轻微右转
////////    {
////////        MotorL_SetSpeed(baseSpeed + adjustSpeed);
////////        MotorR_SetSpeed(baseSpeed - adjustSpeed);
////////        g_CurrentAngle = 10; // 角度右偏10°
////////    }
////////    else if(g_GrayValue[4] > GRAY_THRESHOLD) // X5（右偏2级）：大幅右转
////////    {
////////        MotorL_SetSpeed(baseSpeed + 2*adjustSpeed);
////////        MotorR_SetSpeed(baseSpeed - 2*adjustSpeed);
////////        g_CurrentAngle = 20; // 角度右偏20°
////////    }
////////    else // 无黑线（脱线）：停止或减速
////////    {
////////        MotorL_SetSpeed(0);
////////        MotorR_SetSpeed(0);
////////        g_CurrentAngle = 999; // 脱线标记（OLED可显示异常）
////////    }
////////}

////////void Trace_Init(void)
////////{
////////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
////////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
////////	
////////	GPIO_InitTypeDef GPIO_InitStructure;
////////	
////////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
////////	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
////////	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_10|GPIO_Pin_15|GPIO_Pin_2;
////////	GPIO_Init(GPIOA, &GPIO_InitStructure);
////////	
//////	
//////	//5个循迹初始数字量
////////		
////////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
////////	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
////////	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;  //循迹
////////	GPIO_Init(GPIOB, &GPIO_InitStructure);
////////}	



////////寻迹任务
//////////void Trace_task(void)//直和曲线
//////////{

//////////	 if(X3() == 1) Motor_SetSpeed(25, 25);
//////////     else if(X2() == 1 && X1()==0 && X3()==0 && X4()==0 && X5()==0) Motor_SetSpeed(10, 25);
//////////     else if(X4() == 1 && X1()==0 && X2()==0 && X3()==0 && X5()==0) Motor_SetSpeed(25, 10);
//////////     else if(X1() == 1 && X2()==0 &&X3()==0 && X4()==0 && X5()==0) Motor_SetSpeed(-10, 30);
//////////     else if(X5() == 1 && X1()==0 && X2()==0 && X3()==0 && X4()==0) Motor_SetSpeed(30, -10);
//////////     else if(X2()==1 && X3()==1 && X1()==0 && X4()==0 &&X5()==0) Motor_SetSpeed(2, 25);
//////////     else if(X3()==1 && X4()==1 && X1()==0 && X2()==0 && X5()==0) Motor_SetSpeed(25, 5);
//////////     else if(X1()==1 && X2()==1 && X3()==1 && X4()==0 && X5()==0) Motor_SetSpeed(0, 25);
//////////     else if(X3()==1 && X4()==1 && X5()==1 && X1()==0 && X2()==0) Motor_SetSpeed(-5, 25);
//////////     else if(X1()==1 && X2()==1 && X3()==1 && X4()==1 && X5()==0) Motor_SetSpeed(0, 25);
//////////     else if(X4()==1 && X5()==1 && X1()==0 && X2()==0 && X3()==0) Motor_SetSpeed(25, 0);
//////////     else if(X1()==1 && X2()==1 && X3()==0 && X4()==0 && X5()==0) Motor_SetSpeed(0, 25);
//////////     else if(X2()==1 && X3()==1 && X4()==1 && X5()==1 && X1()==0) Motor_SetSpeed(25, -5);
//////////     else if(X1()==1 && X2()==1 && X3()==1 && X4()==1 && X5()==1) Motor_SetSpeed(25, 25);
//////////     else Motor_SetSpeed(0, 0);


//////////}


////////// 检测起点和终点（X1-X5全为1时触发）
////////void Check_Start_End(void)
////////{
////////    // 读取5路红外传感器的状态
////////    uint8_t x1 = X1(), x2 = X2(), x3 = X3(), x4 = X4(), x5 = X5();
////////    
////////    // 判定条件：X1~X5全为高电平（1）
////////    if(x1 && x2 && x3 && x4 && x5) 
////////    {
////////        if(!is_start) // 还没检测过起点 → 判定为起点
////////        {
////////            is_start = 1;          // 置位起点标志
////////           Buzzer();      // 蜂鸣器响提示
////////        }
////////        else if(is_start && !is_end) // 已过起点，未到终点 → 判定为终点
////////        {
////////            is_end = 1;            // 置位终点标志
////////             Buzzer();     // 蜂鸣器响1000ms提示
////////        }
////////    }
////////}


//////////void Trace_1(void){
//////////	//
//////////	is_end=0;
//////////	is_start=0;
//////////	Motor_SetSpeed(0,0);
//////////	Delay_ms(3000);
//////////	
//////////	while(1){
//////////		Check_Start_End();
//////////		Trace_task();
//////////		if(is_end){
//////////			Motor_SetSpeed(0,0);
//////////			is_end=0;
//////////			//
//////////		break;
//////////		}
//////////	}
//////////}



//////////void Trace_2(void){
//////////	uint8_t i=0;
//////////	if(!g_IsRunning) return; // 未运行则直接返回

//////////    Check_Start_End(); // 检测起点/终点
//////////    Trace_task();      // 执行单次循迹

//////////    // 到达终点则圈数+1，重置终点标志
//////////    if(is_end)
//////////    {
//////////        i++;
//////////        is_end = 0;
//////////        Motor_SetSpeed(50, 50); // 终点后直行
//////////        Delay_ms(500);
//////////    }

//////////    // 跑满3圈则停止循迹
//////////    if(i >= 3)
//////////    {
//////////        Motor_SetSpeed(0, 0);
//////////        g_IsRunning = 0;
//////////        i = 0; // 重置圈数
//////////    }
////////}
////////	is_end=0;
////////	is_start=0;
////////	uint8_t i=0;
////////	Check_Start_End();
////////	Trace_task();
////////	while(i< 3) // 循环3圈
////////    {
////////        Check_Start_End();
////////        Trace_task();
////////        if(is_end) // 到达终点
////////        {
////////            i++;       // 圈数+1
////////            is_end = 0;         // 重置终点标志
////////            // 终点后继续走直线（可根据实际需求调整直线行走的时长/距离）
////////            Motor_SetSpeed(25, 25); 
////////           
////////			Delay_ms(500);      // 直线行走500ms，需根据硬件调试
////////        }
////////   		Delay_ms(10);
////////		break;
////////    }
////////    	Motor_SetSpeed(0, 0); // 3圈完成后停止
////////}



//////////void Trace_3(void){

//////////	is_start = 0;
//////////     is_end = 0;
//////////     
//////////     Motor_SetSpeed(0, 0);
//////////     Delay_ms(3000);
//////////     while(1)
//////////     {
//////////         Check_Start_End();
//////////         Trace_task();
//////////         // 到达终点后，延时3秒开始倒车
//////////         if(is_end)
//////////         {
//////////             Motor_SetSpeed(0, 0);
//////////             //Buzzer_Beep(1000);
//////////             Delay_ms(3000); // 终点延时3秒
//////////             is_start = 0;   // 重置起点标志，检测倒车返回的起点
//////////             // 倒车循迹逻辑
//////////             while(!is_start)
//////////             {
//////////                 //reverse_Trace_task(); // 倒车循迹函数（需实现）
//////////					uint8_t x1 = X1(), x2 = X2(), x3 = X3(), x4 = X4(), x5 = X5();
//////////					// 倒车时：左电机和右电机速度逻辑与前进相反，转向幅度减小提升平稳性
//////////					if(x3 == 1) 
//////////						Motor_SetSpeed(-40, -40); // 直退，速度略低于前进
//////////					else if(x2 == 1 && x1==0 && x3==0 && x4==0 && x5==0) 
//////////						Motor_SetSpeed(-50, -20); // 左偏，倒车时右拐修正
//////////					else if(x4 == 1 && x1==0 && x2==0 && x3==0 && x5==0) 
//////////						Motor_SetSpeed(-20, -50); // 右偏，倒车时左拐修正
//////////					else if(x1 == 1 && x2==0 && x3==0 && x4==0 && x5==0) 
//////////						Motor_SetSpeed(-60, 20);  // 大幅左偏，倒车时急右拐
//////////					else if(x5 == 1 && x1==0 && x2==0 && x3==0 && x4==0) 
//////////						Motor_SetSpeed(20, -60);  // 大幅右偏，倒车时急左拐
//////////					else if(x2==1 && x3==1 && x1==0 && x4==0 && x5==0) 
//////////						Motor_SetSpeed(-50, -10); // 轻微左偏，小幅度修正
//////////					else if(x3==1 && x4==1 && x1==0 && x2==0 && x5==0) 
//////////						Motor_SetSpeed(-10, -50); // 轻微右偏，小幅度修正
//////////					else if(x1==1 && x2==1 && x3==1 && x4==0 && x5==0) 
//////////						Motor_SetSpeed(0, -40);   // 左侧贴线，倒车右移
//////////					else if(x3==1 && x4==1 && x5==1 && x1==0 && x2==0) 
//////////						Motor_SetSpeed(-40, 0);   // 右侧贴线，倒车左移
//////////					else 
//////////						Motor_SetSpeed(0, 0);     // 无检测到线，停止防止失控
//////////								 
//////////				 
//////////				 
//////////                 Check_Start_End();    // 检测是否回到起点
//////////                 Delay_ms(15);
//////////             }
//////////             // 模式3结束条件：回到起点
//////////             Motor_SetSpeed(0, 0);
//////////            // Buzzer_Beep(2000);
//////////             //g_IsRunning = 0;
//////////             break; // 跳出循环，退出该循迹模式
//////////         }
//////////         Delay_ms(10);
//////////     }

//////////}




