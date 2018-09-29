#ifndef _PROTOCAL_H_
#define _PROTOCAL_H_
#include "sys.h"

#define     BODY_MAX_BYTESIZE  52
#define     CRC_BYTESIZE       1

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
    uint16_t moduleId;
    uint16_t dataId;
    uint8_t  dataLen;
    uint8_t  recv_len;
} Head;

typedef struct SerialPackage_
{
    Head head_; // fixed for MODULEID  9.24  birthday
    uint8_t byData_[BODY_MAX_BYTESIZE]; // data content, max size is 1024, append crc16
    uint8_t check_;
} SerialPakage;


#endif
