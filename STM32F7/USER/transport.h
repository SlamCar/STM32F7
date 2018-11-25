#ifndef TRSNDPORT_H
#define TRSNDPORT_H   

#include "Data_Base.h"
#include "Protocal.h"
#include "UART_Interface.h"
#include "sys.h"
#include <string.h>

//FLOAR <--> HEX
typedef  union{
        float fv;
        uint8_t cv[4];
}float_union;

typedef enum RECSTATE
{
    NONE = 0,
    MODULE_ID_H,
    MODULE_ID_L,
    DATA_ID_H,
    DATA_ID_L,
    DATA_LEN,
    RECVLEN,
    BYDATA,
    CRC_H,
    CRC_L
}Receive_State;

void getMsg(void);
void sendMsg(void);

void dataReceive(const UART_MSG *uart_msg);

Bool checkCrc(SerialPakage msg); 
uint16_t generateCrc(SerialPakage msg);

SerialPakage feedMsgPack(Feedback_Msg feedbackMsg);
void EndianTrans(SerialPakage msg);





#endif

