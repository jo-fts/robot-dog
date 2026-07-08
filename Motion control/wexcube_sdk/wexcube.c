/**
 * @file wexcube.c
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 文件
 * @version 2.1.0
 * @date 2025-04-30
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "wexcube_config.h"
#include "wexcube_port.h"
#include "wexcube.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define WEXCUBE_VER 0x0210 // WeXCube 版本号：2.1.0

static wex_u8_t _wexRecCmdBuf[WEX_REC_BUF_SIZE] = {0}; // 接收缓冲区
static wex_u8_t _wexTrsCmdBuf[WEX_TRS_BUF_SIZE] = {0}; // 发送缓冲区
static wex_u8_t _wexRecItemBuf[80] = {0};              // 接收子项缓冲区
static wex_u8_t _wexTrsItemBuf[80] = {0};              // 发送子项缓冲区
static char _wexStrBuf[70] = {0};                      // 字符串缓冲区

typedef enum
{                                   // 设备发送的指令类型
    eWexDevCmd_None = 0x00,         // 无效指令
    eWexDevCmd_ConnectAck = 0x01,   // 确定连接
    eWexDevCmd_Disconnect = 0x02,   // 断开连接
    eWexDevCmd_Handshake = 0x03,    // 握手
    eWexDevCmd_AskHandshake = 0x04, // 请求握手回复
    eWexDevCmd_AskErr = 0x05,       // 请求错误码
    eWexDevCmd_AskUpgrade = 0x06,   // 请求升级包
    eWexDevCmd_AskDate = 0x11,      // 请求日期
    eWexDevCmd_AskTime = 0x12,      // 请求时间
    eWexDevCmd_AskDeviceName = 0x13,// 请求设备名称
    eWexDevCmd_AskBluetoothId = 0x14,// 请求蓝牙 ID
    eWexDevCmd_ShowPopup = 0x15,    // 显示弹出框
    eWexDevCmd_AskValue = 0x24,     // 请求控件值
    eWexDevCmd_AskText = 0x25,      // 请求控件文本
    eWexDevCmd_AskBackRGB = 0x26,   // 请求控件背景 RGB 颜色
    eWexDevCmd_AskTextRGB = 0x27,   // 请求控件文本 RGB 颜色
    eWexDevCmd_AskFontSize = 0x28,  // 请求控件文本大小
    eWexDevCmd_SetValue = 0x54,     // 设置控件值
    eWexDevCmd_SetText = 0x55,      // 设置控件文本
    eWexDevCmd_SetBackRGB = 0x56,   // 设置控件背景 RGB 颜色
    eWexDevCmd_SetTextRGB = 0x57,   // 设置控件文本 RGB 颜色
    eWexDevCmd_SetFontSize = 0x58,  // 设置控件文本字体大小
    eWexDevCmd_SendLineChart = 0x59,// 发送折线图显示点
    eWexDevCmd_ClearLineChart = 0x5A,// 清空折线图显示
} t_eWexDevCmdType;

typedef struct
{
    wex_u16_t usHead;
    wex_u16_t usTail;
    wex_u8_t *pucBuf;
} t_sWexBuf;
static t_sWexBuf _sWexRecBuf = {0, 0, _wexRecCmdBuf};
static t_sWexBuf _sWexTrsBuf = {0, 0, _wexTrsCmdBuf};

#pragma pack(1)

typedef struct
{
    wex_u8_t ucBegin; // 起始标志
    wex_u8_t ucVer;   // 协议版本
    wex_u8_t ucLen;   // 指令长度
    wex_u8_t ucCmd;   // 指令类型
    wex_u16_t usEnd;  // 结束标志
} t_sWexCmdBasic;

typedef struct
{
    wex_u8_t ucBegin;  // 起始标志
    wex_u8_t ucVer;    // 协议版本
    wex_u8_t ucLen;    // 指令长度
    wex_u8_t ucCmd;    // 指令类型
    wex_u8_t ucCtrlId; // 控件 ID
    wex_u16_t usEnd;   // 结束标志
} t_sWexCmdStruct1;

typedef struct
{
    wex_u8_t ucBegin;  // 起始标志
    wex_u8_t ucVer;    // 协议版本
    wex_u8_t ucLen;    // 指令长度
    wex_u8_t ucCmd;    // 指令类型
    wex_u8_t ucCtrlId; // 控件 ID
    wex_u8_t ucValue;  // 控件值
    wex_u16_t usEnd;   // 结束标志
} t_sWexCmdStruct2;

typedef struct
{
    wex_u8_t ucBegin;  // 起始标志
    wex_u8_t ucVer;    // 协议版本
    wex_u8_t ucLen;    // 指令长度
    wex_u8_t ucCmd;    // 指令类型
    wex_u8_t ucCtrlId; // 控件 ID
    char pcText[32];   // 控件文本
    wex_u16_t usEnd;   // 结束标志
} t_sWexCmdStruct3;

typedef struct
{
    wex_u8_t ucBegin;  // 起始标志
    wex_u8_t ucVer;    // 协议版本
    wex_u8_t ucLen;    // 指令长度
    wex_u8_t ucCmd;    // 指令类型
    wex_u8_t ucCtrlId; // 控件 ID
    wex_u8_t ucValue1; // 控件值 1
    wex_u8_t ucValue2; // 控件值 2
    wex_u8_t ucValue3; // 控件值 3
    wex_u16_t usEnd;   // 结束标志
} t_sWexCmdStruct4;

#pragma pack()

static void _wex_pop(wex_u8_t *pucData, wex_u8_t ucLen, wex_u8_t ucFlag);
static void _wex_connectAck(void);
static void _wex_sendHandshake(void);
static void _wex_sendBasic(t_eWexDevCmdType eCmdType, wex_u8_t ucFlag);
static void _wex_sendStruct1(t_eWexDevCmdType eCmdType, wex_u8_t ucCtrlId);
static void _wex_sendStruct2(t_eWexDevCmdType eCmdType, wex_u8_t ucCtrlId, wex_u8_t ucValue);
static void _wex_sendStruct3(t_eWexDevCmdType eCmdType, wex_u8_t ucCtrlId, const char *pcText);
static void _wex_sendStruct4(t_eWexDevCmdType eCmdType, wex_u8_t ucCtrlId, wex_u8_t ucValue1, wex_u8_t ucValue2, wex_u8_t ucValue3);

// t_eWexColor 颜色编号对应的 RGB 值
const wex_u8_t _pucWexColorRGB[][3] = {
    {0, 0, 0},       // 占位
    {0, 0, 0},       // 黑色
    {90, 90, 90},    // 灰色1
    {180, 180, 180}, // 灰色2
    {243, 243, 243}, // 灰色3
    {255, 255, 255}, // 白色
    {135, 206, 250}, // 天蓝色
    {255, 192, 203}, // 粉红色
    {255, 128, 0},   // 橙色
    {128, 128, 0},   // 橄榄色
    {0, 122, 255},   // 蓝色
    {255, 255, 0},   // 黄色
    {0, 128, 0},     // 绿色
    {255, 0, 255},   // 紫红色
    {255, 0, 0},     // 红色
    {128, 0, 128},   // 紫色
    {0, 255, 255},   // 青色
};

static t_sWexInfo _sWexInfo; // WeXCube 信息
static t_sWexCmd _wexRecCmd; // 接收指令

/**
 * @brief 初始化 WeXCube
 *
 */
