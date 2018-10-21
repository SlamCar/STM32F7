#ifndef TRSNDPORT_H
#define TRSNDPORT_H   

#include "Data_Base.h"
#include "Protocal.h"
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

Bool feedMsgPack(Feedback_Msg msg);
int dataCheck(uint8_t data);

Bool EndianTrans();





#endif

