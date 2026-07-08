#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "PetAction.h"
#include "Face_Config.h"
#include "wexcube.h"
#include "RobotState.h"
#include "robot_config.h"

// 注意：全局变量已移至main.c中的robot结构体
// 此处不再定义单独的全局变量

void BlueTooth_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//开启串口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//开启GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//开启串口时钟
	
	//语音
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//复用推挽输出模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;//默认PA9是USART1_TX的复用
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//复用上拉输入模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//默认PA10是USART1_RX的复用
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//蓝牙
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//复用推挽输出模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//默认PB10是USART3_TX的复用
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//复用上拉输入模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;//默认PB11是USART3_RX的复用
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	USART_InitTypeDef UASRT_InitStructure;//USART初始化
	UASRT_InitStructure.USART_BaudRate=9600;//波特率9600
	UASRT_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;//不需要硬件流控制
	UASRT_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;//接受与发送均打开
	UASRT_InitStructure.USART_Parity=USART_Parity_No;//不需要奇偶校验
	UASRT_InitStructure.USART_StopBits=USART_StopBits_1;//停止位为1
	UASRT_InitStructure.USART_WordLength=USART_WordLength_8b;//字长8位
	USART_Init(USART1,&UASRT_InitStructure);
	USART_Init(USART3,&UASRT_InitStructure);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//语音接收中断配置，也就是如果接送到消息就直接中断
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);//蓝牙接收中断配置，也就是如果接送到消息就直接中断
	
	//中断
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//分组2
	//语音中断配置
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;//特定的通道
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//通道使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占式优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;//响应优先级为2
	NVIC_Init(&NVIC_InitStructure);
	//蓝牙中断配置
	NVIC_InitStructure.NVIC_IRQChannel=USART3_IRQn;//特定的通道
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;//通道使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占式优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;//响应优先级为2
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1,ENABLE);//USART使能打开
	USART_Cmd(USART3,ENABLE);//USART使能打开
}

void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE)==SET)//如果接受到
	{
		uint8_t data = USART_ReceiveData(USART1);
		
		if(data==0x29)//放松的趴下
		{
			robot.face_mode=0;
			Face_Config();
			robot.action_mode=0;
		}
		if(data==0x30)//蹲下
		{
			robot.face_mode=1;
			Face_Config();
			robot.action_mode=1;
		}
		if(data==0x31)//直立
		{
			robot.face_mode=5;
			Face_Config();
			robot.action_mode=2;

		}
		if(data==0x32)//坐下
		{
			robot.face_mode=1;
			Face_Config();
			robot.action_mode=3;
		}
		if(data==0x33)//前进
		{
			robot.face_mode=2;
			Face_Config();
			robot.action_mode=4;
		}
		if(data==0x34)//后退
		{
			robot.face_mode=2;
			Face_Config();
			robot.action_mode=5;
		}
		if(data==0x35)//左转
		{
			robot.face_mode=2;
			Face_Config();
			robot.action_mode=6;
		}
		if(data==0x36)//右转
		{
			robot.face_mode=2;
			Face_Config();
			robot.action_mode=7;
		}
		if(data==0x37)//摇摆
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=8;
		}
		
		if(data==0x38)//左前走
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=9;
		}
		if(data==0x39)//左后走
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=10;
		}
		if(data==0x40)//右前走
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=11;
		}
		if(data==0x41)//右后走
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=12;
		}
		if(data==0x42)//右走
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=13;
		}
		if(data==0x43)//左走
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=14;
		}
		if(data==0x44)//测试
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=15;
		}
		if(data==0x45)//跳舞
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=16;
		}

		if(data==0x46)//踏步
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=17;
		}
		
		if(data==0x47)//撒尿
		{
			robot.face_mode=4;
			Face_Config();
			robot.action_mode=18;
		}
		
		if(data==0x50)//身高加
		{
			robot.action_high=1;
			robot.height=DEFAULT_HEIGHT_MM;
		}
		if(data==0x51)//身高减
		{
			robot.action_high=2;
			robot.height=MIN_HEIGHT_MM;
		}
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}
}

// USART3_IRQHandler 已移动到 wexcube_port.c 中处理 WeXCube 协议
// 注意：USART3中断处理已集成到WeXCube库中
// 详见 wexcube_port.c 中的 USART3_IRQHandler 函数