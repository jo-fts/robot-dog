#include "stm32f10x.h"                  // Device header

void PWM_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//开启TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//开启TIM3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);//开启TIM4时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//开启GPIOB时钟
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;//复用推挽输出模式
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;//默认PA0是TIM2通道1的复用，PA1是TIM2通道2的复用所以开启这俩IO口...
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //重映射TIM3通道1和2到PB0和PB1
    GPIO_Init(GPIOB   , &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9; //TIM4通道1和2复用引脚
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    TIM_InternalClockConfig(TIM2);//TIM2切换为内部定时器
    TIM_InternalClockConfig(TIM3);//TIM3切换为内部定时器
    TIM_InternalClockConfig(TIM4);//TIM4切换为内部定时器
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;//不分频
    TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInitStructure.TIM_Period=20000-1;
    TIM_TimeBaseInitStructure.TIM_Prescaler=72-1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
    TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
    
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;//输出比较模式采用PWM1
    TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse=0;//初始化CCR的值为0
    TIM_OC1Init(TIM2,&TIM_OCInitStructure);//TIM2复用通道1开启
    TIM_OC2Init(TIM2,&TIM_OCInitStructure);//TIM2复用通道2开启
    TIM_OC3Init(TIM2,&TIM_OCInitStructure);//TIM2复用通道3开启
    TIM_OC4Init(TIM2,&TIM_OCInitStructure);//TIM2复用通道4开启
    
    TIM_OC1Init(TIM3,&TIM_OCInitStructure);//TIM3复用通道1开启
    TIM_OC2Init(TIM3,&TIM_OCInitStructure);//TIM3复用通道2开启
    TIM_OC3Init(TIM3,&TIM_OCInitStructure);//TIM3复用通道3开启
    TIM_OC4Init(TIM3,&TIM_OCInitStructure);//TIM3复用通道4开启
    
    TIM_OC1Init(TIM4,&TIM_OCInitStructure);//TIM4复用通道1开启
    TIM_OC2Init(TIM4,&TIM_OCInitStructure);//TIM4复用通道2开启
    TIM_OC3Init(TIM4,&TIM_OCInitStructure);//TIM4复用通道3开启
    TIM_OC4Init(TIM4,&TIM_OCInitStructure);//TIM4复用通道4开启
    
    TIM_Cmd(TIM2,ENABLE);//使能TIM2
    TIM_Cmd(TIM3,ENABLE);//使能TIM3
    TIM_Cmd(TIM4,ENABLE);//使能TIM4
}


void PWM_SetCompare1(uint16_t Compare)//a0
{
    TIM_SetCompare1(TIM2, Compare);//设置CCR1的值        
}

void PWM_SetCompare2(uint16_t Compare)//a1
{
    TIM_SetCompare2(TIM2, Compare);//设置CCR2的值
}

void PWM_SetCompare3(uint16_t Compare)//a2
{
    TIM_SetCompare3(TIM2, Compare);//设置CCR3的值
}

void PWM_SetCompare4(uint16_t Compare)//A3
{
    TIM_SetCompare4(TIM2, Compare);//设置CCR4的值
}

void PWM_SetCompare5(uint16_t Compare)//a6
{
    TIM_SetCompare1(TIM3, Compare);//设置CCR1的值        
}

void PWM_SetCompare6(uint16_t Compare)//aA7
{
    TIM_SetCompare2(TIM3, Compare);//设置CCR2的值
}

void PWM_SetCompare7(uint16_t Compare)//B0
{
    TIM_SetCompare3(TIM3, Compare);//设置CCR3的值
}

void PWM_SetCompare8(uint16_t Compare)//B1
{
    TIM_SetCompare4(TIM3, Compare);//设置CCR4的值
}

void PWM_SetCompare9(uint16_t Compare)//B6
{
    TIM_SetCompare1(TIM4, Compare);//设置CCR1的值
}

void PWM_SetCompare10(uint16_t Compare)//B7
{
    TIM_SetCompare2(TIM4, Compare);//设置CCR2的值
}

void PWM_SetCompare11(uint16_t Compare)//B8
{
    TIM_SetCompare3(TIM4, Compare);//设置CCR3的值
}

void PWM_SetCompare12(uint16_t Compare)//B9
{
    TIM_SetCompare4(TIM4, Compare);//设置CCR4的值
}