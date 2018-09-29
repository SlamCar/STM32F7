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
}APP_USER_Parm;

typedef struct
{
    float speed;
    float angle;
}Control_Msg;

typedef struct
{
    u8 test;
}Feedback_Msg;

typedef struct
{
    u8 led;
    u8 beep; 

}APP_USER_Status;

typedef struct
{
    u8 led;
    u8 beep;
    Control_Msg  control_msg;

}APP_USER_Input;

typedef struct
{
    u8 led;
    u8 beep;
    Feedback_Msg  feedback_msg;

}APP_USER_Output;


extern  APP_USER_Parm    g_APP_USER_Parm;
    
extern  APP_USER_Input   g_APP_USER_Input;
extern  APP_USER_Output  g_APP_USER_Output;
    
void USER_GetInput(APP_USER_Input *Input);
void USER_RunCtr(APP_USER_Input *Input, APP_USER_Output *Output);
void USER_SetOutput(APP_USER_Output* Output);

#endif
