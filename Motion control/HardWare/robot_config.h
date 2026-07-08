#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

// 机器人类型选择
#define ROBOT_TYPE_LARGE_DOG  1
#define ROBOT_TYPE_SMALL_DOG  2

// 默认选择大狗
#define ROBOT_TYPE ROBOT_TYPE_LARGE_DOG

// 根据选择配置参数
#if ROBOT_TYPE == ROBOT_TYPE_LARGE_DOG
    // 大狗参数
    #define LEG_LENGTH_MM      100.0f
    #define DEFAULT_HEIGHT_MM  180.0f
    #define MIN_HEIGHT_MM      155.0f
    #define MAX_HEIGHT_MM      200.0f
    #define INITIAL_Y_POS      150.0f  // 初始y坐标
#elif ROBOT_TYPE == ROBOT_TYPE_SMALL_DOG
    // 小狗参数
    #define LEG_LENGTH_MM      80.0f
    #define DEFAULT_HEIGHT_MM  150.0f
    #define MIN_HEIGHT_MM      130.0f
    #define MAX_HEIGHT_MM      170.0f
    #define INITIAL_Y_POS      120.0f  // 初始y坐标
#endif

// 动作参数
#define ACTION_INTERVAL_MS   5       // 动作执行间隔
#define ACTION_TIMEOUT_MS    62000    // 动作超时时间
#define DANCE_STEP_INTERVAL_MS 1000  // 跳舞步骤间隔

// 舵机参数
#define SERVO_MIN_ANGLE      0       // 舵机最小角度
#define SERVO_MAX_ANGLE      180     // 舵机最大角度
#define SERVO_MID_ANGLE      90      // 舵机中间角度

#endif // ROBOT_CONFIG_H
