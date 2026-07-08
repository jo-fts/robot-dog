/**
 * @file wexcube_config.h
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 配置头文件
 * @version 
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __WEXCUBE_CONFIG_H__
#define __WEXCUBE_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define WEX_DEBUG_ENABLE            0           // wexcube 打印调试开关

#define WEX_REC_BUF_SIZE            256         // wexcube 接收缓存大小，最小 64，最大 4095
#define WEX_TRS_BUF_SIZE            512         // wexcube 发送缓存大小，最小 128，最大 4095
#define WEX_TRS_MAX_ONCE            20          // wexcube 一次发送最大字节数，最小 1，最大 255，如果使用的是单片机内部 BLE 推荐设置为 20

#ifdef __cplusplus
}
#endif

#endif /* __WEXCUBE_CONFIG_H__ */