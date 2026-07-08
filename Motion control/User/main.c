#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "BlueTooth.h"
#include "Servo.h"
#include "PetAction.h"
#include "Face_Config.h"
#include "wexcube.h"
#include "RobotState.h"
#include "robot_config.h"
#include "error_handler.h"

// 临时的handle_error函数实现，解决链接错误
void handle_error(ErrorCode_t error)
{
    // 在OLED上显示错误信息
    OLED_Clear();
    OLED_ShowString(0, 0, "ERROR:", OLED_8X16);
    OLED_ShowNum(48, 0, error, 2, OLED_8X16);
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

// 全局变量定义 - 使用配置文件中的参数
RobotState_t robot = {
    .height = DEFAULT_HEIGHT_MM,  // 初始身高
    .action_mode = 0,
    .face_mode = 0,
    .action_high = 1,
    .speed_delay = 10,
    .swing_delay = 50,
    .wei_ba = 0,
    .action_state = ACTION_IDLE,
    .leg_positions = {
        .x = {0, 0, 0, 0},
        .y = {INITIAL_Y_POS, INITIAL_Y_POS, INITIAL_Y_POS, INITIAL_Y_POS},  // 初始y坐标
        .z = {0, 0, 0, 0}
    }
};

// 系统时间计数器
uint32_t g_SystemTime = 0;

// 控件ID定义 - 根据小程序端控件ID设置
#define BUTTON_ID        1  // 设备控制页面按钮的ID
#define SWITCH_ID        2  // 设备控制页面开关的ID
#define TEXT_ID          3  // 设备控制页面文本框的ID
#define SLIDER_ID        4  // 设备控制页面滑动条ID
#define INPUT_ID         5  // 设备控制页面输入框ID
#define LINECHART_ID     6  // 设备控制页面折线图ID

// 动作映射表结构
typedef struct {
    uint8_t ctrl_id;     // 控件ID
    uint8_t face_mode;   // 面部表情模式
    uint8_t action_mode; // 动作模式
} ActionMap_t;

// 动作映射表
const ActionMap_t action_map[] = {
    {1, 0, 0},   // 放松趴下
    {2, 1, 1},   // 停止/立正
    {3, 5, 2},   // 站立
    {4, 1, 3},   // 坐下
    {5, 2, 4},   // 前进
    {6, 2, 5},   // 后退
    {7, 2, 6},   // 左转
    {8, 2, 7},   // 右转
    {9, 4, 8},   // 摇摆
    {10, 4, 9},  // 左前走
    {11, 4, 10}, // 左后走
    {12, 4, 11}, // 右前走
    {13, 4, 12}, // 右后走
    {14, 4, 13}, // 右走
    {15, 4, 14}, // 左走
    {16, 4, 15}, // 测试
    {17, 4, 16}, // 跳舞
    {20, 4, 17}, // 踏步
    {21, 4, 18}, // 撒尿
    {0, 0, 0}    // 结束标记
};

// 动作执行状态变量
ActionState currentActionState = ACTION_IDLE;
uint32_t lastActionTime = 0;
uint32_t actionInterval = ACTION_INTERVAL_MS; // 动作执行间隔(ms)
uint32_t actionStartTime = 0; // 动作开始时间

// 非阻塞式动作执行函数 - 使用大狗的参数
void executeActionNonBlocking(void)
{
    static uint8_t danceStep = 0;
    static uint32_t danceStartTime = 0;
    static uint8_t peeStep = 0;
    static uint32_t peeStartTime = 0;
    
    if (currentActionState == ACTION_IDLE) {
        return;
    }
    
    // 超时检查
    if (currentActionState != ACTION_IDLE && g_SystemTime - actionStartTime > ACTION_TIMEOUT_MS) {
        currentActionState = ACTION_CANCELLED;
        handle_error(ERROR_ACTION_TIMEOUT); // 处理动作超时错误
        return;
    }
    
    if (g_SystemTime - lastActionTime < actionInterval) {
        return;
    }
    
    lastActionTime = g_SystemTime;
    
    switch (robot.action_mode) {
        case 0: // 放松趴下 - 使用大狗参数
            Action_dongzhuo_1(-15, -15, -15, -15, 130, 130, 130, 130, 10, 0);
				//x01 x02 x03 x04 y01 y02 y03 y04 时间 中断
            currentActionState = ACTION_COMPLETED;
            break;
            
        case 1: // 停止/立正 - 使用大狗参数
            Action_dongzhuo_1(-15, -15, -15, -15, robot.height, robot.height, robot.height, robot.height, 10, 1);
            //x01 x02 x03 x04 y01 y02 y03 y04 时间 中断
				currentActionState = ACTION_COMPLETED;
            break;
            
        case 2: // 站立 - 使用大狗参数
            Action_dongzhuo_1(-15, -15, -15, -15, robot.height, robot.height, robot.height, robot.height, 10, 2);
           //x01 x02 x03 x04 y01 y02 y03 y04 时间 中断
				currentActionState = ACTION_COMPLETED;
            break;
            
        case 3: // 坐下 - 使用大狗参数
            Action_dongzhuo_1(-15, -15, -15, -15, 170, 170, 130, 130, 10, 3);
           //x01 x02 x03 x04 y01 y02 y03 y04 时间 中断
				currentActionState = ACTION_COMPLETED;
            break;
            
        case 4: // 前进 - 使用大狗参数
            Action_advance(50, 55, +15, robot.height, +17, 3, 1, 1, 1, 4);
              //步长60、 步高、x轴前后重心（+中心前移）、腿高、 z轴左右重心(+身体中心左移) 、时序 、前进1后退-1 左转-1：1、右转1：1	、中断
				break;
            
        case 5: // 后退 - 使用大狗参数
            Action_advance(50, 55, +15, robot.height, +18.5, 3, -1, 1, 1, 5);
            //步长60、 步高、x轴前后重心（+中心前移）、腿高、 z轴左右重心(+身体中心左移) 、时序 、前进1后退-1 左转-1：1、右转1：1	、中断
				break;
            
        case 6: // 左转 - 使用大狗参数
            Action_advance_yuandizhuan(50, 50, 13, robot.height, 10, 3, 1, 1, 1, 6);
              //步长60、 步高、x轴前后重心（+中心前移）、腿高、 z轴左右重心(+身体中心左移) 、时序 、前进1后退-1 左转1、1：1	、中断
				break;
            
        case 7: // 右转 - 使用大狗参数
            Action_advance_yuandizhuan(50, 50, 13, robot.height, 10, 3, -1, 1, 1, 7);
             //步长60、 步高、x轴前后重心（+中心前移）、腿高、 z轴左右重心(+身体中心左移) 、时序 、前进1后退-1 左转-1、1：1	、中断
				break;
            
        case 8: // 摇摆 - 使用大狗参数
            Action_yaobai(50, 30, 0, robot.height, 11, 25, 1, 1, 1, 8);
             //步长60、 步高、x轴前后重心（+中心前移）、腿高、 z轴左右重心(+身体中心左移) 、时序 、前进1后退-1 左转-1：1、右转1：1	、中断
				break;
            
        case 9: // 左前走 - 使用大狗参数
            Action_advance(50, 55, +15, robot.height, +18.5, 3, 1, 0.7, 1, 9);
             //步长60、 步高、x轴前后重心（+中心前移）、腿高、 z轴左右重心(+身体中心左移) 、时序 、前进1后退-1 左转-1：1、右转1：1	、中断
				break;
            
        case 10: // 左后走 - 使用大狗参数
            Action_advance(50, 55, +15, robot.height, +18.5, 3, -1, 0.7, 1, 10);
             //步长60、 步高、x轴前后重心（+中心前移）、腿高、 z轴左右重心(+身体中心左移) 、时序 、前进1后退-1 左转-1：1、右转1：1	、中断
				break;
            
        case 11: // 右前走 - 使用大狗参数
            Action_advance(50, 55, +15, robot.height, +18.5, 3, 1, 1, 0.7, 11);
           //步长60、 步高、x轴前后重心（+中心前移）、腿高、 z轴左右重心(+身体中心左移) 、时序 、前进1后退-1 左转-1：1、右转1：1	、中断
				break;
            
        case 12: // 右后走 - 使用大狗参数
            Action_advance(50, 55, +15, robot.height, +18.5, 3, -1, 1, 0.7, 12);
            break;
            
        case 13: // 右走 - 使用大狗参数
            Action_advance_zuoyou(40, 55, +15, 190, +18.5, 4,1,1, 1, 13);
           ////步长 抬腿高度 侧向重心（—是向左移重心+是向右移重心） 初始高度 前后重心（-向前移重心+向后移重心） 步间延时 
				
				break;
            
        case 14: // 左走 - 使用大狗参数
            Action_advance_zuoyou(35, 55,+18.5, 190,+15, 4, -1, 1, 1, 14);
				   ////步长 抬腿高度 侧向重心（—是向左移重心+是向右移重心） 初始高度 前后重心（-向前移重心+向后移重心） 步间延时 
            break;
            
        case 15: // 测试
            Action_ceshi();
            currentActionState = ACTION_COMPLETED;
            break;
            
        case 16: // 跳舞 - 使用大狗参数
            if (danceStartTime == 0) 
							{
                danceStartTime = g_SystemTime;
                danceStep = 0;
            }
            
            // 跳舞序列 - 使用大狗坐标
            if (danceStep == 0 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(
							20, 20, 20, 20, //x1 x2 x3 x4
							robot.height, robot.height, robot.height, robot.height, //y1 y2 y3 y4
							0, 0, 0, 0, 2, 16);//z1 z2 z3 z4
             
				        danceStep = 1;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 1 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(20, 20, 20, 20, 130, robot.height, robot.height, robot.height, 30, 0, 0, 0, 1, 16);
                danceStep = 2;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 2 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(20, 20, 20, 20, robot.height, robot.height, robot.height, robot.height, 0, 0, 0, 0, 1, 16);
                danceStep = 3;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 3 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(20, 20, 20, 20, robot.height, 130, robot.height, robot.height, 0, 30, 0, 0, 1, 16);
                danceStep = 4;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 4 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(20, 20, 20, 20, robot.height, robot.height, robot.height, robot.height, 0, 0, 0, 0, 1, 16);
                danceStep = 5;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 5 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(-30, -30, -30, -30, robot.height, robot.height, robot.height, robot.height, 0, 0, 0, 0, 2, 16);
                danceStep = 6;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 6 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(-30, -30, -30, -30, robot.height, robot.height, 130, robot.height, 0, 0, -30, 0, 2, 16);
                danceStep = 7;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 7 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(-30, -30, -30, -30, robot.height, robot.height, robot.height, robot.height, 0, 0, 0, 0, 1, 16);
                danceStep = 8;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 8 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(-30, -30, -30, -30, robot.height, robot.height, robot.height, 130, 0, 0, 0, -30, 1, 16);
                danceStep = 9;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 9 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(-30, -30, -30, -30, robot.height, robot.height, robot.height, robot.height, 0, 0, 0, 0, 1, 16);
                danceStep = 10;
                danceStartTime = g_SystemTime;
            } else if (danceStep == 10 && g_SystemTime - danceStartTime >= 1000) {
                Action_dongzhuo_2(0, 0, 0, 0, robot.height, robot.height, robot.height, robot.height, 0, 0, 0, 0, 2, 16);
                danceStep = 0;
                danceStartTime = g_SystemTime;
            } 
            break;
            
						case 17: // 踏步 - 使用大狗参数
            Action_advance(0, 55, +15, robot.height, +35, 4, 1, 1, 1, 17);
              //步长0（原地踏步）、 步高、x轴前后重心（+中心前移）、腿高、 z轴左右重心(+身体中心左移) 、时序 、前进1后退-1 左转-1：1、右转1：1	、中断
		
						break;
						
						case 18: // 撒尿 - 使用大狗参数
            if (peeStartTime == 0) 
						{
                peeStartTime = g_SystemTime;
                peeStep = 0;
            }
            
            // 撒尿序列 - 使用大狗坐标
            if (peeStep == 0 && g_SystemTime - peeStartTime >= 1000) {
                // 准备姿势：后腿稍微弯曲
                Action_dongzhuo_2(-15, -15, -15, -15, robot.height, robot.height, robot.height, robot.height, 0, 0, 0, 0, 2, 18);
                peeStep = 1;
                peeStartTime = g_SystemTime;
            } else if (peeStep == 1 && g_SystemTime - peeStartTime >= 1000) {
                // 开始撒尿：后腿弯曲，身体前倾
                Action_dongzhuo_2(-20, -20, -20, -20, robot.height, robot.height, robot.height, robot.height, 0, 0, 0, 0, 1, 18);
                peeStep = 2;
                peeStartTime = g_SystemTime;
            } else if (peeStep == 2 && g_SystemTime - peeStartTime >= 2000) {
                // 撒尿中：保持姿势
                Action_dongzhuo_2(-45, -45, -45, -45, robot.height, robot.height, robot.height, 110, 0, 0, 0, 60, 1, 18);
                peeStep = 3;
                peeStartTime = g_SystemTime;
            } else if (peeStep == 3 && g_SystemTime - peeStartTime >= 2000) {
                // 撒尿中：稍微调整姿势
                Action_dongzhuo_2(-40, -40, -40, -40, robot.height, robot.height,robot.height, 115, 0, 0, 0, 55, 1, 18);
                peeStep = 4;
                peeStartTime = g_SystemTime;
            } else if (peeStep == 4 && g_SystemTime - peeStartTime >= 2000) {
                // 结束撒尿：恢复站姿
                Action_dongzhuo_2(-15, -15, -15, -15, robot.height, robot.height, robot.height, robot.height, 0, 0, 0, 0, 2, 18);
                peeStep = 0;
                peeStartTime = g_SystemTime;
            }
			break;
						
        default:
            currentActionState = ACTION_COMPLETED;
            break;
    }
}

int main(void)
{
    // 初始化各个模块
    Servo_Init();
    
    // 开机直接站立
    robot.action_mode = 17;
    currentActionState = ACTION_RUNNING;
    actionStartTime = g_SystemTime; // 记录动作开始时间
    currentActionState = ACTION_COMPLETED;
    while(1)
    {    
        executeActionNonBlocking();
        
        // 更新系统时间
        g_SystemTime += 5;
        
        // 短暂延时，防止CPU过载
        Delay_ms(8);
    }
}