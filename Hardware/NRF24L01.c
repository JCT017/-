# include "NRF24L01.h"
#include "NRF24L01_Ins.h"
#include "stm32f10x.h"                  // Device header
#include "Delay.h"


#define TX_ADR_WIDTH    5     //5字节地址宽度
#define RX_ADR_WIDTH    5     //5字节地址宽度
#define TX_PLOAD_WIDTH  32    //32字节有效数据宽度
#define RX_PLOAD_WIDTH  32    //32字节有效数据宽度

const uint8_t TX_ADDRESS[TX_ADR_WIDTH]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
const uint8_t RX_ADDRESS[RX_ADR_WIDTH]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

//改变SS引脚的输出电平
void W_SS(uint8_t BitValue)
{

	GPIO_WriteBit(CSN_Port, CSN_Pin, (BitAction)BitValue);
} 


//改变CE引脚的输出电平
void W_CE(uint8_t BitValue)
{
	GPIO_WriteBit(CE_Port, CE_Pin, (BitAction)BitValue);
} 


//改变SCK引脚的输出电平
void W_SCK(uint8_t BitValue)
{
	GPIO_WriteBit(SCK_Port, SCK_Pin, (BitAction)BitValue);
} 


//改变MOSI引脚的输出电平
void W_MOSI(uint8_t BitValue)
{
	GPIO_WriteBit(MOSI_Port, MOSI_Pin,(BitAction)BitValue);
} 


//读取MISO引脚的输出电平
uint8_t R_MISO(void)
{
	return GPIO_ReadInputDataBit(MISO_Port, MISO_Pin);
}


//读取IRQ引脚的输出电平 
uint8_t R_IRQ(void)
{
	return GPIO_ReadInputDataBit(IRQ_Port, IRQ_Pin);
}



