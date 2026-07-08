#include "stm32f10x.h"                  // Device header
#include "Servo.h"
#include "Delay.h"
#include "BlueTooth.h"
#include "RobotState.h"
#include "robot_config.h"
#include <math.h>   // 需链接数学库（编译时加 -lm）

// 使用大狗的尺寸参数 - 腿长100mm
// 全局坐标变量已移至robot结构体中

uint16_t TiaoTurn=0;
uint16_t TiaoTurn2=0;

// 函数声明
void x1_y1(float x, float y, float z);
void x2_y2(float x, float y, float z);
void x3_y3(float x, float y, float z);
void x4_y4(float x, float y, float z);

// 身高调整函数 - 使用配置文件中的参数
float tiao_high(void)
{
    float high1;
    if(robot.action_high==1)//身高
    {
        high1=DEFAULT_HEIGHT_MM;
    }
    else if(robot.action_high==2)//身高
    {
        high1=MIN_HEIGHT_MM;
    }
    else
    {
        high1 = robot.height; // 使用全局变量high
    }
    return high1;
}

// 测试函数 - 保留小狗的测试代码
void Action_ceshi(void)//测试
{
    Servo_Angle1(90);
    Delay_ms(100);
    Servo_Angle2(90);
    Delay_ms(100);
    Servo_Angle3(90);
    Delay_ms(100);
    Servo_Angle4(90);
    Delay_ms(100);
    Servo_Angle5(90);
    Delay_ms(100);
    Servo_Angle6(90-10);
    Delay_ms(100);
    Servo_Angle7(90);
    Delay_ms(100);
    Servo_Angle8(90);
    Delay_ms(100);
    Servo_Angle9(90-5);
    Delay_ms(100);
    Servo_Angle10(90);
    Delay_ms(100);
    Servo_Angle11(90);
    Delay_ms(100);
    Servo_Angle12(90);
    Delay_ms(100);
}

// 动作控制函数 - 直接设置目标位置
void Action_dongzhuo_0(float x01,float y01,float z01,float x02,float y02,float z02,
                      float x03,float y03,float z03,float x04,float y04,float z04,
                      float t,float zd)
{
    if(robot.action_mode != zd) return;
    
    // 直接设置目标位置
    robot.leg_positions.x[0] = x01;
    robot.leg_positions.x[1] = x02;
    robot.leg_positions.x[2] = x03;
    robot.leg_positions.x[3] = x04;
    
    robot.leg_positions.y[0] = y01;
    robot.leg_positions.y[1] = y02;
    robot.leg_positions.y[2] = y03;
    robot.leg_positions.y[3] = y04;
    
    robot.leg_positions.z[0] = z01;
    robot.leg_positions.z[1] = z02;
    robot.leg_positions.z[2] = z03;
    robot.leg_positions.z[3] = z04;
    
    // 设置舵机位置
    x1_y1(robot.leg_positions.x[0],robot.leg_positions.y[0],robot.leg_positions.z[0]);
    Delay_ms(5);
    x2_y2(robot.leg_positions.x[1],robot.leg_positions.y[1],robot.leg_positions.z[1]);
    Delay_ms(5);
    x3_y3(robot.leg_positions.x[2],robot.leg_positions.y[2],robot.leg_positions.z[2]);
    Delay_ms(5);
    x4_y4(robot.leg_positions.x[3],robot.leg_positions.y[3],robot.leg_positions.z[3]);
    Delay_ms(5);
}

