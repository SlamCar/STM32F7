#include "transport.h"

void getMessage(void)
{
    if (UART_RxdWatch(UART_DEV1, 10))
    {
        UART_MSG sUartAppRxd = {0};

        INTX_DISABLE();
        sUartAppRxd = g_sUartAppRxd[UART_DEV1];
        g_sUartAppRxd[UART_DEV1].wRxdLen = 0;
        INTX_ENABLE();
        
        dataReceive(&sUartAppRxd);
    }
}

void gsendMessage(void)
{
    SerialPakage pack = {0};
    pack = feedBackMsgPack(db_feedbackMsg);
    
//    EndianTrans(msg);
    
    HAL_UART_Transmit(&UART_Handler[UART_DEV1],(uint8_t *)&pack, 
                       HEAD_BYTESIZE + pack.head_.dataLen + CRC_BYTESIZE ,1000);
    
    while(__HAL_UART_GET_FLAG(&UART_Handler[UART_DEV1],UART_FLAG_TC)!=SET){};    //wait  untill send end 
}

void dataReceive(const UART_MSG *uart_msg)
{
    SerialPakage pack = {0};
    if(uart_msg->wRxdLen > HEAD_BYTESIZE + BODY_MAX_BYTESIZE + CRC_BYTESIZE)
    {
        return;
    }
    memcpy(&pack, uart_msg->byRxdBuf, uart_msg->wRxdLen);
    
//    pack = EndianTrans(pack);
    
    if(!checkCrc(pack))
    {
        return;
    }
    updataMessage(pack);
}

Bool checkCrc(SerialPakage pack)
{
    uint16_t msgCrc = 0;
    uint16_t crc = 0;
    
    memcpy(&msgCrc,&pack.byData_[pack.head_.dataLen],CRC_BYTESIZE);
    crc = generateCrc(pack);
    
    return crc == msgCrc ? TRUE : FALSE;
}

uint16_t generateCrc(SerialPakage pack)
{
    u8 t = 0;
    uint8_t* data = (uint8_t *)&pack;
    uint16_t crc = 0;
    for(t=0; t < HEAD_BYTESIZE + pack.head_.dataLen; t++)
    {
        crc += data[t];
    }
    return crc;
}

SerialPakage feedBackMsgPack(Feedback_Msg feedbackMsg)
{
    uint16_t crc = 0;
    SerialPakage pack = {0};
    
    pack.head_.moduleId = 0X039Cu;
    pack.head_.dataId = 0X5010u;
    pack.head_.dataLen = sizeof(Feedback_Msg);
    pack.head_.recvLen = 0X00u;
    memcpy((uint8_t *)&pack.byData_,(uint8_t *)&feedbackMsg,sizeof(Feedback_Msg));
    
    crc = generateCrc(pack);
    memcpy((uint8_t *)&pack.byData_[pack.head_.dataLen],(uint8_t *)&crc,CRC_BYTESIZE);
    
    return pack;
}

SerialPakage EndianTrans(SerialPakage pack)
{
    memrev16((void *)&pack.head_.moduleId);
    memrev16((void *)&pack.head_.dataId);
    memrev32((void *)&pack.byData_);
    memrev32((void *)&pack.byData_[4]);
    memrev16((void *)&pack.byData_[pack.head_.dataLen]);
    return pack;
}