//初始化NRF24L01使用的引脚
//其中CSN、SCK、MOSI、CE设置为推挽输出 IRQ、MISO设置为上拉输入模式
void NRF24L01_Pin_Init(void)	
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = CSN_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CSN_Port, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = SCK_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SCK_Port, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = MOSI_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(MOSI_Port, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = CE_Pin;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(CE_Port, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = MISO_Pin;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(MISO_Port, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = IRQ_Pin;  			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(IRQ_Port, &GPIO_InitStructure);
	
}
		

//标准SPI交换一个字节
uint8_t SPI_SwapByte(uint8_t Byte)  
{
	uint8_t i;
	for(i = 0; i < 8; i ++) 
	{
		if((uint8_t)(Byte & 0x80) == 0x80)
		{
			W_MOSI(1);
		} 		
		else 
		{
			W_MOSI(0);
		}			
		Byte = (Byte << 1);			
		W_SCK(1);		//模拟时钟上升沿			
		Byte |= R_MISO();	 //接收从机发来的一个数据        
		W_SCK(0);		//模拟时钟下降沿			
	}
	return Byte;//交换从机的一个完整字节并返回
}

//通过调用SPI与NRF24L01交换一个字节
//Reg：交换的第一个字节就是从机的寄存器地址
//Value：交换的第二个字节就是要在这个寄存器写入的数据
uint8_t NRF24L01_Write_Reg(uint8_t Reg, uint8_t Value)
{
	uint8_t Status;

	W_SS(0);                  
  	Status = SPI_SwapByte(Reg);
	SPI_SwapByte(Value);	
	W_SS(1);                 

	return Status;
}

//通过调用SPI读取NRF24L01的一个字节
//Reg：交换的第一个字节就是要读取的从机的寄存器地址
//Value：再次调用SPI的交换函数，用NOP交换会寄存器的储存的数据并用Value接收并返回
uint8_t NRF24L01_Read_Reg(uint8_t Reg)
{
 	uint8_t Value;

	W_SS(0);              
  	SPI_SwapByte(Reg);		
	Value = SPI_SwapByte(NOP);
	W_SS(1);             	

	return Value;
}


//在指定的寄存器写入一堆数据，和上面函数的关系上面的函数只写一个，这个可以写很多个
//Reg：要读取的寄存器地址
//*Buf：要写入寄存器的的数据的首地址
//Len：要写入的数据长度
uint8_t NRF24L01_Write_Buf(uint8_t Reg, uint8_t *Buf, uint8_t Len)
{
	uint8_t Status, i;
	W_SS(0); 
	Status = SPI_SwapByte(Reg);		
	for(i = 0; i < Len; i ++)
	{
		SPI_SwapByte(*Buf ++); 		
	}
	W_SS(1); 
	return Status;     		
}	


//在指定的寄存器读取一堆数据，和上面函数的区别就是上面的读一个这个能读很多个
//Reg：要读取的寄存器地址
//*Buf：用于存储读取到的数据的数组的首地址
//Len：要读取的数据的长度
uint8_t NRF24L01_Read_Buf(uint8_t Reg, uint8_t *Buf, uint8_t Len)
{
	uint8_t Status, i;
	W_SS(0);   //下拉SS表示选中从机                 	
	Status =SPI_SwapByte(Reg);		
 	for(i = 0; i < Len; i ++)
	{
		Buf[i] = SPI_SwapByte(NOP);	
	}
	W_SS(1);   //拉高SS表示通讯结束              		
	return Status;   //返回状态值作用不大     			
}


//通过调用函数向WR_TX_PLOAD寄存器写入要发送的数据（在WR_TX_PLOAD寄存器中写入的数据就是要发送的数据）
//*Buf：要写入寄存器发射的数据的数组的首地址，也就是要发送的数据，最多可以发送32个字节

uint8_t NRF24L01_SendTxBuf(uint8_t *Buf)
{
	uint8_t State;
   
	W_CE(0);		//CE置0调为发射模式						
  	NRF24L01_Write_Buf(WR_TX_PLOAD, Buf, TX_PLOAD_WIDTH);	
 	W_CE(1);		//CE置1发射										
	while(R_IRQ() == 1);  //检测中断位								
	State = NRF24L01_Read_Reg(STATUS);  					
	NRF24L01_Write_Reg(nRF_WRITE_REG + STATUS, State); 		//清除中断位
	if(State&MAX_TX)		//判断是否发射达到最大次数触发中断		 						
	{
		NRF24L01_Write_Reg(FLUSH_TX, NOP);					
		return MAX_TX; 
	}
	if(State & TX_OK)	  //判断是否发射成功触发中断
	{
		return TX_OK;
	}
	return NOP;					//其他情况返回NOP
}						  					   


//通过调用函数读取RD_RX_PLOAD寄存器读取接收到的数据（在RD_RX_PLOAD寄存器中储存的就是接收到的数据）
//*Buf：储存读取到的数据的首地址
//和上面的函数相比没有CE的设置，因为默认状态下CE为高电平，就是默认的接收模式
uint8_t NRF24L01_GetRxBuf(uint8_t *Buf)
{
	uint8_t State;
	State = NRF24L01_Read_Reg(STATUS);  //读取STATUS状态寄存器判读触发的中断类型			
	NRF24L01_Write_Reg(nRF_WRITE_REG + STATUS, State); //再次在STATUS寄存器的位置写入1可以重置中断
	if(State & RX_OK)								
	{                                                       
		W_CE(1);						//为了确保是接收模式，假设不写也没关系，但是写了更保险
		NRF24L01_Read_Buf(RD_RX_PLOAD, Buf, RX_PLOAD_WIDTH); 
		NRF24L01_Write_Reg(FLUSH_RX, NOP);					
		W_CE(1);				
		Delay_us(150);      //适当延迟以延长接收时间
		return 0; 
	}	   
	return 1;
}

//用于检测

uint8_t NRF24L01_Check(void)   
{
	uint8_t check_in_buf[5] = {0x11 ,0x22, 0x33, 0x44, 0x55};
	uint8_t check_out_buf[5] = {0x00};
	
//保证SCK、SS、CE被初始化
	W_SCK(0);
	W_SS(1);   
	W_CE(0);	

	
//将check_in_buf数组中的数据写入TX_ADDR发送地址寄存器中
	NRF24L01_Write_Buf(nRF_WRITE_REG + TX_ADDR, check_in_buf, 5);
	
	
//读取TX_ADDR寄存器里的数据并储存在check_out_buf中
	NRF24L01_Read_Buf(nRF_READ_REG + TX_ADDR, check_out_buf, 5);

	
	
//鉴定check_out_buf中储存的数据和写入的check_in_buf中的一样，如果一样说明NRF24L01已经连接
	if((check_out_buf[0] == 0x11) && (check_out_buf[1] == 0x22) && (check_out_buf[2] == 0x33) && (check_out_buf[3] == 0x44) && (check_out_buf[4] == 0x55))
	{
		return 0; //如果检测成功返回0
	}
	else 
	{
		return 1; //检测失败返回1
	}
}			

//初始化NRF24L01的发送或者接收状态
void NRF24L01_RT_Init(void) 
{	
	W_CE(0);	//以下可能要进行接收或者发射模式的转换，模式转换时CE引脚要先置0
		
	//初始化设置接收数据的数据宽度（为32字节）
  	NRF24L01_Write_Reg(nRF_WRITE_REG+RX_PW_P0, RX_PLOAD_WIDTH);
	
	//调用FLUSH_RX指令可以清空接收缓存区，发送的NOP没用
		NRF24L01_Write_Reg(FLUSH_RX, NOP);		

  //写入TX_ADDR寄存器的是发送的数据的接收地址，TX_ADR_WIDTH为固定的地址长度（长度为5个字节）
  	NRF24L01_Write_Buf(nRF_WRITE_REG + TX_ADDR, (uint8_t*)TX_ADDRESS, TX_ADR_WIDTH);
	
	//写入RX_ADDR_P0确认第一个接收地址（最多可以有6个），RX_ADR_WIDTH为固定的地址长度（长度为5个字节）
  	NRF24L01_Write_Buf(nRF_WRITE_REG + RX_ADDR_P0, (uint8_t*)RX_ADDRESS, RX_ADR_WIDTH);   
	
	//启动自动应答
  	NRF24L01_Write_Reg(nRF_WRITE_REG + EN_AA, 0x01);

	//使能数据接收的通道，这里只使用通道0，因此在EN_RXADDR寄存器中写入0000 0001
  	NRF24L01_Write_Reg(nRF_WRITE_REG + EN_RXADDR, 0x01); 
	
	//配置SETUP_RETR寄存器 0001 1010 高四位表示延时槽，0001：表示两个延时槽（250×2=86=586us） 1010：表示重发10次
  	NRF24L01_Write_Reg(nRF_WRITE_REG + SETUP_RETR, 0x1A);
		
	//工作频率=(2400 + RF_CH) MHz，在这个寄存器写0表示工作频率为2.4G
  	NRF24L01_Write_Reg(nRF_WRITE_REG + RF_CH, 0);        
		
	//配置寄存器0000 1111 配置只有接收到对应地址触发中断，默认进入接收模式，且发射结束后自动进入接收模式接收应答信号
	//B东西无脑配成0x0F就行
  //NRF24L01_Write_Reg(nRF_WRITE_REG + RF_SETUP, 0x0F);  
		
	//CONFIG寄存器就这个样配置（默认配置为接收模式）
  	NRF24L01_Write_Reg(nRF_WRITE_REG + CONFIG, 0x0F);    
	W_CE(1);									  
}

//初始化
//引脚初始化，检查，NRF24L01寄存器初始化
void NRF24L01_Init()
{
	NRF24L01_Pin_Init();
	while(NRF24L01_Check());
	NRF24L01_RT_Init();
	
}


//
void NRF24L01_SendBuf(uint8_t *Buf)
{
	W_CE(0);	//模式转换							
  
	//CONFIG最低位置0，配置为发射模式
	NRF24L01_Write_Reg(nRF_WRITE_REG + CONFIG, 0x0E); 
	
	W_CE(1);
	
	Delay_us(15);//15us之后进入发射模式
	
	NRF24L01_SendTxBuf(Buf);//将数据写入寄存器中发射						
	
	W_CE(0);//模式转换
	NRF24L01_Write_Reg(nRF_WRITE_REG + CONFIG, 0x0F);
	W_CE(1);	
}


//看看NRF24L01是不是触发中断
uint8_t NRF24L01_Get_Value_Flag(void)
{
	return R_IRQ();
}