// 摇摆动作
void Action_yaobai(float ll,float hh,float dd,float mm,float zz,float tt,
                   float ii,float rr1,float rr2,float zd)
{
    if(robot.action_mode != zd) return;
    
    mm = tiao_high();  // 使用调整后的身高
    
    float y1, y2, y3, y4;
    float x1, x2, x3, x4;
    float z1 = zz, z2 = zz, z3 = zz, z4 = zz;
    
    float r1 = rr1;
    float r2 = rr2;
    float r3 = rr1;
    float r4 = rr2;
    
    float speed_value = 0.04;
    float faai = 0.5;
    float ts = 1;
    float pi = 3.14;
    static float t = 0; // 使用静态变量保存状态
    
    if(t > 1) t = 0;
    t = t + speed_value;
    
    float xf, xs;
    if(ii == 1) {
        xf = ll/2 + dd;
        xs = dd - ll/2;
    } else if(ii == 2) {
        xs = ll/2 + dd;
        xf = dd - ll/2;
    } else {
        xf = ll/2 + dd;
        xs = dd - ll/2;
    }
    
    if(t <= ts * faai) {
        float sigma = 2 * pi * t / (faai * ts);
        float yep = hh * (1 - cos(sigma)) / 2;
        float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
        float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
        
        y1 = mm - yep;
        y2 = mm;
        y3 = mm;
        y4 = mm - yep;
        
        x1 = -xep_z * r1;
        x2 = -xep_b * r2;
        x3 = -xep_b * r3;
        x4 = -xep_z * r4;
        
        x1_y1(x1, y1, z1);
        Delay_ms(tt);
        x2_y2(x1, y1, z2);
        Delay_ms(tt);
        x3_y3(x1, y1, z3);
        Delay_ms(tt);
        x4_y4(x1, y1, z4);
        Delay_ms(tt);
    } else if(t > ts * faai && t < ts) {
        float sigma = 2 * pi * (t - faai * ts) / (faai * ts);
        float yep = hh * (1 - cos(sigma)) / 2;
        float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
        float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
        
        y1 = mm;
        y2 = mm - yep;
        y3 = mm - yep;
        y4 = mm;
        
        x1 = -xep_b * r1;
        x2 = -xep_z * r2;
        x3 = -xep_z * r3;
        x4 = -xep_b * r4;
        
        x1_y1(x1, y1, z1);
        Delay_ms(tt);
        x2_y2(x1, y1, z2);
        Delay_ms(tt);
        x3_y3(x1, y1, z3);
        Delay_ms(tt);
        x4_y4(x1, y1, z4);
        Delay_ms(tt);
    }
}

// 前进动作
void Action_advance(float ll,float hh,float dd,float mm,float zz,float tt,
                    float ii,float rr1,float rr2,float zd)
{
    if(robot.action_mode != zd) return;
    
    mm = tiao_high();  // 使用调整后的身高
    
    float y1, y2, y3, y4;
    float x1, x2, x3, x4;
    float z1 = zz, z2 = zz, z3 = zz, z4 = zz;
    
    float r1 = rr1;
    float r2 = rr2;
    float r3 = rr1;
    float r4 = rr2;
    
    float speed_value = 0.04;
    float faai = 0.5;
    float ts = 1;
    float pi = 3.14;
    static float t = 0; // 使用静态变量保存状态
    
    if(t > 1) t = 0;
    t = t + speed_value;
    
    float xf, xs;
    if(ii == 1) {
        xf = ll/2 + dd;
        xs = dd - ll/2;
    } else if(ii == -1) {
        xs = ll/2 + dd;
        xf = dd - ll/2;
    } else {
        xf = ll/2 + dd;
        xs = dd - ll/2;
    }
    
    if(t <= ts * faai) {
        float sigma = 2 * pi * t / (faai * ts);
        float yep = hh * (1 - cos(sigma)) / 2;
        float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
        float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
        
        y1 = mm - yep;
        y2 = mm;
        y3 = mm;
        y4 = mm - yep;
        
        x1 = -xep_z * r1;
        x2 = -xep_b * r2;
        x3 = -xep_b * r3;
        x4 = -xep_z * r4;
        
        x1_y1(x1, y1, z1);
        Delay_ms(tt);
        x2_y2(x2, y2, z2);
        Delay_ms(tt);
        x3_y3(x3, y3, z3);
        Delay_ms(tt);
        x4_y4(x4, y4, z4);
        Delay_ms(tt);
    } else if(t > ts * faai && t < ts) {
        float sigma = 2 * pi * (t - faai * ts) / (faai * ts);
        float yep = hh * (1 - cos(sigma)) / 2;
        float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
        float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
        
        y1 = mm;
        y2 = mm - yep;
        y3 = mm - yep;
        y4 = mm;
        
        x1 = -xep_b * r1;
        x2 = -xep_z * r2;
        x3 = -xep_z * r3;
        x4 = -xep_b * r4;
        
        x1_y1(x1, y1, z1);
        Delay_ms(tt);
        x2_y2(x2, y2, z2);
        Delay_ms(tt);
        x3_y3(x3, y3, z3);
        Delay_ms(tt);
        x4_y4(x4, y4, z4);
        Delay_ms(tt);
    }
}

