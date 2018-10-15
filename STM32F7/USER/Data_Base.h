#ifndef __DATA_BASE_H
#define __DATA_BASE_H
#include "sys.h"

enum  APP_USER_Line
{
   LineOK   = 0,
   LineERROR
};

typedef struct
{
   u8 test;
}ROBOT_Parm;

typedef struct
{
    float speed;
    float angle;
}Control_Msg;

typedef struct
{
    float now_speed;
    float now_angle;
}Feedback_Msg;

typedef struct
{
    u8 led;
    u8 beep; 
}ROBOT_Status;

typedef struct
{
    u8 led;
    u8 beep;
    Control_Msg  control_msg;
}ROBOT_Input;

typedef struct
{
    u8 led;
    u8 beep;
}ROBOT_Output;


extern  ROBOT_Parm    g_ROBOT_Parm;
    
extern  ROBOT_Input   g_ROBOT_Input;
extern  ROBOT_Output  g_ROBOT_Output;
    
//void ROBOT_Input(ROBOT_Input *Input);
//void ROBOT_RunCtr(ROBOT_Input *Input, ROBOT_Output *Output);
//void ROBOT_Output(ROBOT_Output* Output);

#endif
