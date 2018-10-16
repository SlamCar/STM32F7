#include "transport.h"

extern  SerialPakage g_SerialPackRX;
extern  SerialPakage g_SerialPackTX;

int dataCheck(uint8_t * data)
{
    static uint8_t flag = 0;
    uint8_t* p_data =  (uint8_t *)&g_SerialPackRX;
    
    switch(flag)
    {
     /***moduleId***/
        case 0:
            //memset(&g_SerialPackRX, 0 , sizeof(SerialPakage));
           if(0x03 == *data)
           {
              p_data =  (uint8_t *)&g_SerialPackRX;
              memcpy(p_data, data, 1);
              flag ++;
           }
            else 
              flag = 0;
        break;
        
        case 1:
            if(0x9c == *data)
            {
               memcpy(++p_data, data, 1); 
               flag ++;
            }
            else
            {
               flag = 0;
               memset(&g_SerialPackRX, 0 , sizeof(SerialPakage));
            }
            break;
        /***dataId***/    
        case 2:  
            memcpy(++p_data, data, 1); 
            flag ++;
            break;
        case 3:
            memcpy(++p_data, data, 1);
            flag ++;                
            break;
        /***dataLen***/
        case 4:
            memcpy(++p_data, data, 1);
            flag ++;
            break;
        /***recvLen***/
        case 5:
            memcpy(++p_data, data, 1); 
            if(0 == g_SerialPackRX.head_.dataLen)
            {
               flag = 7;
            }
            else flag ++;  
            break;
        /***data***/
        case 6: 
            if(g_SerialPackRX.head_.dataLen == g_SerialPackRX.head_.recvLen)
            {
               flag = 7;
            }
            
            if(g_SerialPackRX.head_.recvLen < g_SerialPackRX.head_.dataLen)
            {
               g_SerialPackRX.head_.recvLen++; 
               memcpy(++p_data, &data, 1);                        
            }
            else
            {
              flag = 0;    //data error
              memset(&g_SerialPackRX, 0 , sizeof(SerialPakage));
            }
            break;
        /***crc L***/
        case 7:
            memcpy(++p_data, &data, 1);
            //g_SerialPackRX.check_ = (uint16_t)data;
            flag ++;
            break;
        /***crc H***/
        case 8:
            memcpy(++p_data, &data, 1);
            //g_SerialPackRX.check_ = (uint16_t)data << 8;
            //校验成功 db_cmd_update 打开
            flag = 0;
           // rec_flag = 1;
            break;
        case 53:
            break;
    }

    return 0;
}

Bool feedMsgPack(Feedback_Msg msg)
{
    g_SerialPackTX.head_.moduleId = 0X039Cu;
    g_SerialPackTX.head_.dataId = 0X5010u;
    g_SerialPackTX.head_.dataLen = sizeof(Feedback_Msg);
    g_SerialPackTX.head_.recvLen = 0X00u;
    memcpy((uint8_t *)&g_SerialPackTX.byData_,(uint8_t *)&msg,sizeof(Feedback_Msg));

    
    g_SerialPackTX.check_ = 0X1234u;
    return TRUE;
}

Bool EndianTrans()
{
    memrev16((void *)&g_SerialPackTX.head_.moduleId);
    memrev16((void *)&g_SerialPackTX.head_.dataId);
    memrev32((void *)&g_SerialPackTX.byData_);
    memrev32((void *)&g_SerialPackTX.byData_[3]);
    memrev16((void *)&g_SerialPackTX.check_);
}

