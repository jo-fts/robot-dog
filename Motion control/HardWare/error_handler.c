#include "error_handler.h"
#include "OLED.h"
#include "RobotState.h"
#include "Servo.h"

// 错误描述数组
static const char* error_descriptions[] = {
    "No error",
    "Servo out of range",
    "Action timeout",
    "Bluetooth disconnected",
    "Low battery",
    "Overload",
    "USART communication error",
    "Invalid command"
};

/**
 * @brief 获取错误描述
 * @param error 错误码
 * @return 错误描述字符串
 */
const char* get_error_description(ErrorCode_t error)
{
    if (error >= ERROR_NONE && error < ERROR_MAX) {
        return error_descriptions[error];
    }
    return "Unknown error";
}

/**
 * @brief 错误处理函数
 * @param error 错误码
 */
void handle_error(ErrorCode_t error)
{
    // 在OLED上显示错误信息
    OLED_Clear();
    OLED_ShowString(0, 0, "ERROR:", OLED_8X16);
    OLED_ShowNum(48, 0, error, 2, OLED_8X16);
    OLED_ShowString(0, 2, get_error_description(error), OLED_8X16);
    OLED_Update();
    
    // 安全停止所有动作
    robot.action_mode = 1; // 回到停止状态
    robot.action_state = ACTION_IDLE;
    
    // 设置舵机到安全位置（中间位置）
    for(int i=1; i<=12; i++) {
        switch(i) {
            case 1: Servo_Angle1(90); break;
            case 2: Servo_Angle2(90); break;
            case 3: Servo_Angle3(90); break;
            case 4: Servo_Angle4(90); break;
            case 5: Servo_Angle5(90); break;
            case 6: Servo_Angle6(90); break;
            case 7: Servo_Angle7(90); break;
            case 8: Servo_Angle8(90); break;
            case 9: Servo_Angle9(90); break;
            case 10: Servo_Angle10(90); break;
            case 11: Servo_Angle11(90); break;
            case 12: Servo_Angle12(90); break;
        }
    }
}
