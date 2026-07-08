#ifndef __PET_ACTION_
#define __PET_ACTION_

#include <stdint.h>
#include "RobotState.h"

// 函数声明
void Action_dongzhuo_0(float x01,float x02,float x03,float x04,
                      float y01,float y02,float y03,float y04,
                      float z01,float z02,float z03,float z04,
                      float t,float zd);
void Action_upright(void);
void Action_getdowm(void);
void Action_sit(void);
void Action_Swing(void);
void Action_advance(float ll,float hh,float dd,float mm,float zz,
                   float tt,float ii,float rr1,float rr2,float zd);
void Action_advance_zuoyou(float ll,float hh,float dd,float mm,float zz,
                          float tt,float ii,float rr1,float rr2,float zd);
void Action_dongzhuo_1(float x01,float x02,float x03,float x04,
                      float y01,float y02,float y03,float y04,
                      float t,float zd);
void Action_dongzhuo_2(float x01,float x02,float x03,float x04,
                      float y01,float y02,float y03,float y04,
                      float z01,float z02,float z03,float z04,
                      float t,float zd);
void Action_advance_yuandizhuan(float ll,float hh,float dd,float mm,float zz,
                               float tt,float ii,float rr1,float rr2,float zd);
void Action_ceshi(void);
void Action_yaobai(float ll,float hh,float dd,float mm,float zz,
                  float tt,float ii,float rr1,float rr2,float zd);
float tiao_high(void);

#endif