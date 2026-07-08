/**
 * @file wexcube.h
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 头文件
 * @version 2.1.0
 * @date 2025-04-30
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __WEXCUBE_H__
#define __WEXCUBE_H__

#ifdef __cplusplus
extern "C"
{
#endif

// 数据类型定义
typedef signed char wex_s8_t;         //  8 位有符号整数类型
typedef signed short wex_s16_t;       // 16 位有符号整数类型
typedef signed long wex_s32_t;        // 32 位有符号整数类型
typedef signed long long wex_s64_t;   // 64 位有符号整数类型
typedef unsigned char wex_u8_t;       //  8 位无符号整数类型
typedef unsigned short wex_u16_t;     // 16 位无符号整数类型
typedef unsigned long wex_u32_t;      // 32 位无符号整数类型
typedef unsigned long long wex_u64_t; // 64 位无符号整数类型
typedef float wex_f32_t;              // 32 位单精度浮点数类型
typedef double wex_f64_t;             // 64 位双精度浮点数类型

#define WEX_NULL (0)  // 空
#define WEX_OK (0)    // 正确
#define WEX_FAIL (-1) // 错误
#define WEX_TRUE (1)  // 真
#define WEX_FALSE (0) // 假

#define WEX_CMD_BEGIN (0xBE) // WeXCube 指令起始码
#define WEX_CMD_END (0xEDEB) // WeXCube 指令结束码（小端模式，实际传输为 EB ED）
#define WEX_CMD_VER (0x11)   // WeXCube 指令版本号: 1.1

typedef enum
{                                // 设备接收到的指令类型
    eWexCmd_None = 0x00,         // 无指令
    eWexCmd_Connect = 0x81,      // 连接
    eWexCmd_Disconnect = 0x82,   // 断开连接
    eWexCmd_Handshake = 0x83,    // 握手
    eWexCmd_AskHandshake = 0x84, // 请求握手
    eWexCmd_Err = 0x85,          // 错误码
    eWexCmd_Upgrade = 0x86,      // 固件升级
    eWexCmd_Date = 0x91,         // 日期
    eWexCmd_Time = 0x92,         // 时间
    eWexCmd_DeviceName = 0x93,   // 设备名称
    eWexCmd_BluetoothId = 0x94,  // 蓝牙 ID
    eWexCmd_PopupDialog = 0x95,  // 对话框回复
    eWexCmd_Event = 0xA1,        // 控件事件触发
    eWexCmd_Value = 0xA4,        // 控件值
    eWexCmd_Text = 0xA5,         // 控件文本
    eWexCmd_BackRGB = 0xA6,      // 控件背景 RGB 颜色
    eWexCmd_TextRGB = 0xA7,      // 控件文本 RGB 颜色
    eWexCmd_FontSize = 0xA8,     // 控件文本字体大小
} t_eWexCmdType;

typedef enum
{                              // 错误码
    eWexErr_None = 0x00,       // 无错误
    eWexErr_InvCmd = 0x01,     // 无效指令
    eWexErr_InvCtrlId = 0x20,  // 无效控件 ID
    eWexErr_InvCtrlPro = 0x21, // 无效控件属性
    eWexErr_InvValue = 0x22,   // 无效值
    eWexErr_TextOver = 0x23,   // 文本长度溢出
} t_eWexErrCode;

typedef enum
{                           // 按键（Button）状态
    eWexBtn_Release = 0x00, // 按键释放
    eWexBtn_Press = 0x01,   // 按键按下
} t_eWexBtnState;

typedef enum
{                           // 开关（Switch）状态
    eWexSw_Off = 0x00,      // 开关关闭
    eWexSw_On = 0x01,       // 开关打开
} t_eWexSwState;

typedef enum
{                             // 颜色编号
    eWexColor_Black = 0x01,   // 黑色
    eWexColor_Gray1 = 0x02,   // 灰色1
    eWexColor_Gray2 = 0x03,   // 灰色2
    eWexColor_Gray3 = 0x04,   // 灰色3
    eWexColor_White = 0x05,   // 白色
    eWexColor_Skyblue = 0x06, // 天蓝色
    eWexColor_Pink = 0x07,    // 粉红色
    eWexColor_Orange = 0x08,  // 橙色
    eWexColor_Olive = 0x09,   // 橄榄色
    eWexColor_Blue = 0x0A,    // 蓝色
    eWexColor_Yellow = 0x0B,  // 黄色
    eWexColor_Green = 0x0C,   // 绿色
    eWexColor_Magenta = 0x0D, // 紫红色
    eWexColor_Red = 0x0E,     // 红色
    eWexColor_Purple = 0x0F,  // 紫色
    eWexColor_Cyan = 0x10,    // 青色
} t_eWexColor;

typedef enum
{                                   // 弹出框类型
    eWexPopupType_Normal = 0x01,    // 普通弹出框
    eWexPopupType_Success,          // 成功弹出框
    eWexPopupType_Error,            // 错误弹出框
    eWexPopupType_Select,           // 选择对话框
} t_eWexPopupType;

typedef struct
{                            // WeXCube 信息结构体
    wex_u16_t usWexVer;      // WeXCube 版本号
    wex_u8_t ucCmdVer;       // 指令协议版本号
    wex_u8_t ucWexState;     // WeXCube 运行状态, 0:停止, 1:正在运行
    wex_u16_t usRecBufSize;  // WeXCube 接收缓冲区大小
    wex_u16_t usTrsBufSize;  // WeXCube 发送缓冲区大小
    wex_u16_t usTrsMaxOnce;  // WeXCube 一次最多发送字节数
    wex_u8_t ucDebugEnable;  // WeXCube 调试使能, 0:禁用, 1:使能
    wex_u8_t ucConnectState; // 连接状态, 0:未连接, 1:已连接
    wex_u32_t ulTrsCmdCount; // 用户发送的指令条数
    wex_u32_t ulRecCmdCount; // 用户接收的指令条数
} t_sWexInfo;

typedef struct
{                           // WeXCube 指令结构体
    t_eWexCmdType eCmdType; // 指令类型
    wex_u8_t ucCtrlId;      // 控件 ID
    wex_u8_t ucErrCode;     // 错误码
    wex_u8_t ucValue;       // 值
    struct
    {
        wex_u8_t ucR; // 红色
        wex_u8_t ucG; // 绿色
        wex_u8_t ucB; // 蓝色
    } sColor;
    struct
    {
        wex_u8_t ucYear;  // 年
        wex_u8_t ucMonth; // 月
        wex_u8_t ucDay;   // 日
    } sDate;
    struct
    {
        wex_u8_t ucHour;   // 小时
        wex_u8_t ucMinute; // 分钟
        wex_u8_t ucSecond; // 秒钟
    } sTime;
    wex_u8_t ucFontSize;    // 文本大小
    char *pcText;           // 文本
    char *pcDeviceName;     // 设备名称
    char *pcBluetoothId;    // 蓝牙 ID
    struct
    {
        wex_u16_t usPackNo;    // 包号
        wex_u8_t ucPackSize;   // 包大小
        wex_u8_t *pucPackData; // 包数据
    } sUpgrade;
    wex_u8_t pucData[70]; // 数据缓存区
} t_sWexCmd;

// 系统操作
void wex_init(void);                              // 初始化 WeXCube
void wex_reset(void);                             // 重置 WeXCube
void wex_start(void);                             // 开启 WeXCube
void wex_stop(void);                              // 停止 WeXCube
void wex_push(wex_u8_t *pucData, wex_u8_t ucLen); // 向 WeXCube 推送数据

// 系统信息
const t_sWexInfo *wex_getInfo(void); // 获取 WeXCube 信息
void wex_printInfo(void);            // 打印 WeXCube 信息
wex_u8_t wex_getWexState(void);      // 获取 WeXCube 运行状态
wex_u8_t wex_getConnectState(void);  // 获取连接状态
wex_u32_t wex_geTrsCmdCount(void);   // 获取用户发送的指令条数
wex_u32_t wex_getRecCmdCount(void);  // 获取用户接收的指令条数

const t_sWexCmd *wex_process(void); // WeXCube 数据处理函数，需要在主循环中调用

// 系统通信
void wex_askHandshake(void);                // 请求握手
void wex_sendDisconnect(void);              // 发送断开连接
void wex_askErr(void);                      // 请求错误码
void wex_askUpradePack(wex_u16_t usPackNo); // 请求升级数据包
void wex_askDate(void);                     // 请求日期
void wex_askTime(void);                     // 请求时间
void wex_askDeviceName(void);               // 请求设备名称
void wex_askBluetoothId(void);              // 请求蓝牙 ID
void wex_showPopup(t_eWexPopupType eType, const char *pcMessage); // 显示弹出框

// 请求控件属性
void wex_askValue(wex_u8_t ucCtrlId);    // 请求控件值
void wex_askText(wex_u8_t ucCtrlId);     // 请求控件文本
void wex_askBackRGB(wex_u8_t ucCtrlId);  // 请求控件背景 RGB 颜色值
void wex_askTextRGB(wex_u8_t ucCtrlId);  // 请求控件文本 RGB 颜色值
void wex_askFontSize(wex_u8_t ucCtrlId); // 请求控件文本字体大小

// 设置控件属性
void wex_setValue(wex_u8_t ucCtrlId, wex_u8_t ucValue);                           // 设置控件值
void wex_setText(wex_u8_t ucCtrlId, const char *pcText);                          // 设置控件文本
void wex_setBackColor(wex_u8_t ucCtrlId, t_eWexColor eColor);                     // 设置控件背景颜色种类
void wex_setTextColor(wex_u8_t ucCtrlId, t_eWexColor eColor);                     // 设置控件文本颜色种类
void wex_setBackRGB(wex_u8_t ucCtrlId, wex_u8_t ucR, wex_u8_t ucG, wex_u8_t ucB); // 设置控件背景 RGB 颜色值
void wex_setTextRGB(wex_u8_t ucCtrlId, wex_u8_t ucR, wex_u8_t ucG, wex_u8_t ucB); // 设置控件文本 RGB 颜色值
void wex_setFontSize(wex_u8_t ucCtrlId, wex_u8_t ucSize);                         // 设置控件文本字体大小
void wex_sendLineChart(wex_u8_t ucCtrlId, wex_u8_t ucSize, const wex_f32_t* pfArr);// 发送折线图显示点，点为单精度浮点数且为小端模式
void wex_clearLineChart(wex_u8_t ucCtrlId);                                       // 清空折线图显示

// 数据转换
wex_s32_t wex_strToInt(const char *pcStr);                             // 将字符串转换为有符号整数
wex_u32_t wex_strToUint(const char *pcStr);                            // 将字符串转换为无符号整数
const char *wex_intToStr(wex_s32_t slInt);                             // 将有符号整数转换为字符串
const char *wex_uintToStr(wex_u32_t ulUint);                           // 将无符号整数转换为字符串
wex_f32_t wex_strToFloat(const char *pcStr);                           // 将字符串转换为单精度浮点数
const char *wex_floatToStr(wex_f32_t fFloat, wex_u8_t ucPrecision);    // 将单精度浮点数转换为字符串
wex_f64_t wex_strToDouble(const char *pcStr);                          // 将字符串转换为双精度浮点数
const char *wex_doubleToStr(wex_f64_t dDouble, wex_u8_t ucPrecision);  // 将双精度浮点数转换为字符串

#ifdef __cplusplus
}
#endif

#endif /* __WEXCUBE_H__ */
