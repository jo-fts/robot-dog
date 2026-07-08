#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

// 错误码定义
typedef enum {
    ERROR_NONE = 0,           // 无错误
    ERROR_SERVO_OUT_OF_RANGE, // 舵机角度超出范围
    ERROR_ACTION_TIMEOUT,     // 动作执行超时
    ERROR_BLUETOOTH_DISCONNECTED, // 蓝牙断开连接
    ERROR_LOW_BATTERY,        // 电池电量低
    ERROR_OVERLOAD,           // 过载
    ERROR_USART_COMMUNICATION, // 串口通信错误
    ERROR_INVALID_COMMAND,     // 无效命令
    ERROR_MAX                 // 错误码最大值
} ErrorCode_t;

// 错误处理函数
void handle_error(ErrorCode_t error);

// 获取错误描述
const char* get_error_description(ErrorCode_t error);

#endif // ERROR_HANDLER_H
