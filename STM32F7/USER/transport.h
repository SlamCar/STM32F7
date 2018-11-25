#ifndef TRSNDPORT_H
#define TRSNDPORT_H   

#include "Data_Base.h"
#include "Protocal.h"
#include "UART_Interface.h"
#include "sys.h"
#include <string.h>

void getMessage(void);
void sendMessage(void);

void dataReceive(const UART_MSG *uart_msg);
void dataSend(const UART_MSG *uart_msg);

uint16_t generateCrc(SerialPakage pack);
Bool checkCrc(SerialPakage pack); 

SerialPakage feedBackMsgPack(Feedback_Msg feedbackMsg);

SerialPakage EndianTrans(SerialPakage pack);

#endif

