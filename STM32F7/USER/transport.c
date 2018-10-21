#include "transport.h"

extern  SerialPakage g_SerialPackRX;
extern  SerialPakage g_SerialPackTX;

//Receive_State rec_flag = NONE;

int dataCheck(uint8_t data)
{
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

    return 0;
}

Bool feedMsgPack(Feedback_Msg msg)
{
    u8 t = 0;
    uint8_t* data = (uint8_t *)&g_SerialPackTX;
    
    memset(&g_SerialPackTX, 0 , sizeof(SerialPakage));     //clear data
    g_SerialPackTX.head_.moduleId = 0X039Cu;
    g_SerialPackTX.head_.dataId = 0X5010u;
    g_SerialPackTX.head_.dataLen = sizeof(Feedback_Msg);
    g_SerialPackTX.head_.recvLen = 0X00u;
    memcpy((uint8_t *)&g_SerialPackTX.byData_,(uint8_t *)&msg,sizeof(Feedback_Msg));
    
    for(t=0; t < HEAD_BYTESIZE + g_SerialPackTX.head_.dataLen; t++)
    {
        g_SerialPackTX.check_ += data[t];
    }
    //g_SerialPackTX.check_ = 0X1234u;
    return TRUE;
}

Bool EndianTrans()
{
    memrev16((void *)&g_SerialPackRX.head_.moduleId);
    memrev16((void *)&g_SerialPackRX.head_.dataId);
    memrev32((void *)&g_SerialPackRX.byData_);
    memrev32((void *)&g_SerialPackRX.byData_[3]);
    memrev16((void *)&g_SerialPackRX.check_);
}

