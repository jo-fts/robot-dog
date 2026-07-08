#ifndef __SERVO_H
#define __SERVO_H

#include <stdint.h>

void Servo_Init(void);
void Servo_Angle1(float Angle);
void Servo_Angle2(float Angle);
void Servo_Angle3(float Angle);
void Servo_Angle4(float Angle);
void Servo_Angle5(float Angle);
void Servo_Angle6(float Angle);
void Servo_Angle7(float Angle);
void Servo_Angle8(float Angle);
void Servo_Angle9(float Angle);
void Servo_Angle10(float Angle);
void Servo_Angle11(float Angle);
void Servo_Angle12(float Angle);
void x_y_return_w1_w2(float x, float y, float z, float *w1, float *w2, float *w3);
void x1_y1(float x1, float y1, float z1);
void x2_y2(float x1, float y1, float z1);
void x3_y3(float x1, float y1, float z1);
void x4_y4(float x1, float y1, float z1);

#endif