void wex_init(void)
{
    wex_port_init();
    wex_reset();

    // WeXCube 信息初始化
    _sWexInfo.usWexVer = WEXCUBE_VER;
    _sWexInfo.ucCmdVer = WEX_CMD_VER;
    _sWexInfo.ucWexState = 0;
    _sWexInfo.usRecBufSize = WEX_REC_BUF_SIZE;
    _sWexInfo.usTrsBufSize = WEX_TRS_BUF_SIZE;
    _sWexInfo.usTrsMaxOnce = WEX_TRS_MAX_ONCE;
    _sWexInfo.ucDebugEnable = WEX_DEBUG_ENABLE;
    _sWexInfo.ucConnectState = 0;
    _sWexInfo.ulTrsCmdCount = 0;
    _sWexInfo.ulRecCmdCount = 0;

    _wexRecItemBuf[0] = (wex_u8_t)WEX_CMD_BEGIN;
    _wexRecItemBuf[1] = (wex_u8_t)WEX_CMD_VER;
    _wexTrsItemBuf[0] = (wex_u8_t)WEX_CMD_BEGIN;
    _wexTrsItemBuf[1] = (wex_u8_t)WEX_CMD_VER;
}

/**
 * @brief 重置 WeXCube
 *
 */
void wex_reset(void)
{
    // 清空接收缓冲区
    _sWexRecBuf.usHead = 0;
    _sWexRecBuf.usTail = 0;
    memset((void *)_wexRecCmdBuf, 0, WEX_REC_BUF_SIZE);

    // 清空发送缓冲区
    _sWexTrsBuf.usHead = 0;
    _sWexTrsBuf.usTail = 0;
    memset((void *)_wexTrsCmdBuf, 0, WEX_TRS_BUF_SIZE);

    // 清空指令发送数量记录
    _sWexInfo.ulTrsCmdCount = 0;
    _sWexInfo.ulRecCmdCount = 0;

    // 清空指令接收指令信息
    _wexRecCmd.eCmdType = eWexCmd_None;
    _wexRecCmd.ucCtrlId = 0;
    _wexRecCmd.ucErrCode = 0;
    _wexRecCmd.ucValue = 0;
    _wexRecCmd.sColor.ucR = 0;
    _wexRecCmd.sColor.ucG = 0;
    _wexRecCmd.sColor.ucB = 0;
    _wexRecCmd.ucFontSize = 0;
    _wexRecCmd.pcText = (char *)_wexRecCmd.pucData; // 默认指向数据区
    _wexRecCmd.pcDeviceName = (char *)_wexRecCmd.pucData; // 默认指向数据区
    _wexRecCmd.pcBluetoothId = (char *)_wexRecCmd.pucData; // 默认指向数据区
    _wexRecCmd.sUpgrade.usPackNo = 0;
    _wexRecCmd.sUpgrade.ucPackSize = 0;
    _wexRecCmd.sUpgrade.pucPackData = (wex_u8_t *)_wexRecCmd.pucData; // 默认指向数据区
    memset((void *)_wexRecCmd.pcText, 0, sizeof(_wexRecCmd.pucData));
}

/**
 * @brief 开启 WeXCube
 *
 */
void wex_start(void)
{
    _sWexInfo.ucWexState = 1;
#if defined(WEX_DEBUG_ENABLE) && (WEX_DEBUG_ENABLE == 1)
    wex_printf("WeXCube Start\r\n");
#endif
}

/**
 * @brief 停止 WeXCube
 *
 */
void wex_stop(void)
{
    _sWexInfo.ucWexState = 0;
#if defined(WEX_DEBUG_ENABLE) && (WEX_DEBUG_ENABLE == 1)
    wex_printf("WeXCube Stop\r\n");
#endif
}

