#include "transport.h"

void getMsg(void)
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

void sendMsg(void)
{
    SerialPakage msg = {0};
    msg = feedMsgPack(db_feedbackMsg);
    
    EndianTrans(msg);
    
    HAL_UART_Transmit(&UART_Handler[UART_DEV1],(uint8_t *)&msg, 
                       HEAD_BYTESIZE + sizeof(Feedback_Msg) + CRC_BYTESIZE ,1000);
    
    while(__HAL_UART_GET_FLAG(&UART_Handler[UART_DEV1],UART_FLAG_TC)!=SET){};    //wait  untill send end 
    memset(&msg, 0 , sizeof(SerialPakage));     //clear data    
}

void dataReceive(const UART_MSG *uart_msg)
{
    SerialPakage msg = {0};
    if(uart_msg->wRxdLen > HEAD_BYTESIZE + BODY_MAX_BYTESIZE + CRC_BYTESIZE)
    {
        return;
    }
    memcpy(&msg, uart_msg->byRxdBuf, uart_msg->wRxdLen);
    
    EndianTrans(msg);
    
    if(checkCrc(msg))
    {
       //update dataBase 
    }
}

Bool checkCrc(SerialPakage msg)
{
    uint16_t msgCrc = 0;
    uint16_t crc = 0;
    
    memcpy(&msgCrc,&msg.byData_[msg.head_.dataLen],CRC_BYTESIZE);
    crc = generateCrc(msg);
    
    return crc == msgCrc ? TRUE : FALSE;
}

uint16_t generateCrc(SerialPakage msg)
{
    u8 t = 0;
    uint8_t* data = (uint8_t *)&msg;
    uint16_t crc = 0;
    for(t=0; t < HEAD_BYTESIZE + msg.head_.dataLen; t++)
    {
        crc += data[t];
    }
    return crc;
}

SerialPakage feedMsgPack(Feedback_Msg feedbackMsg)
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


void EndianTrans(SerialPakage pack)
{
    memrev16((void *)&pack.head_.moduleId);
    memrev16((void *)&pack.head_.dataId);
    memrev32((void *)&pack.byData_);
    memrev32((void *)&pack.byData_[3]);
    memrev16((void *)&pack.byData_[7]);
}


#if 0    
    switch(rec_flag)
    {
        case NONE:
            rec_flag = MODULE_ID_H
            break;
        
/********************moduleId********************/
        
        case MODULE_ID_H:
            //memset(&g_SerialPackRX, 0 , sizeof(SerialPakage));
           if(0x03 == data)
           {
              p_data =  (uint8_t *)&g_SerialPackRX;
              memcpy(p_data, data, 1);
              rec_flag = MODULE_ID_L;
           }
            else 
              rec_flag = 0;
        break;                                     
        case MODULE_ID_L:
            if(0x9c == data)
            {
               memcpy(++p_data, &data, 1); 
               rec_flag = DATA_ID_H;
            }
            else
            {
               rec_flag = NONE;
               memset(&g_SerialPackRX, 0 , sizeof(SerialPakage));
            }
            break;
            
/********************dataId********************/ 
            
        case DATA_ID_H:  
            memcpy(++p_data, &data, 1); 
            rec_flag = DATA_ID_L;
            break;
        case DATA_ID_L:
            memcpy(++p_data, &data, 1);
            rec_flag = DATA_LEN;                
            break;
        
/********************dataLen********************/

        case DATA_LEN:
            memcpy(++p_data, &data, 1);
            rec_flag = RECVLEN;
            break;
        /***recvLen***/
        case RECVLEN:
            memcpy(++p_data, &data, 1); 
            if(0 == g_SerialPackRX.head_.dataLen)
            {
               rec_flag = CRC_H;
            }
            else rec_flag ++;  
            break;
            
/********************data********************/
        
        case BYDATA: 
            if(g_SerialPackRX.head_.dataLen == g_SerialPackRX.head_.recvLen)
            {
               rec_flag = CRC_H;
            }
            
            if(g_SerialPackRX.head_.recvLen < g_SerialPackRX.head_.dataLen)
            {
               g_SerialPackRX.head_.recvLen++; 
               memcpy(++p_data, &data, 1);                        
            }
            else
            {
              rec_flag = 0;    //data error
              memset(&g_SerialPackRX, 0 , sizeof(SerialPakage));
            }
            break;
            
/******************** crc ********************/
            
        case CRC_H:
            memcpy(++p_data, &data, 1);
            //g_SerialPackRX.check_ = (uint16_t)data;
            rec_flag ++;
            break;
        case CRC_L:
            memcpy(++p_data, &data, 1);
            //g_SerialPackRX.check_ = (uint16_t)data << 8;
            //校验成功 db_cmd_update 打开
            rec_flag = 0;
           // rec_flag = 1;
            break;
        
        default:
            break;
    }

#endif