// 单轴动作控制1
void Action_dongzhuo_1(float x01, float x02, float x03, float x04, 
                       float y01, float y02, float y03, float y04, 
                       float t, float zd) 
{
    if (robot.action_mode != zd) return;

    // 保存起始位置
    float start_x1 = robot.leg_positions.x[0];
    float start_y1 = robot.leg_positions.y[0];
    float start_x2 = robot.leg_positions.x[1];
    float start_y2 = robot.leg_positions.y[1];
    float start_x3 = robot.leg_positions.x[2];
    float start_y3 = robot.leg_positions.y[2];
    float start_x4 = robot.leg_positions.x[3];
    float start_y4 = robot.leg_positions.y[3];

    // 计算每步的增量
    float dx1 = (x01 - start_x1) / 20;
    float dy1 = (y01 - start_y1) / 20;
    float dx2 = (x02 - start_x2) / 20;
    float dy2 = (y02 - start_y2) / 20;
    float dx3 = (x03 - start_x3) / 20;
    float dy3 = (y03 - start_y3) / 20;
    float dx4 = (x04 - start_x4) / 20;
    float dy4 = (y04 - start_y4) / 20;

    // 计算每步的延迟时间
    float step_delay = t / 2;

    // 移动第一个舵机
    for (int i = 1; i <= 20; i++) {
        robot.leg_positions.x[0] = start_x1 + i * dx1;
        robot.leg_positions.y[0] = start_y1 + i * dy1;
        x1_y1(robot.leg_positions.x[0], robot.leg_positions.y[0], 0);
        Delay_ms(step_delay);
    }

    // 移动第二个舵机
    for (int i = 1; i <= 20; i++) {
        robot.leg_positions.x[1] = start_x2 + i * dx2;
        robot.leg_positions.y[1] = start_y2 + i * dy2;
        x2_y2(robot.leg_positions.x[1], robot.leg_positions.y[1], 0);
        Delay_ms(step_delay);
    }

    // 移动第三个舵机
    for (int i = 1; i <= 20; i++) {
        robot.leg_positions.x[2] = start_x3 + i * dx3;
        robot.leg_positions.y[2] = start_y3 + i * dy3;
        x3_y3(robot.leg_positions.x[2], robot.leg_positions.y[2], 0);
        Delay_ms(step_delay);
    }

    // 移动第四个舵机
    for (int i = 1; i <= 20; i++) {
        robot.leg_positions.x[3] = start_x4 + i * dx4;
        robot.leg_positions.y[3] = start_y4 + i * dy4;
        x4_y4(robot.leg_positions.x[3], robot.leg_positions.y[3], 0);
        Delay_ms(step_delay);
    }
}

// 单轴动作控制2
void Action_dongzhuo_2(float x01,float x02,float x03,float x04,
                      float y01,float y02,float y03,float y04,
                      float z01,float z02,float z03,float z04,
                      float t,float zd)
{
    if (robot.action_mode != zd) return;

    // 保存起始位置
    float start_x1 = robot.leg_positions.x[0];
    float start_y1 = robot.leg_positions.y[0];
    float start_z1 = robot.leg_positions.z[0];
    float start_x2 = robot.leg_positions.x[1];
    float start_y2 = robot.leg_positions.y[1];
    float start_z2 = robot.leg_positions.z[1];
    float start_x3 = robot.leg_positions.x[2];
    float start_y3 = robot.leg_positions.y[2];
    float start_z3 = robot.leg_positions.z[2];
    float start_x4 = robot.leg_positions.x[3];
    float start_y4 = robot.leg_positions.y[3];
    float start_z4 = robot.leg_positions.z[3];

    // 计算每步的增量
    float dx1 = (x01 - start_x1) / 20;
    float dy1 = (y01 - start_y1) / 20;
    float dz1 = (z01 - start_z1) / 20;
    float dx2 = (x02 - start_x2) / 20;
    float dy2 = (y02 - start_y2) / 20;
    float dz2 = (z02 - start_z2) / 20;
    float dx3 = (x03 - start_x3) / 20;
    float dy3 = (y03 - start_y3) / 20;
    float dz3 = (z03 - start_z3) / 20;
    float dx4 = (x04 - start_x4) / 20;
    float dy4 = (y04 - start_y4) / 20;
    float dz4 = (z04 - start_z4) / 20;

    // 计算每步的延迟时间
    float step_delay = t / 2;

    // 移动第一个舵机
    for (int i = 1; i <= 20; i++) {
        robot.leg_positions.x[0] = start_x1 + i * dx1;
        robot.leg_positions.y[0] = start_y1 + i * dy1;
        robot.leg_positions.z[0] = start_z1 + i * dz1;
        x1_y1(robot.leg_positions.x[0], robot.leg_positions.y[0], robot.leg_positions.z[0]);
        Delay_ms(step_delay);
    }

    // 移动第二个舵机
    for (int i = 1; i <= 20; i++) {
        robot.leg_positions.x[1] = start_x2 + i * dx2;
        robot.leg_positions.y[1] = start_y2 + i * dy2;
        robot.leg_positions.z[1] = start_z2 + i * dz2;
        x2_y2(robot.leg_positions.x[1], robot.leg_positions.y[1], robot.leg_positions.z[1]);
        Delay_ms(step_delay);
    }

    // 移动第三个舵机
    for (int i = 1; i <= 20; i++) {
        robot.leg_positions.x[2] = start_x3 + i * dx3;
        robot.leg_positions.y[2] = start_y3 + i * dy3;
        robot.leg_positions.z[2] = start_z3 + i * dz3;
        x3_y3(robot.leg_positions.x[2], robot.leg_positions.y[2], robot.leg_positions.z[2]);
        Delay_ms(step_delay);
    }

    // 移动第四个舵机
    for (int i = 1; i <= 20; i++) {
        robot.leg_positions.x[3] = start_x4 + i * dx4;
        robot.leg_positions.y[3] = start_y4 + i * dy4;
        robot.leg_positions.z[3] = start_z4 + i * dz4;
        x4_y4(robot.leg_positions.x[3], robot.leg_positions.y[3], robot.leg_positions.z[3]);
        Delay_ms(step_delay);
    }
}