/**
 * @brief 向 WeXCube 推送数据
 *
 * @param pucData 数据指针
 * @param ucLen 数据长度
 */
void wex_push(wex_u8_t *pucData, wex_u8_t ucLen)
{
    if (!_sWexInfo.ucWexState)
        return;

#if (0) // 一次推送数据较多时使用
    if (_sWexRecBuf.usTail + ucLen < WEX_REC_BUF_SIZE)
    {
        memcpy((void *)&_sWexRecBuf.pucBuf[_sWexRecBuf.usTail], (const void *)pucData, (size_t)ucLen);
        _sWexRecBuf.usTail += ucLen;
    }
    else
    {
        wex_u8_t len = WEX_REC_BUF_SIZE - _sWexRecBuf.usTail;
        memcpy((void *)&_sWexRecBuf.pucBuf[_sWexRecBuf.usTail], (const void *)pucData, (size_t)len);
        memcpy((void *)&_sWexRecBuf.pucBuf[0], (const void *)&pucData[len], (size_t)(ucLen - len));
        _sWexRecBuf.usTail = ucLen - len;
    }
#else // 一次推送数据较少时使用
    for (wex_u8_t i = 0; i < ucLen; i++)
    {
        _sWexRecBuf.pucBuf[_sWexRecBuf.usTail++] = pucData[i];
        _sWexRecBuf.usTail = _sWexRecBuf.usTail % WEX_REC_BUF_SIZE;
    }
#endif
}

/**
 * @brief 获取 WeXCube 信息
 *
 * @param psWexInfo WeXCube 信息结构体指针
 */
const t_sWexInfo *wex_getInfo(void)
{
    return (const t_sWexInfo *)&_sWexInfo;
}

/**
 * @brief 打印 WeXCube 信息
 *
 */
void wex_printInfo(void)
{
    wex_printf("******************* WeXCube Information *******************\r\n");
    wex_printf("    WeXCube Version         : %d.%d.%d\r\n", (_sWexInfo.usWexVer >> 8) & 0x000F, (_sWexInfo.usWexVer >> 4) & 0x000F, _sWexInfo.usWexVer & 0x000F);
    wex_printf("    Command Version         : %d.%d\r\n", (_sWexInfo.ucCmdVer >> 4) & 0x0F, _sWexInfo.usWexVer & 0x0F);
    wex_printf("    WeXCube State           : %s\r\n", _sWexInfo.ucWexState ? "Running" : "Stop");
    wex_printf("    Receive Buffer Size     : %d\r\n", _sWexInfo.usRecBufSize);
    wex_printf("    Transmit Buffer Size    : %d\r\n", _sWexInfo.usTrsBufSize);
    wex_printf("    Max Transmit At Once    : %d\r\n", _sWexInfo.usTrsMaxOnce);
    wex_printf("    Debug Enable State      : %s\r\n", _sWexInfo.ucDebugEnable ? "Enable" : "Disable");
    wex_printf("    Connect State           : %s\r\n", _sWexInfo.ucConnectState ? "Connected" : "Disconnect");
    wex_printf("    Transmit Command Count  : %d\r\n", _sWexInfo.ulTrsCmdCount);
    wex_printf("    Receive Command Count   : %d\r\n", _sWexInfo.ulRecCmdCount);
    wex_printf("***********************************************************\r\n");
}

/**
 * @brief 获取 WeXCube 运行状态
 *
 * @return wex_u8_t 运行状态 0: 停止 1: 正在运行
 */
wex_u8_t wex_getWexState(void)
{
    return _sWexInfo.ucWexState;
}

/**
 * @brief 获取连接状态
 *
 * @return wex_u8_t 连接状态 0: 未连接 1: 已连接
 */
wex_u8_t wex_getConnectState(void)
{
    return _sWexInfo.ucConnectState;
}

/**
 * @brief 获取用户发送的指令条数
 *
 * @return wex_u32_t 发送的指令条数
 */
wex_u32_t wex_geTrsCmdCount(void)
{
    return _sWexInfo.ulTrsCmdCount;
}

/**
 * @brief 获取用户接收的指令条数
 *
 * @return wex_u32_t 接收的指令条数
 */
wex_u32_t wex_getRecCmdCount(void)
{
    return _sWexInfo.ulRecCmdCount;
}

/**
 * @brief WeXCube 数据处理函数，需要在主循环中调用
 *
 * @return const t_sWexCmd 指令结构体
 */
