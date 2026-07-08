#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "Delay.h"
#include "error_handler.h"
#include <math.h>   // 需链接数学库（编译时加 -lm）
#define PI 3.14

// 舵机角度范围定义
#define SERVO1_MIN 0
#define SERVO1_MAX 180
#define SERVO2_MIN 0
#define SERVO2_MAX 180
#define SERVO3_MIN 0
#define SERVO3_MAX 180
#define SERVO4_MIN 0
#define SERVO4_MAX 180
#define SERVO5_MIN 0
#define SERVO5_MAX 180
#define SERVO6_MIN 0
#define SERVO6_MAX 180
#define SERVO7_MIN 0
#define SERVO7_MAX 180
#define SERVO8_MIN 0
#define SERVO8_MAX 180
#define SERVO9_MIN 0
#define SERVO9_MAX 180
#define SERVO10_MIN 0
#define SERVO10_MAX 180
#define SERVO11_MIN 0
#define SERVO11_MAX 180
#define SERVO12_MIN 0
#define SERVO12_MAX 180

// 安全角度设置函数
float safe_servo_angle(uint8_t servo_num, float angle)
{
    // 根据舵机编号检查角度范围
    switch(servo_num) {
        case 1:
            if(angle < SERVO1_MIN) angle = SERVO1_MIN;
            if(angle > SERVO1_MAX) angle = SERVO1_MAX;
            break;
        case 2:
            if(angle < SERVO2_MIN) angle = SERVO2_MIN;
            if(angle > SERVO2_MAX) angle = SERVO2_MAX;
            break;
        case 3:
            if(angle < SERVO3_MIN) angle = SERVO3_MIN;
            if(angle > SERVO3_MAX) angle = SERVO3_MAX;
            break;
        case 4:
            if(angle < SERVO4_MIN) angle = SERVO4_MIN;
            if(angle > SERVO4_MAX) angle = SERVO4_MAX;
            break;
        case 5:
            if(angle < SERVO5_MIN) angle = SERVO5_MIN;
            if(angle > SERVO5_MAX) angle = SERVO5_MAX;
            break;
        case 6:
            if(angle < SERVO6_MIN) angle = SERVO6_MIN;
            if(angle > SERVO6_MAX) angle = SERVO6_MAX;
            break;
        case 7:
            if(angle < SERVO7_MIN) angle = SERVO7_MIN;
            if(angle > SERVO7_MAX) angle = SERVO7_MAX;
            break;
        case 8:
            if(angle < SERVO8_MIN) angle = SERVO8_MIN;
            if(angle > SERVO8_MAX) angle = SERVO8_MAX;
            break;
        case 9:
            if(angle < SERVO9_MIN) angle = SERVO9_MIN;
            if(angle > SERVO9_MAX) angle = SERVO9_MAX;
            break;
        case 10:
            if(angle < SERVO10_MIN) angle = SERVO10_MIN;
            if(angle > SERVO10_MAX) angle = SERVO10_MAX;
            break;
        case 11:
            if(angle < SERVO11_MIN) angle = SERVO11_MIN;
            if(angle > SERVO11_MAX) angle = SERVO11_MAX;
            break;
        case 12:
            if(angle < SERVO12_MIN) angle = SERVO12_MIN;
            if(angle > SERVO12_MAX) angle = SERVO12_MAX;
            break;
        default:
            // 未知舵机，使用默认安全角度
            angle = 90;
            handle_error(ERROR_SERVO_OUT_OF_RANGE);
            break;
    }
    return angle;
}

void Servo_Init()
{
    PWM_Init();    
}

// 舵机角度设置函数
void Servo_Angle1(float Angle)//左前1
{
    Angle = safe_servo_angle(1, Angle);
    PWM_SetCompare1(Angle / 180 * 2000 + 500);            
}

void Servo_Angle2(float Angle)//左前2
{
    Angle = safe_servo_angle(2, Angle);
    PWM_SetCompare2((180-Angle) / 180 * 2000 + 500);        
}

void Servo_Angle3(float Angle)//左前3
{
    Angle = safe_servo_angle(3, Angle);
    PWM_SetCompare3(Angle / 180 * 2000 + 500);            
}

void Servo_Angle4(float Angle)//右前1
{
    Angle = safe_servo_angle(4, Angle);
    PWM_SetCompare4((180-Angle) / 180 * 2000 + 500);            
}

void Servo_Angle5(float Angle)//右前2
{
    Angle = safe_servo_angle(5, Angle);
    PWM_SetCompare5(Angle / 180 * 2000 + 500);            
}

void Servo_Angle6(float Angle)//右前3
{
    Angle = safe_servo_angle(6, Angle);
    PWM_SetCompare6((180-Angle) / 180 * 2000 + 500);        
}

void Servo_Angle7(float Angle)//左后1
{
    Angle = safe_servo_angle(7, Angle);
    PWM_SetCompare7(Angle / 180 * 2000 + 500);            
}

void Servo_Angle8(float Angle)//左后2
{
    Angle = safe_servo_angle(8, Angle);
    PWM_SetCompare8((180-Angle) / 180 * 2000 + 500);            
}

void Servo_Angle9(float Angle)//左后3
{
    Angle = safe_servo_angle(9, Angle);
    PWM_SetCompare9((180-Angle) / 180 * 2000 + 500);            
}

