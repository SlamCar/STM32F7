#ifndef __DATA_BASE_H
#define __DATA_BASE_H
#include "sys.h"
#include "Protocal.h"

static Bool db_cmd_update = FALSE;

extern  SerialPakage g_SerialPackRX;
extern  SerialPakage g_SerialPackTX;

typedef struct
{
    float speed;
    float angle;
}Control_Msg;

typedef struct
{
    float r_speed;
    float l_speed;
    float wave1_dis;
    float wave2_dis;
}Sensor_Msg;

typedef struct
{
    float now_speed;
    float now_angle;
}Feedback_Msg;

extern Feedback_Msg db_feedbackMsg;
extern Control_Msg db_controlMsg;

enum  ROBOT_Line
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

void Update_CmdMsg();
void Update_Param();
void Update_FeedbackMsg();



#endif
