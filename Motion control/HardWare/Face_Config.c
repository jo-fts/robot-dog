#include "stm32f10x.h"      
#include "OLED.h"
#include "BlueTooth.h"
#include "RobotState.h"

//实现表情变化，调节是进中断后

void Face_Config(void)
{
	if(robot.face_mode==0)
	{
		OLED_Clear();
		OLED_ShowImage(0,0,128,64,Face_sleep);//睡觉
		OLED_Update();
	}
	else if(robot.face_mode==1)
	{
		OLED_Clear();
		OLED_ShowImage(0,0,128,64,Face_stare);//瞪大眼
		OLED_Update();
	}
	else if(robot.face_mode==2)
	{
		OLED_Clear();
		OLED_ShowImage(0,0,128,64,Face_happy);//快乐
		OLED_Update();
	}
	else if(robot.face_mode==3)
	{
		OLED_Clear();
		OLED_ShowImage(0,0,128,64,Face_mania);//狂热
		OLED_Update();
	}
	else if(robot.face_mode==4)
	{
		OLED_Clear();
		OLED_ShowImage(0,0,128,64,Face_very_happy);//非常快乐
		OLED_Update();
	}
	else if(robot.face_mode==5)
	{
		OLED_Clear();
		OLED_ShowImage(0,0,128,64,Face_eyes);//眼睛
		OLED_Update();
	}
	else if(robot.face_mode==6)
	{
		OLED_Clear();
		OLED_ShowImage(0,0,128,64,Face_hello);//打招呼
		OLED_Update();
	}
}
