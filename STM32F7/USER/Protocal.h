#ifndef _PROTOCAL_H_
#define _PROTOCAL_H_
#include "sys.h"

#define     HEAD_BYTESIZE      6
#define     BODY_MAX_BYTESIZE  20
#define     CRC_BYTESIZE       2


#define     MODULEID           0x039c

enum CmdId
{
    /**
     * TEST
     */
    DEBUG_TEST_COMMOND   = 0x0001u,
    /**
     * QT -> STM32
     */
    DEBUG_QT_COMMOND     = 0xA010u,
    /**
     * IPC -> STM32
     */
    CMD_GET_VERSION      = 0x1010u,
    CMD_IPC_COMMOND      = 0x2010u,
    CMD_RESET            = 0x2020u,
    /**
     * STM32 -> IPC || QT
     */
    STM32_FEED_BACK      = 0X5010,
    STM32_HEART_BEAT     = 0x6010,
    STM32_TASK_FINISH    = 0x7010,
};

typedef struct DataHead_
{
    uint16_t moduleId;                  // fixed for MODULEID  9.24  birthday
    uint16_t dataId;
    uint8_t  dataLen;
    uint8_t  recvLen;
} Head;

typedef struct SerialPackage_
{
    Head head_;                         // 6 bype
    uint8_t byData_[BODY_MAX_BYTESIZE + CRC_BYTESIZE]; // data content, max size is 50
} SerialPakage;
  
#endif
