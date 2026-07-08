/**
 * @file wexcube_port.h
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 接口头文件
 * @version 
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __WEXCUBE_PORT_H__
#define __WEXCUBE_PORT_H__

#include "wexcube.h"
#include <stdio.h>
#include <stdarg.h>

/************************ 打印接口 ************************/
#define wex_printf(...)  // 不使用打印功能时可以使用此行
/*********************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void wex_port_init(void);                               // 初始化串口
void wex_port_send(wex_u8_t *pucData, wex_u8_t ucLen);  // 发送数据
void wex_printf(const char *format, ...);               // 格式化输出函数

#ifdef __cplusplus
}
#endif

#endif /* __WEXCUBE_PORT_H__ */