void Servo_Angle10(float Angle)//右后1
{
    Angle = safe_servo_angle(10, Angle);
    PWM_SetCompare10((180-Angle) / 180 * 2000 + 500);        
}

void Servo_Angle11(float Angle)//右后2
{
    Angle = safe_servo_angle(11, Angle);
    PWM_SetCompare11(Angle / 180 * 2000 + 500);            
}

void Servo_Angle12(float Angle)//右后3
{
    Angle = safe_servo_angle(12, Angle);
    PWM_SetCompare12(Angle / 180 * 2000 + 500);    
}

// 运动学逆解计算 - 使用大狗的腿长参数100mm
void x_y_return_w1_w2(float_t x, float_t y, float_t z, 
                      float_t *w1, float_t *w2, float_t *w3)
{
    // 使用大狗的腿长参数
    float_t a = 100.0;  // 大腿长度100mm
    float_t b = 100.0;  // 小腿长度100mm
    float_t c;          // 脚位移
    
    float_t y1;         // 不考虑Z轴脚Y轴位移
    float_t r;          // 脚位移与竖直的夹角
    float_t v;          // 脚竖直位移与水平面的夹角
    float_t bb;         // a与c夹角
    float_t cc;         // a与b夹角
    float_t w11;        // 舵机1角度
    float_t w22;        // 舵机2角度
    float_t w33;        // 舵机3角度
    
    if(x >= 0)
    {
        x = fabs(x);
        y1 = sqrt(y*y + z*z);
        float tan_v = (z/y);
        v = atan(tan_v) * (180.0 / PI);
        c = sqrt(x*x + y1*y1);
        float tan_r = (x/y1);
        r = atan(tan_r) * (180.0 / PI);
        float cos_bb = (a*a + c*c - b*b) / (2*a*c);
        bb = acos(cos_bb) * (180.0 / PI);
        float cos_cc = (a*a + b*b - c*c) / (2*a*b);
        cc = acos(cos_cc) * (180.0 / PI);
        w11 = bb - r + 45;
        w22 = 270 - w11 - cc;
    }
    else if(x < 0)
    {
        x = fabs(x);
        y1 = sqrt(y*y + z*z);
        float tan_v = (z/y);
        v = atan(tan_v) * (180.0 / PI);
        c = sqrt(x*x + y*y);
        float tan_r = (x/y);
        r = atan(tan_r) * (180.0 / PI);
        float cos_bb = (a*a + c*c - b*b) / (2*a*c);
        bb = acos(cos_bb) * (180.0 / PI);
        float cos_cc = (a*a + b*b - c*c) / (2*a*b);
        cc = acos(cos_cc) * (180.0 / PI);
        w11 = bb + r + 45;
        w22 = 270 - w11 - cc;
    }
    
    w33 = v + 90;
    
    // 使用大狗的角度范围限制
    if(w11 > 0 && w11 < 130)
    {
        *w1 = w11;
    }
    else
    {
        *w1 = 45;  // 默认安全角度
    }
    
    if(w22 > 35 && w22 < 135)
    {
        *w2 = w22;
    }
    else
    {
        *w2 = 90;  // 默认安全角度
    }
    
    if(w33 > 35 && w33 < 180)
    {
        *w3 = w33;
    }
    else
    {
        *w3 = 90;  // 默认安全角度
    }
}

// 左前腿控制 - 添加大狗的舵机偏置
void x1_y1(float_t x1, float_t y1, float_t z1)
{
    float_t w11_result;
    float_t w21_result;
    float_t w31_result;
    z1 = -z1;  // 坐标转换
    
    x_y_return_w1_w2(x1, y1, z1, &w11_result, &w21_result, &w31_result);
    
    // 应用大狗的舵机偏置
    Servo_Angle1(45 + w11_result);
    Servo_Angle2(w21_result);
    Servo_Angle3(w31_result);  // 大狗的偏置
}

// 右前腿控制 - 添加大狗的舵机偏置
void x2_y2(float_t x1, float_t y1, float_t z1)
{
    float_t w11_result;
    float_t w21_result;
    float_t w31_result;
    
    x_y_return_w1_w2(x1, y1, z1, &w11_result, &w21_result, &w31_result);
    
    // 应用大狗的舵机偏置
    Servo_Angle4(45 + w11_result);
    Servo_Angle5(w21_result);
    Servo_Angle6(w31_result-10);  // 大狗的偏置
}

// 左后腿控制 - 添加大狗的舵机偏置
void x3_y3(float_t x1, float_t y1, float_t z1)
{
    float_t w11_result;
    float_t w21_result;
    float_t w31_result;
    z1 = -z1;  // 坐标转换
    
    x_y_return_w1_w2(x1, y1, z1, &w11_result, &w21_result, &w31_result);
    
    // 应用大狗的舵机偏置
    Servo_Angle7(45 + w11_result);
    Servo_Angle8(w21_result);   // 大狗的偏置
    Servo_Angle9(w31_result);  // 大狗的偏置-向内
}

// 右后腿控制 - 添加大狗的舵机偏置
void x4_y4(float_t x1, float_t y1, float_t z1)
{
    float_t w11_result;
    float_t w21_result;
    float_t w31_result;
    
    x_y_return_w1_w2(x1, y1, z1, &w11_result, &w21_result, &w31_result);
    
    // 应用大狗的舵机偏置
    Servo_Angle10(45 + w11_result);
    Servo_Angle11(w21_result);  // 大狗的偏置
    Servo_Angle12(w31_result);
}