const t_sWexCmd *wex_process(void)
{
    _wexRecCmd.eCmdType = eWexCmd_None;

    if (!_sWexInfo.ucWexState)
        goto WEX_PROCESS_FINISH;

    // 处理接收数据
    while (_sWexRecBuf.usHead != _sWexRecBuf.usTail)
    {
        wex_u16_t head = 0;
        wex_u16_t recLen = _sWexRecBuf.usTail > _sWexRecBuf.usHead ? _sWexRecBuf.usTail - _sWexRecBuf.usHead : WEX_REC_BUF_SIZE - _sWexRecBuf.usHead + _sWexRecBuf.usTail;

        // 接收数据长度不足则退出
        if (recLen < sizeof(t_sWexCmdBasic))
            break;

        head = _sWexRecBuf.usHead;
        // 判断是否为开始标志
        if (_sWexRecBuf.pucBuf[head++] == WEX_CMD_BEGIN)
        {
            head = head % WEX_REC_BUF_SIZE;

            // 判断协议版本
            if ((_sWexRecBuf.pucBuf[head] & 0xF0) == (WEX_CMD_VER & 0xF0))
            {
                wex_u8_t len = 0;

                head = (head + 1) % WEX_REC_BUF_SIZE;
                len = _sWexRecBuf.pucBuf[head];

                if ((len >= sizeof(t_sWexCmdBasic)) && (len <= sizeof(_wexRecItemBuf)))
                {
                    wex_u8_t n = len - 3;
                    wex_u16_t head1 = 0;

                    // 如果指令未接收完则退出
                    if (len > recLen)
                        break;

                    head1 = (head + 1) % WEX_REC_BUF_SIZE;
                    _wexRecItemBuf[2] = len;
                    for (wex_u8_t i = 0; i < n; i++)
                    {
                        _wexRecItemBuf[i + 3] = _sWexRecBuf.pucBuf[head1];
                        head1 = (head1 + 1) % WEX_REC_BUF_SIZE;
                    }

                    // 判断是否为结束标志
                    if ((_wexRecItemBuf[len - 1] == ((WEX_CMD_END & 0xFF00) >> 8)) && (_wexRecItemBuf[len - 2] == (WEX_CMD_END & 0x00FF)))
                    {
                        // 赋值
                        _wexRecCmd.eCmdType = (t_eWexCmdType)_wexRecItemBuf[3];

                        // 等待连接指令，升级指令除外
                        if ((!_sWexInfo.ucConnectState) && (_wexRecCmd.eCmdType != eWexCmd_Connect) && (_wexRecCmd.eCmdType != eWexCmd_Upgrade))
                        {
                            _wexRecCmd.eCmdType = eWexCmd_None;
                        }
                        else
                        {
#if defined(WEX_DEBUG_ENABLE) && (WEX_DEBUG_ENABLE == 1)
                            // 请求握手指令，不对外抛出该指令
                            if (_wexRecCmd.eCmdType != eWexCmd_AskHandshake)
                            {
                                wex_printf("WeXCube Rec Cmd: ");
                                for (wex_u8_t i = 0; i < len; i++)
                                    wex_printf("%02X ", _wexRecItemBuf[i]);
                                wex_printf("\r\n");
                            }
#endif
                            // 破坏已读指令，防止被再次读取
                            if (head1 == 0)
                                _sWexRecBuf.pucBuf[WEX_REC_BUF_SIZE - 1] = 0;
                            else
                                _sWexRecBuf.pucBuf[head1 - 1] = 0;

                            // 请求握手指令，不对外抛出该指令
                            if (_wexRecCmd.eCmdType != eWexCmd_AskHandshake)
                                _sWexInfo.ulRecCmdCount++;
                        }
                        head = head1;
                                                
                        switch (_wexRecCmd.eCmdType)
                        {
                        case eWexCmd_Connect: // 连接指令
                        {
                            _sWexInfo.ucConnectState = 1;
                            _wex_connectAck();
#if defined(WEX_DEBUG_ENABLE) && (WEX_DEBUG_ENABLE == 1)
                            wex_printf("WeXCube Connected\r\n");
#endif
                        }
                        break;

                        case eWexCmd_Disconnect: // 断开连接指令
                        {
                            _sWexInfo.ucConnectState = 0;
#if defined(WEX_DEBUG_ENABLE) && (WEX_DEBUG_ENABLE == 1)
                            wex_printf("WeXCube Disconnect\r\n");
#endif
                        }
                        break;

                        case eWexCmd_AskHandshake: // 请求握手指令
                        {
                            _wex_sendHandshake();
                        }
                        break;

                        case eWexCmd_Err: // 错误码指令
                        {
                            _wexRecCmd.ucCtrlId = _wexRecItemBuf[4];
                            _wexRecCmd.ucErrCode = _wexRecItemBuf[5];
                        }
                        break;

                        case eWexCmd_Upgrade: // 升级指令
                        {
                            wex_u8_t check = _wexRecItemBuf[len - 3];
                            wex_u8_t tempCheck = 0;
                            wex_u8_t end = 0;
                            _wexRecCmd.sUpgrade.usPackNo = _wexRecItemBuf[4] << 8 | _wexRecItemBuf[5];
                            _wexRecCmd.sUpgrade.ucPackSize = len - 9;
                                                        end = _wexRecCmd.sUpgrade.ucPackSize + 6;
                            for (wex_u8_t i = 6; i < end; i++)
                                tempCheck += _wexRecItemBuf[i];
                            if (check == tempCheck)
                                memcpy((void *)_wexRecCmd.sUpgrade.pucPackData, (const void *)&_wexRecItemBuf[6], len - 9);
                            else
                                _wexRecCmd.eCmdType = eWexCmd_None;
                        }
                        break;

                        case eWexCmd_Date: // 日期指令
                        {
                            _wexRecCmd.sDate.ucYear = _wexRecItemBuf[4];
                            _wexRecCmd.sDate.ucMonth = _wexRecItemBuf[5];
                            _wexRecCmd.sDate.ucDay = _wexRecItemBuf[6];
                        }
                        break;

                        case eWexCmd_Time: // 时间指令
                        {
                            _wexRecCmd.sTime.ucHour = _wexRecItemBuf[4];
                            _wexRecCmd.sTime.ucMinute = _wexRecItemBuf[5];
                            _wexRecCmd.sTime.ucSecond = _wexRecItemBuf[6];
                        }
                        break;

                        case eWexCmd_DeviceName: // 设备名称指令
                        {
                            memcpy((void *)_wexRecCmd.pcDeviceName, (const void *)&_wexRecItemBuf[4], len - 6);
                            _wexRecCmd.pcDeviceName[len - 6] = '\0';
                        }
                        break;

                        case eWexCmd_BluetoothId: // 蓝牙 ID 指令
                        {
                            memcpy((void *)_wexRecCmd.pcBluetoothId, (const void *)&_wexRecItemBuf[4], len - 6);
                            _wexRecCmd.pcBluetoothId[len - 6] = '\0';
                        }
                        break;

                        case eWexCmd_PopupDialog: // 对话框回复指令
                        {
                            _wexRecCmd.ucValue = _wexRecItemBuf[4];
                        }
                        break;

                        case eWexCmd_Event: // 控件事件触发
                        {
                            _wexRecCmd.ucCtrlId = _wexRecItemBuf[4];
                            _wexRecCmd.ucValue = _wexRecItemBuf[5];
                        }
                        break;

                        case eWexCmd_Value: // 控件值
                        {
                            _wexRecCmd.ucCtrlId = _wexRecItemBuf[4];
                            _wexRecCmd.ucValue = _wexRecItemBuf[5];
                        }
                        break;

                        case eWexCmd_Text: // 控件文本
                        {
                            _wexRecCmd.ucCtrlId = _wexRecItemBuf[4];
                            memcpy((void *)_wexRecCmd.pcText, (const void *)&_wexRecItemBuf[5], len - 7);
                            _wexRecCmd.pcText[len - 7] = '\0';
                        }
                        break;

                        case eWexCmd_BackRGB: // 控件背景 RGB 颜色
                        case eWexCmd_TextRGB: // 控件文本 RGB 颜色
                        {
                            _wexRecCmd.ucCtrlId = _wexRecItemBuf[4];
                            _wexRecCmd.sColor.ucR = _wexRecItemBuf[5];
                            _wexRecCmd.sColor.ucG = _wexRecItemBuf[6];
                            _wexRecCmd.sColor.ucB = _wexRecItemBuf[7];
                        }
                        break;

                        case eWexCmd_FontSize: // 控件文本字体大小
                        {
                            _wexRecCmd.ucCtrlId = _wexRecItemBuf[4];
                            _wexRecCmd.ucFontSize = _wexRecItemBuf[5];
                        }
                        break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
        _sWexRecBuf.usHead = head % WEX_REC_BUF_SIZE;

        // 每次只处理一条指令
        if (_wexRecCmd.eCmdType != eWexCmd_None)
            break;
    }

    // 处理发送数据
    if (_sWexTrsBuf.usHead != _sWexTrsBuf.usTail)
    {
        if (_sWexTrsBuf.usHead < _sWexTrsBuf.usTail)
        {
            if (_sWexTrsBuf.usTail - _sWexTrsBuf.usHead < WEX_TRS_MAX_ONCE)
            {
                wex_port_send(&_sWexTrsBuf.pucBuf[_sWexTrsBuf.usHead], _sWexTrsBuf.usTail - _sWexTrsBuf.usHead);
                _sWexTrsBuf.usHead = _sWexTrsBuf.usTail;
            }
            else
            {
                wex_port_send(&_sWexTrsBuf.pucBuf[_sWexTrsBuf.usHead], WEX_TRS_MAX_ONCE);
                _sWexTrsBuf.usHead += WEX_TRS_MAX_ONCE;
            }
        }
        else
        {
            if (WEX_TRS_BUF_SIZE - _sWexTrsBuf.usHead > WEX_TRS_MAX_ONCE)
            {
                wex_port_send(&_sWexTrsBuf.pucBuf[_sWexTrsBuf.usHead], WEX_TRS_MAX_ONCE);
                _sWexTrsBuf.usHead += WEX_TRS_MAX_ONCE;
            }
            else
            {
                wex_port_send(&_sWexTrsBuf.pucBuf[_sWexTrsBuf.usHead], WEX_TRS_BUF_SIZE - _sWexTrsBuf.usHead);
                _sWexTrsBuf.usHead = 0;
            }
        }
    }

WEX_PROCESS_FINISH:
    return (const t_sWexCmd *)(&_wexRecCmd);
}

/**
 * @brief 把发送数据推入发送缓冲区
 *
 * @param pucData 发送数据
 * @param ucLen 发送数据长度
 * @param ucFlag 指令发出身份标志, 0: WeXCube 自身发出, 1: 用户发出
 */
static void _wex_pop(wex_u8_t *pucData, wex_u8_t ucLen, wex_u8_t ucFlag)
{
    // 升级数据发送不需要WeXCube连接
    if (!_sWexInfo.ucWexState && pucData[3] != eWexDevCmd_AskUpgrade)
        return;

    if (_sWexTrsBuf.usTail + ucLen < WEX_TRS_BUF_SIZE)
    {
        memcpy((void *)&_sWexTrsBuf.pucBuf[_sWexTrsBuf.usTail], (const void *)pucData, (size_t)ucLen);
        _sWexTrsBuf.usTail += ucLen;
    }
    else
    {
        wex_u8_t len = WEX_TRS_BUF_SIZE - _sWexTrsBuf.usTail;
        memcpy((void *)&_sWexTrsBuf.pucBuf[_sWexTrsBuf.usTail], (const void *)pucData, (size_t)len);
        memcpy((void *)&_sWexTrsBuf.pucBuf[0], (const void *)&pucData[len], (size_t)(ucLen - len));
        _sWexTrsBuf.usTail = ucLen - len;
    }

    if (ucFlag == 1)
        _sWexInfo.ulTrsCmdCount++;
}

/**
 * @brief 发送基础指令
 *
 * @param eCmdType 指令类型
 * @param ucFlag 指令发出身份标志, 0: WeXCube 自身发出, 1: 用户发出
 */
static void _wex_sendBasic(t_eWexDevCmdType eCmdType, wex_u8_t ucFlag)
{
    t_sWexCmdBasic *wexCmd = (t_sWexCmdBasic *)_wexTrsItemBuf;

    if (((wex_u8_t)eCmdType >= (wex_u8_t)0x80) || (eCmdType == 0))
        return;

    wexCmd->ucLen = sizeof(t_sWexCmdBasic);
    wexCmd->ucCmd = eCmdType;
    wexCmd->usEnd = WEX_CMD_END;
    _wex_pop(_wexTrsItemBuf, wexCmd->ucLen, ucFlag);
}

/**
 * @brief 发送指令结构 1
 *
 * @param eCmdType 指令类型
 * @param ucCtrlId 控制 ID
 */
static void _wex_sendStruct1(t_eWexDevCmdType eCmdType, wex_u8_t ucCtrlId)
{
    t_sWexCmdStruct1 *wexCmd = (t_sWexCmdStruct1 *)_wexTrsItemBuf;

    if (((wex_u8_t)eCmdType >= (wex_u8_t)0x80) || (eCmdType == 0))
        return;
    if (ucCtrlId == 0)
        return;

    wexCmd->ucLen = sizeof(t_sWexCmdStruct1);
    wexCmd->ucCmd = eCmdType;
    wexCmd->ucCtrlId = ucCtrlId;
    wexCmd->usEnd = WEX_CMD_END;
    _wex_pop(_wexTrsItemBuf, wexCmd->ucLen, 1);
}

/**
 * @brief 发送指令结构 2
 *
 * @param eCmdType 指令类型
 * @param ucCtrlId 控制 ID
 * @param ucValue 值
 */
static void _wex_sendStruct2(t_eWexDevCmdType eCmdType, wex_u8_t ucCtrlId, wex_u8_t ucValue)
{
    t_sWexCmdStruct2 *wexCmd = (t_sWexCmdStruct2 *)_wexTrsItemBuf;

    if (((wex_u8_t)eCmdType >= (wex_u8_t)0x80) || (eCmdType == 0))
        return;
    if ((ucCtrlId == 0) && (eCmdType != eWexDevCmd_AskUpgrade))
        return;

    wexCmd->ucLen = sizeof(t_sWexCmdStruct2);
    wexCmd->ucCmd = eCmdType;
    wexCmd->ucCtrlId = ucCtrlId;
    wexCmd->ucValue = ucValue;
    wexCmd->usEnd = WEX_CMD_END;
    _wex_pop(_wexTrsItemBuf, wexCmd->ucLen, 1);
}

/**
 * @brief 发送指令结构 3
 *
 * @param eCmdType 指令类型
 * @param ucCtrlId 控制 ID
 * @param pcText 文本，于'\0'结尾
 */
static void _wex_sendStruct3(t_eWexDevCmdType eCmdType, wex_u8_t ucCtrlId, const char *pcText)
{
    t_sWexCmdStruct3 *wexCmd = (t_sWexCmdStruct3 *)_wexTrsItemBuf;
    wex_u32_t textLen = strlen(pcText);

    if (((wex_u8_t)eCmdType >= (wex_u8_t)0x80) || (eCmdType == 0))
        return;
    if (ucCtrlId == 0)
        return;
    if (pcText == WEX_NULL)
        textLen = 0;
    if (textLen > sizeof(wexCmd->pcText))
        textLen = sizeof(wexCmd->pcText);

    wexCmd->ucLen = sizeof(t_sWexCmdStruct3) - sizeof(wexCmd->pcText) + textLen;
    wexCmd->ucCmd = eCmdType;
    wexCmd->ucCtrlId = ucCtrlId;
    memcpy((void *)wexCmd->pcText, pcText, (size_t)textLen);
    _wexTrsItemBuf[wexCmd->ucLen - 2] = (wex_u8_t)(WEX_CMD_END & 0xFF);
    _wexTrsItemBuf[wexCmd->ucLen - 1] = (wex_u8_t)(WEX_CMD_END >> 8);
    _wex_pop(_wexTrsItemBuf, wexCmd->ucLen, 1);
}

/**
 * @brief 发送指令结构 4
 *
 * @param eCmdType 指令类型
 * @param ucCtrlId 控制 ID
 * @param ucValue1 值 1
 * @param ucValue2 值 2
 * @param ucValue3 值 3
 */
static void _wex_sendStruct4(t_eWexDevCmdType eCmdType, wex_u8_t ucCtrlId, wex_u8_t ucValue1, wex_u8_t ucValue2, wex_u8_t ucValue3)
{
    t_sWexCmdStruct4 *wexCmd = (t_sWexCmdStruct4 *)_wexTrsItemBuf;

    if (((wex_u8_t)eCmdType >= (wex_u8_t)0x80) || (eCmdType == 0))
        return;
    if (ucCtrlId == 0)
        return;

    wexCmd->ucLen = sizeof(t_sWexCmdStruct4);
    wexCmd->ucCmd = eCmdType;
    wexCmd->ucCtrlId = ucCtrlId;
    wexCmd->ucValue1 = ucValue1;
    wexCmd->ucValue2 = ucValue2;
    wexCmd->ucValue3 = ucValue3;
    wexCmd->usEnd = WEX_CMD_END;
    _wex_pop(_wexTrsItemBuf, wexCmd->ucLen, 1);
}

/**
 * @brief 确定连接
 *
 */
static void _wex_connectAck(void)
{
    _wex_sendBasic(eWexDevCmd_ConnectAck, 0);
}

/**
 * @brief WeXCube 发送握手
 *
 */
void _wex_sendHandshake(void)
{
    _wex_sendBasic(eWexDevCmd_Handshake, 0);
}

/**
 * @brief 发送断开连接
 *
 */
void wex_sendDisconnect(void)
{
    _wex_sendBasic(eWexDevCmd_Disconnect, 1);
}

/**
 * @brief 请求握手
 *
 */
void wex_askHandshake(void)
{
    _wex_sendBasic(eWexDevCmd_AskHandshake, 1);
}

/**
 * @brief 请求错误码
 *
 */
void wex_askErr(void)
{
    _wex_sendBasic(eWexDevCmd_AskErr, 1);
}

/**
 * @brief 请求升级数据包
 *
 */
void wex_askUpradePack(wex_u16_t usPackNo)
{
    _wex_sendStruct2(eWexDevCmd_AskUpgrade, usPackNo >> 8, usPackNo & 0x00FF);
}

/**
 * @brief 请求日期
 *
 */
void wex_askDate(void)
{
    _wex_sendBasic(eWexDevCmd_AskDate, 1);
}

/**
 * @brief 请求时间
 *
 */
void wex_askTime(void)
{
    _wex_sendBasic(eWexDevCmd_AskTime, 1);
}

/**
 * @brief 请求设备名称
 */
void wex_askDeviceName(void)
{
    _wex_sendBasic(eWexDevCmd_AskDeviceName, 1);
}

/**
 * @brief 请求蓝牙 ID
 */
void wex_askBluetoothId(void)
{
    _wex_sendBasic(eWexDevCmd_AskBluetoothId, 1);
}

/**
 * @brief 显示弹出框
 * 
 * @param eType 弹出框类型
 * @param pcMessage 弹出框消息
 */
void wex_showPopup(t_eWexPopupType eType, const char *pcMessage)
{
    _wex_sendStruct3(eWexDevCmd_ShowPopup, eType, pcMessage);
}

/**
 * @brief 请求控件值
 *
 * @param ucCtrlId 控件 ID
 */
void wex_askValue(wex_u8_t ucCtrlId)
{
    _wex_sendStruct1(eWexDevCmd_AskValue, ucCtrlId);
}

/**
 * @brief 请求控件文本
 *
 * @param ucCtrlId 控件 ID
 */
void wex_askText(wex_u8_t ucCtrlId)
{
    _wex_sendStruct1(eWexDevCmd_AskText, ucCtrlId);
}

/**
 * @brief 请求控件背景 RGB 颜色值
 *
 * @param ucCtrlId 控件 ID
 */
void wex_askBackRGB(wex_u8_t ucCtrlId)
{
    _wex_sendStruct1(eWexDevCmd_AskBackRGB, ucCtrlId);
}

/**
 * @brief 请求控件文本 RGB 颜色值
 *
 * @param ucCtrlId 控件 ID
 */
void wex_askTextRGB(wex_u8_t ucCtrlId)
{
    _wex_sendStruct1(eWexDevCmd_AskTextRGB, ucCtrlId);
}

/**
 * @brief 请求控件文本字体大小
 *
 * @param ucCtrlId 控件 ID
 */
void wex_askFontSize(wex_u8_t ucCtrlId)
{
    _wex_sendStruct1(eWexDevCmd_AskFontSize, ucCtrlId);
}

/**
 * @brief 设置控件值
 *
 * @param ucCtrlId 控件 ID
 * @param ucValue 值
 */
void wex_setValue(wex_u8_t ucCtrlId, wex_u8_t ucValue)
{
    _wex_sendStruct2(eWexDevCmd_SetValue, ucCtrlId, ucValue);
}

/**
 * @brief 设置控件文本
 *
 * @param ucCtrlId 控件 ID
 * @param pcText 文本，于'\0'结尾
 */
void wex_setText(wex_u8_t ucCtrlId, const char *pcText)
{
    _wex_sendStruct3(eWexDevCmd_SetText, ucCtrlId, pcText);
}

/**
 * @brief 设置控件背景颜色种类
 *
 * @param ucCtrlId 控件 ID
 * @param eColor 颜色编号
 */
void wex_setBackColor(wex_u8_t ucCtrlId, t_eWexColor eColor)
{
    _wex_sendStruct4(eWexDevCmd_SetBackRGB, ucCtrlId, _pucWexColorRGB[eColor][0], _pucWexColorRGB[eColor][1], _pucWexColorRGB[eColor][2]);
}

/**
 * @brief 设置控件文本颜色种类
 *
 * @param ucCtrlId 控件 ID
 * @param eColor 颜色编号
 */
void wex_setTextColor(wex_u8_t ucCtrlId, t_eWexColor eColor)
{
    _wex_sendStruct4(eWexDevCmd_SetTextRGB, ucCtrlId, _pucWexColorRGB[eColor][0], _pucWexColorRGB[eColor][1], _pucWexColorRGB[eColor][2]);
}

/**
 * @brief 设置控件背景 RGB 颜色值
 *
 * @param ucCtrlId 控件 ID
 * @param ucR Red 值
 * @param ucG Green 值
 * @param ucB Blue 值
 */
void wex_setBackRGB(wex_u8_t ucCtrlId, wex_u8_t ucR, wex_u8_t ucG, wex_u8_t ucB)
{
    _wex_sendStruct4(eWexDevCmd_SetBackRGB, ucCtrlId, ucR, ucG, ucB);
}

/**
 * @brief 设置控件文本 RGB 颜色值
 *
 * @param ucCtrlId 控件 ID
 * @param ucR Red 值
 * @param ucG Green 值
 * @param ucB Blue 值
 */
void wex_setTextRGB(wex_u8_t ucCtrlId, wex_u8_t ucR, wex_u8_t ucG, wex_u8_t ucB)
{
    _wex_sendStruct4(eWexDevCmd_SetTextRGB, ucCtrlId, ucR, ucG, ucB);
}

/**
 * @brief 设置控件文本字体大小
 *
 * @param ucCtrlId 控件 ID
 * @param ucSize 字体大小
 */
void wex_setFontSize(wex_u8_t ucCtrlId, wex_u8_t ucSize)
{
    _wex_sendStruct2(eWexDevCmd_SetFontSize, ucCtrlId, ucSize);
}

/**
 * @brief 发送折线图显示点
 * 
 * @param ucCtrlId 控件 ID
 * @param ucSize 显示点个数
 * @param pfArr 显示点数组
 */
void wex_sendLineChart(wex_u8_t ucCtrlId, wex_u8_t ucSize, const wex_f32_t* pfArr)
{
    if ((ucCtrlId == 0) || (ucSize == 0) || (pfArr == WEX_NULL))
        return;
    if (ucSize > 15)
        ucSize = 15;

    t_sWexCmdStruct1 *wexCmd = (t_sWexCmdStruct1 *)_wexTrsItemBuf;
    wexCmd->ucLen = sizeof(t_sWexCmdStruct1) + ucSize * 4;
    wexCmd->ucCmd = eWexDevCmd_SendLineChart;
    wexCmd->ucCtrlId = ucCtrlId;
    memcpy((void *)&wexCmd->usEnd, pfArr, ucSize * 4);
    _wexTrsItemBuf[wexCmd->ucLen - 2] = (wex_u8_t)(WEX_CMD_END & 0xFF);
    _wexTrsItemBuf[wexCmd->ucLen - 1] = (wex_u8_t)(WEX_CMD_END >> 8);
    _wex_pop(_wexTrsItemBuf, wexCmd->ucLen, 1);
}

/**
 * @brief 清空折线图显示
 * 
 * @param ucCtrlId 控件 ID
 */
void wex_clearLineChart(wex_u8_t ucCtrlId)
{
    _wex_sendStruct1(eWexDevCmd_ClearLineChart, ucCtrlId);
}

/**
 * @brief 将字符串转换为有符号整数
 *
 * @param pcStr 字符串指针，于'\0'结尾
 * @return wex_s32_t 有符号整数
 */
wex_s32_t wex_strToInt(const char *pcStr)
{
    return strtol(pcStr, NULL, 10);
}

/**
 * @brief 将字符串转换为无符号整数
 *
 * @param pcStr 字符串指针，于'\0'结尾
 * @return wex_u32_t 无符号整数
 */
wex_u32_t wex_strToUint(const char *pcStr)
{
    return strtoul(pcStr, NULL, 10);
}

/**
 * @brief 将有符号整数转换为字符串
 *
 * @param slInt 有符号整数
 * @return const char* 字符串指针，临时变量需要尽快使用
 */
const char *wex_intToStr(wex_s32_t slInt)
{
    sprintf(_wexStrBuf, "%ld", slInt);
    return (const char *)_wexStrBuf;
}

/**
 * @brief 将无符号整数转换为字符串
 *
 * @param ulUint 无符号整数
 * @return const char* 字符串指针，临时变量需要尽快使用
 */
const char *wex_uintToStr(wex_u32_t ulUint)
{
    sprintf(_wexStrBuf, "%lu", ulUint);
    return (const char *)_wexStrBuf;
}

/**
 * @brief 将字符串转换为单精度浮点数
 *
 * @param pcStr 字符串指针，于'\0'结尾
 * @return wex_f32_t 单精度浮点数
 */
wex_f32_t wex_strToFloat(const char *pcStr)
{
    return strtof(pcStr, NULL);
}

/**
 * @brief 将单精度浮点数转换为字符串
 *
 * @param fFloat 单精度浮点数
 * @param ucPrecision 小数点位数，为 255 时不限制小数点位数
 * @return const char* 字符串指针，临时变量需要尽快使用
 */
const char *wex_floatToStr(wex_f32_t fFloat, wex_u8_t ucPrecision)
{
    if (ucPrecision == 255)
        sprintf(_wexStrBuf, "%f", fFloat);
    else
        sprintf(_wexStrBuf, "%.*f", ucPrecision, fFloat);
    return (const char *)_wexStrBuf;
}

/**
 * @brief 将字符串转换为双精度浮点数
 *
 * @param pcStr 字符串指针，于'\0'结尾
 * @return wex_f64_t 双精度浮点数
 */
wex_f64_t wex_strToDouble(const char *pcStr)
{
    return strtod(pcStr, NULL);
}

/**
 * @brief 将双精度浮点数转换为字符串
 *
 * @param dDouble 双精度浮点数
 * @param ucPrecision 小数点位数，为 255 时不限制小数点位数
 * @return const char* 字符串指针，临时变量需要尽快使用
 */
const char *wex_doubleToStr(wex_f64_t dDouble, wex_u8_t ucPrecision)
{
    if (ucPrecision == 255)
        sprintf(_wexStrBuf, "%lf", dDouble);
    else
        sprintf(_wexStrBuf, "%.*lf", ucPrecision, dDouble);
    return (const char *)_wexStrBuf;
}
