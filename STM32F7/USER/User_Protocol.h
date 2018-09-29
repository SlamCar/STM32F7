#ifndef __USER_PROTOCOL_H
#define __USER_PROTOCOL_H

#include "udp_demo.h"

/* protocol command */
#define CMD_GET_VERSION     0x3030u
#define CMD_GET_RTC_TIME    0x3130u
#define CMD_GET_DIAGNOSE    0x3230u  // get diagnoses information
#define CMD_GET_CONFIG      0x3330u  // get configuration information
#define CMD_GET_USER_REAL   0x3430u  // get real time data
#define CMD_READ_MEM        0x3530u  // get the memory of the specified address
#define CMD_SET_TIME        0x3140u
#define CMD_SET_CONFIG      0x3340u
#define CMD_WRITE_MEM       0x3540u

#define CMD_COM_CAN1TXD     0x3050u  // sending data through CAN1
#define CMD_COM_CAN2TXD     0x3150u  //

#define CMD_UPDATE_PREPARE  0x3060u  // firmware update preparation
#define CMD_UPDATE_DOWN     0x3160u  // firmware update, 1024 bytes per package, insufficient supplement 0xFF
#define CMD_UPDATE_UPCHECK  0x3260u
#define CMD_UPDATE_REFLASH  0x3360u
#define CMD_RESET           0x3070u

// Server command
#define CMD_HEART_REQ               0x0002
#define CMD_HEART_RESP              0xF002
#define CMD_ENTER_BUNDLE_REQ        0x0003
#define CMD_ENTER_BUNDLE_RESP       0xF003
#define CMD_TASK_FINISH_REQ         0x0004
#define CMD_TASK_FINISH_RESP        0xF004
#define CMD_TASK_REQ                0x200A
#define CMD_TASK_RESP               0xF00A
#define CMD_REMOTE_CONTROL_REQ      0x200B
#define CMD_REMOTE_CONTROL_RESP     0xF00B

// IPC command
#define CMD_IPC_REQUEST             0x0005u  
#define CMD_IPC_RESP                0xF005u

// command execution
#define CO_NORMAL                   0x00u
#define CO_ERR_PARAMETER            0x01u
#define CO_ERR_EXECUTE              0x02u

#define APP_CONFIG_MARK_OK          0x55AA5AA5u

#define APP_COM_HEAD_LEN            16u
#define MAX_PACKAGE_LEN             1024u

// AGV type
#define AGV_MODE_DS                 0x01u
#define AGV_MODE_FK                 0x02u
#define MODULE_TYPE                 AGV_MODE_FK

#define UPGRADING_PAIO_UDP_THREAD   3u

#define VERSION_INFO                "CoTEK-FL V3.1 201805"

// protocol package
typedef struct
{                                        
    u16 wCmd;
    u8  byCO;           // execution
    u8  byModType;      // module type
    u8  byRsv[6];       // reserved
    u16 wTotal;         // total number of packages
    u16 wSeq;           // package serial number
    u16 wLen;           // data context length
    u8  byData[UDP_SEND_BUFSIZE - APP_COM_HEAD_LEN];
}APP_COM_PACKAGE;

/* IPC cmd parameter */
typedef struct 
{
    u32 command;
    float velocity;
    float omega;
    float driverVelocity;
    float steeringAngle;
    u32 abnormal;
    int obstacle_strategy;
	int mls_direction;
}IPC_Req;

/* Driver feedback parameter */
typedef struct 
{
    u8 velocity[4];            // fork speed   
    u8 omega[4];               // fork angle
    u8 driverVelocity[4];
    u8 steeringAngle[4];
	u8 mls_plc[4];             // mls offset
	u32 line_info;             // mls line information
	u32 hs_rfid;               // rfid info
    u32 forklift_state;        // fork lift state: upload or download;
    u32 forkcheck;             // fork dection sensor: left, right, both
    u32 poscheck;              // postion check sensor: left, right, both
    u32 avoidance;             // avoid sensor: normal, warning, emergency   
    u32 beep;                  // beeper state: on, off
    u32 batcapacity;           // battery capacity
    u32 statusCode;            // status code
    u32 errorCode;             // error code
}IPC_Rsp;

typedef struct
{
    u16 cmd;
    void (*exec)(u32, u16, APP_COM_PACKAGE*);
}CmdExecutors;

BOOLEAN USER_RecvParse(u32 ip, u16 port, u8 *pUdpData, u32 dwLen);

#endif
