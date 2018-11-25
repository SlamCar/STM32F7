#include "Data_Base.h"

Feedback_Msg  db_feedbackMsg = {0};
Control_Msg   db_controlMsg =  {0};

ROBOT_Input   g_ROBOT_Input =  {0};
ROBOT_Output  g_ROBOT_Output = {0};

void dataBaseInit(void)
{
    db_feedbackMsg.now_angle = 0.0;
    db_feedbackMsg.now_speed = 0.0;
    
    db_controlMsg.angle = 0.0;
    db_controlMsg.speed = 0.0;
}

void updataMessage(const SerialPakage pack)
{
    switch(pack.head_.dataId)
    {
        case CMD_IPC_COMMOND:  updateCmdMsg(pack);
            break;
        case CMD_RESET:
            break;
        case DEBUG_QT_COMMOND:
            break;
    }
}

void updateCmdMsg(const SerialPakage pack)
{
    memcpy(&db_controlMsg, pack.byData_, sizeof(Control_Msg)); 
}

//void Update_FeedbackMsg(const Feedback_Msg msg)
//{
//}

/*
void ROBOT_Input(ROBOT_Input *Input)
{
    //�������ݽ���
    //�������ݿ�  ��������  ����������  �����˲���
    
}

void ROBOT_RunCtr(ROBOT_Input *Input, ROBOT_Output *Output)
{
    //�߼�����  ģ�ͽ���  
}
*/


