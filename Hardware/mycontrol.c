#include "stm32f10x.h"
#include "Delay.h"
#include "Trace.h"
#include "Motor.h"





////寻迹任务
//void Trace_task(void)//直和曲线
//{

//	 if(X3() == 1) Motor_SetSpeed(25, 25);
//     else if(X2() == 1 && X1()==0 && X3()==0 && X4()==0 && X5()==0) Motor_SetSpeed(10, 25);
//     else if(X4() == 1 && X1()==0 && X2()==0 && X3()==0 && X5()==0) Motor_SetSpeed(25, 10);
//     else if(X1() == 1 && X2()==0 &&X3()==0 && X4()==0 && X5()==0) Motor_SetSpeed(-10, 30);
//     else if(X5() == 1 && X1()==0 && X2()==0 && X3()==0 && X4()==0) Motor_SetSpeed(30, -10);
//     else if(X2()==1 && X3()==1 && X1()==0 && X4()==0 &&X5()==0) Motor_SetSpeed(2, 25);
//     else if(X3()==1 && X4()==1 && X1()==0 && X2()==0 && X5()==0) Motor_SetSpeed(25, 5);
//     else if(X1()==1 && X2()==1 && X3()==1 && X4()==0 && X5()==0) Motor_SetSpeed(0, 25);
//     else if(X3()==1 && X4()==1 && X5()==1 && X1()==0 && X2()==0) Motor_SetSpeed(-5, 25);
//     else if(X1()==1 && X2()==1 && X3()==1 && X4()==1 && X5()==0) Motor_SetSpeed(0, 25);
//     else if(X4()==1 && X5()==1 && X1()==0 && X2()==0 && X3()==0) Motor_SetSpeed(25, 0);
//     else if(X1()==1 && X2()==1 && X3()==0 && X4()==0 && X5()==0) Motor_SetSpeed(0, 25);
//     else if(X2()==1 && X3()==1 && X4()==1 && X5()==1 && X1()==0) Motor_SetSpeed(25, -5);
//     else if(X1()==1 && X2()==1 && X3()==1 && X4()==1 && X5()==1) Motor_SetSpeed(25, 25);
//     else Motor_SetSpeed(0, 0);


//}


//// 模式1：单次循迹，到终点停止
//void trace_1(void)
//{
//    is_start = 0;
//    is_end = 0;
//    while(1)
//    {
//        Check_Start_End();   // 检测起点/终点
//        tracetask();         // 执行循迹逻辑
//        if(is_end)           // 到达终点
//        {
//            Motor_SetSpeed(0, 0); // 电机停止
//            break;
//        }
//        delay_ms(10); // 循迹周期，可根据需求调整
//    }
//}

//// 模式2：循迹3圈后停止
//void trace_2(void)
//{
//    is_start = 0;
//    is_end = 0;
//    circle_cnt = 0;
//    while(circle_cnt < 3) // 循环3圈
//    {
//        Check_Start_End();
//        tracetask();
//        if(is_end) // 到达终点
//        {
//            circle_cnt++;       // 圈数+1
//            is_end = 0;         // 重置终点标志
//            // 终点后继续走直线（可根据实际需求调整直线行走的时长/距离）
//            Motor_SetSpeed(50, 50); 
//            delay_ms(500);      // 直线行走500ms，需根据硬件调试
//        }
//        delay_ms(10);
//    }
//    Motor_SetSpeed(0, 0); // 3圈完成后停止
//}

//// 模式3：到终点延时3秒，倒车返回起点后停止
//void trace_3(void)
//{
//    is_start = 0;
//    is_end = 0;
//    while(1)
//    {
//        Check_Start_End();
//        tracetask();
//        if(is_end) // 到达终点
//        {
//            Motor_SetSpeed(0, 0); // 先停止
//            delay_ms(3000);       // 延时3秒
//            // 倒车返回起点（反向循迹，可根据实际调试倒车逻辑）
//            is_start = 0; // 重置起点标志，重新检测起点
//            while(!is_start)
//            {
//                // 倒车循迹：将tracetask的电机速度取反（示例，需根据硬件调试）
//                uint8_t x1 = X1(), x2 = X2(), x3 = X3(), x4 = X4(), x5 = X5();
//                if(x3 == 1) Motor_SetSpeed(-50, -50);
//                else if(x2 == 1) Motor_SetSpeed(-20, -50);
//                else if(x4 == 1) Motor_SetSpeed(-50, -20);
//                else if(x1 == 1) Motor_SetSpeed(60, -20);
//                else if(x5 == 1) Motor_SetSpeed(-20, 60);
//                else Motor_SetSpeed(0, 0);
//                Check_Start_End(); // 检测是否回到起点
//                delay_ms(10);
//            }
//            Motor_SetSpeed(0, 0); // 回到起点后停止
//            break;
//        }
//        delay_ms(10);
//    }
//}



//	if((X1()==0&&X2()==0&&X3()==1&&X4()==0&&X5()==0)) Motor_SetSpeed(50,50);
//		
//	if(X1()==0&&X2()==0&&X3()==1&&X4()==1&&X5()==0) Motor_SetSpeed(0,50);
//		
//	if(X1()==0&&X2()==1&&X3()==1&&X4()==0&&X5()==0) Motor_SetSpeed(0,50);
//		
//	if(X5()==0&&X4()==1&&X3()==1&&X2()==1&&X1()==1)Motor_SetSpeed(50,0);
//	
//	if(X5()==0&&X4()==0&&X3()==0&&X2()==1&&X1()==0) Motor_SetSpeed(-10,50);
//	if(X5()==0&&X4()==1&&X3()==0&&X2()==0&&X1()==0) Motor_SetSpeed(50,-10);
//	//if(X2==1&&X1==1&&X3==1&&X4==1) Set_Car_Speed(4000,4000);