// 左右移动动作
void Action_advance_zuoyou(float ll,float hh,float dd,float mm,float zz,float tt,
                    float ii,float rr1,float rr2,float zd)
{
    if(robot.action_mode != zd) return;
    
    mm = tiao_high();  // 使用调整后的身高
    
    float y1, y2, y3, y4;
    float x1, x2, x3, x4;
	  float ax1, ax2, ax3, ax4;
	  float ay1, ay2, ay3, ay4;
	  float az1, az2, az3, az4;
    float z1 = zz, z2 = zz, z3 = zz, z4 = zz;
    
    float r1 = rr1;
    float r2 = rr2;
    float r3 = rr1;
    float r4 = rr2;
    
    float speed_value = 0.04;
    float faai = 0.5;
    float ts = 1;
    float pi = 3.14;
    static float t = 0; // 使用静态变量保存状态
    
    if(t > 1) t = 0;
    t = t + speed_value;
    
    float xf, xs;
    if(ii == 1) {
        xf = ll/2 + dd;
        xs = dd - ll/2;
    } else if(ii == -1) {
        xs = ll/2 + dd;
        xf = dd - ll/2;
    } else {
        xf = ll/2 + dd;
        xs = dd - ll/2;
    }
    
    if(t <= ts * faai) {
        float sigma = 2 * pi * t / (faai * ts);
        float yep = hh * (1 - cos(sigma)) / 2;
        float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
        float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
        
        y1 = mm - yep;
        y2 = mm;
        y3 = mm;
        y4 = mm - yep;
        
        x1 = -xep_z * r1;
        x2 = -xep_b * r2;
        x3 = -xep_b * r3;
        x4 = -xep_z * r4;
        
			ax1=z3+10;ay1=y3;az1=x3;
			ax2=z1+10;ay2=y1;az2=x1;
			ax3=z4-10;ay3=y4;az3=x4;
			ax4=z2-10;ay4=y2;az4=x2;
			
        x1_y1(ax1, ay1, az1);
        Delay_ms(tt);
        x2_y2(ax2, ay2, az2);
        Delay_ms(tt);
        x3_y3(ax3, ay3, az3);
        Delay_ms(tt);
        x4_y4(ax4, ay4, az4);
        Delay_ms(tt);
    } else if(t > ts * faai && t < ts) {
        float sigma = 2 * pi * (t - faai * ts) / (faai * ts);
        float yep = hh * (1 - cos(sigma)) / 2;
        float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
        float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
        
        y1 = mm;
        y2 = mm - yep;
        y3 = mm - yep;
        y4 = mm;
        
        x1 = -xep_b * r1;
        x2 = -xep_z * r2;
        x3 = -xep_z * r3;
        x4 = -xep_b * r4;
        
			ax1=z3+10;ay1=y3;az1=x3;
			ax2=z1+10;ay2=y1;az2=x1;
			ax3=z4-10;ay3=y4;az3=x4;
			ax4=z2-10;ay4=y2;az4=x2;
			
        x1_y1(ax1, ay1, az1);
        Delay_ms(tt);
        x2_y2(ax2, ay2, az2);
        Delay_ms(tt);
        x3_y3(ax3, ay3, az3);
        Delay_ms(tt);
        x4_y4(ax4, ay4, az4);
			}
}

