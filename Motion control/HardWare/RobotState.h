#ifndef ROBOT_STATE_H
#define ROBOT_STATE_H

// 动作状态枚举
typedef enum {
    ACTION_IDLE,
    ACTION_STARTING,
    ACTION_RUNNING,
    ACTION_COMPLETING,
    ACTION_COMPLETED,
    ACTION_CANCELLED
} ActionState;

// 全局数据结构体
typedef struct {
    float height;             // 身高参数
    uint16_t action_mode;     // 动作模式
    uint16_t face_mode;       // 面部表情模式
    uint16_t action_high;     // 动作高度
    uint16_t speed_delay;     // 速度延迟
    uint16_t swing_delay;     // 摇摆延迟
    uint8_t wei_ba;           // 尾巴状态
    ActionState action_state; // 动作执行状态
    
    // 坐标变量
    struct {
        float x[4];
        float y[4];
        float z[4];
    } leg_positions;
} RobotState_t;

// 声明全局变量
extern RobotState_t robot;

#endif // ROBOT_STATE_H
