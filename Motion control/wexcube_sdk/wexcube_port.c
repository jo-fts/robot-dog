/**
 * @file wexcube_port.c
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 接口文件
 * @version 
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "wexcube_port.h"
#include "wexcube.h"
#include "stm32f10x.h"
#include "BlueTooth.h"
#include "RobotState.h"

// 注意：Action_Mode已移至robot结构体中

/**
 * @brief 串口初始化函数
 * 
 */
void wex_port_init(void)
{
    // 使用USART3作为WeXCube通信接口
    // 初始化已在BlueTooth_Init中完成
}

/**
 * @brief 发送数据
 * 
 * @param pucData 数据指针
 * @param ucLen 数据长度
 */
void wex_port_send(wex_u8_t *pucData, wex_u8_t ucLen)
{
    for (wex_u8_t i = 0; i < ucLen; i++)
    {
        USART_SendData(USART3, pucData[i]);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    }
}

/**
 * @brief 格式化输出函数
 * 
 * @param format 格式字符串
 * @param ... 可变参数
 */
/*
void wex_printf(const char *format, ...)
{
    // 如果需要实现printf功能，可以在这里添加代码
    // 或者保留为空，不使用打印功能
}
*/
/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  */
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
    {
        uint8_t data = USART_ReceiveData(USART3);
        wex_push(&data, 1);
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}