// 原地转动动作
void Action_advance_yuandizhuan(float ll,float hh,float dd,float mm,float zz,float tt,
                                float ii,float rr1,float rr2,float zd)
{
    if(robot.action_mode != zd) return;
    
    mm = tiao_high();  // 使用调整后的身高
    
    float y1, y2, y3, y4;
    float x1, x2, x3, x4;
    float z1 = zz, z2 = zz, z3 = zz, z4 = zz;
    
    float r1 = rr1;
    float r2 = rr2;
    float r3 = rr1;
    float r4 = rr2;
    
    float speed_value = 0.04;
    float faai = 0.5;
    float ts = 1;
    float pi = 3.14;
    static float t = 0; // 使用静态变量保存状态
    
    if(t > 1) t = 0;
    t = t + speed_value;
    
    float xf = ll/2 + dd;
    float xs = dd - ll/2;
    float xff = dd - ll/2;
    float xss = ll/2 + dd;
    float xep_bb = 0;
    float xep_zz = 0;	

    if(ii == 1) {
        if(t <= ts * faai) {
            float sigma = 2 * pi * t / (faai * ts);
            float yep = hh * (1 - cos(sigma)) / 2;
            float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
            float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
            xep_bb = (xff - xss) * ((sigma - sin(sigma)) / (2 * pi)) + xss;
            xep_zz = (xss - xff) * ((sigma - sin(sigma)) / (2 * pi)) + xff;
            
            y1 = mm - yep;
            y2 = mm;
            y3 = mm;
            y4 = mm - yep;
            
            x1 = -xep_zz * r1;
            x2 = -xep_b * r2;
            x3 = -xep_bb * r3;
            x4 = -xep_z * r4;
            
            x1_y1(x1, y1, z1);
            Delay_ms(tt);
            x2_y2(x2, y2, z2);
            Delay_ms(tt);
            x3_y3(x3, y3, z3);
            Delay_ms(tt);
            x4_y4(x4, y4, z4);
            Delay_ms(tt);
        } else if(t > ts * faai && t < ts) {
            float sigma = 2 * pi * (t - faai * ts) / (faai * ts);
            float yep = hh * (1 - cos(sigma)) / 2;
            float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
            float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
            
            y1 = mm;
            y2 = mm - yep;
            y3 = mm - yep;
            y4 = mm;
            
            x1 = -xep_bb * r1;
            x2 = -xep_z * r2;
            x3 = -xep_zz * r3;
            x4 = -xep_b * r4;
            
            x1_y1(x1, y1, z1);
            Delay_ms(tt);
            x2_y2(x2, y2, z2);
            Delay_ms(tt);
            x3_y3(x3, y3, z3);
            Delay_ms(tt);
            x4_y4(x4, y4, z4);
            Delay_ms(tt);
        }
    } else if(ii == -1) {
        if(t <= ts * faai) {
            float sigma = 2 * pi * t / (faai * ts);
            float yep = hh * (1 - cos(sigma)) / 2;
            float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
            float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
            xep_bb = (xff - xss) * ((sigma - sin(sigma)) / (2 * pi)) + xss;
            xep_zz = (xss - xff) * ((sigma - sin(sigma)) / (2 * pi)) + xff;
            
            y1 = mm - yep;
            y2 = mm;
            y3 = mm;
            y4 = mm - yep;
            
            x1 = -xep_z * r1;
            x2 = -xep_bb * r2;
            x3 = -xep_b * r3;
            x4 = -xep_zz * r4;
            
            x1_y1(x1, y1, z1);
            Delay_ms(tt);
            x2_y2(x2, y2, z2);
            Delay_ms(tt);
            x3_y3(x3, y3, z3);
            Delay_ms(tt);
            x4_y4(x4, y4, z4);
            Delay_ms(tt);
        } else if(t > ts * faai && t < ts) {
            float sigma = 2 * pi * (t - faai * ts) / (faai * ts);
            float yep = hh * (1 - cos(sigma)) / 2;
            float xep_b = (xf - xs) * ((sigma - sin(sigma)) / (2 * pi)) + xs;
            float xep_z = (xs - xf) * ((sigma - sin(sigma)) / (2 * pi)) + xf;
            
            y1 = mm;
            y2 = mm - yep;
            y3 = mm - yep;
            y4 = mm;
            
            x1 = -xep_b * r1;
            x2 = -xep_zz * r2;
            x3 = -xep_z * r3;
            x4 = -xep_bb * r4;
            
            x1_y1(x1, y1, z1);
            Delay_ms(tt);
            x2_y2(x2, y2, z2);
            Delay_ms(tt);
            x3_y3(x3, y3, z3);
            Delay_ms(tt);
            x4_y4(x4, y4, z4);
            Delay_ms(tt);
        }
    }
}