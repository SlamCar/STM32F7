/**
 * Copyright (c) 2018 CoTEK Inc. All rights reserved.
 */
 
#include "User_Protocol.h"
#include "User_Config.h"
#include "sys.h"
#include "delay.h"
#include "CRC.h"
#include "udp_demo.h"
#include "Update.h"
#include "STM32_FlashInterface.h"

static __align(4) u8 s_byUserSendBuf[UDP_SEND_BUFSIZE] = {0};

static void getVersion(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void getRTCTime(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void getDiagnose(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void getConfig(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void getUserReal(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void readMem(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void reset(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void setTime(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void setConfig(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void setCan1Tx(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void setCan2Tx(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void updatePrepare(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void updateDownload(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void updateUpcheck(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void updateReflash(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void ipcSync(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack);
static void USER_Echo(u32 IPaddr, u16 UdpPort, APP_COM_PACKAGE *psRxdApp, u8 byCO, u8 *pUserData, u16 wAppLen);

static CmdExecutors gCmdExes[] = {
    {CMD_IPC_RESP, ipcSync},
    {CMD_GET_VERSION, getVersion},
    {CMD_GET_RTC_TIME, getRTCTime},
    {CMD_SET_TIME, setTime},
    {CMD_GET_DIAGNOSE, getDiagnose},
    {CMD_GET_CONFIG, getConfig},
    {CMD_GET_USER_REAL, getUserReal},
    {CMD_READ_MEM, readMem},
    {CMD_RESET, reset},
    {CMD_SET_CONFIG, setConfig},
    {CMD_COM_CAN1TXD, setCan1Tx},
    {CMD_COM_CAN2TXD, setCan2Tx},
    {CMD_UPDATE_PREPARE, updatePrepare},
    {CMD_UPDATE_DOWN, updateDownload},
    {CMD_UPDATE_UPCHECK, updateUpcheck},
    {CMD_UPDATE_REFLASH, updateReflash},
};

BOOLEAN USER_RecvParse(u32 ip, u16 port, u8 *pUdpData, u32 dwLen)
{
    ASSERT_NULL_RETN(pUdpData, FALSE);
    
    APP_COM_PACKAGE *pUserPack = (APP_COM_PACKAGE *)pUdpData;

    if (dwLen < APP_COM_HEAD_LEN+2)
    {
        CT_PRINTF("get a packet less than head len\n");
        return FALSE;
    }
    // CRC error
    if (CRC_GetCrc16(pUdpData, dwLen - sizeof(u16)) != *(u16 *)&pUdpData[dwLen - sizeof(u16)])
    {
        CT_PRINTF("CRC check error\n");
        return FALSE;
    }
    
    if (pUserPack->wLen > MAX_PACKAGE_LEN)
    {
        CT_PRINTF("get a package longer than protocol\n");
        return FALSE;
    }
    
    if (MODULE_TYPE != pUserPack->byModType)
    {
        CT_PRINTF("get a wrong module message\n");
        return FALSE;
    }

    g_sAppDiagnose.dwEtheCnt++;
    u16 cmd_cnt = sizeof(gCmdExes) / sizeof(CmdExecutors);

    for (int i = 0; i < cmd_cnt; i++)
    {
        if (gCmdExes[i].cmd == pUserPack->wCmd && NULL != gCmdExes[i].exec)
        {
            gCmdExes[i].exec(ip, port, pUserPack);
            return TRUE;
        }
    }

    CT_PRINTF("Command[%d] error!", pUserPack->wCmd);
    return FALSE;
}


static void USER_Echo(u32 IPaddr, u16 UdpPort, APP_COM_PACKAGE *psRxdApp, u8 byCO, u8 *pUserData, u16 wAppLen)
{
    ASSERT_NULL_VOID(psRxdApp);
    //ASSERT_NULL_VOID(pUserData);

    APP_COM_PACKAGE *psTxdApp = (APP_COM_PACKAGE *)&s_byUserSendBuf[0];

    // set header
    memcpy((u8 *)psTxdApp, (u8 *)psRxdApp, APP_COM_HEAD_LEN);    

    // set command
    psTxdApp->byCO  = byCO;

    // set data length
    psTxdApp->wLen  = wAppLen;

    // set data
    if (wAppLen > 0)
    {
        memcpy((u8 *)&psTxdApp->byData[0], (u8 *)&pUserData[0], wAppLen);
    }

    // set crc
    *(u16 *)&psTxdApp->byData[wAppLen] = CRC_GetCrc16(s_byUserSendBuf, APP_COM_HEAD_LEN + wAppLen);

    UDP_SendData(IPaddr, UdpPort, s_byUserSendBuf, APP_COM_HEAD_LEN + wAppLen + 2);
}

static void getVersion(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    USER_Echo(ip, port, pUserPack, CO_NORMAL, (u8 *)VERSION_INFO, strlen(VERSION_INFO));
}

static void getRTCTime(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    RTC_GetSysRTC(&g_sSysRTC);
    USER_Echo(ip, port, pUserPack, CO_NORMAL, (u8 *)&g_sSysRTC, sizeof(g_sSysRTC));
}

static void getDiagnose(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    USER_Echo(ip, port, pUserPack, CO_NORMAL, (u8 *)&g_sAppDiagnose, sizeof(g_sAppDiagnose));
}

static void getConfig(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    USER_Echo(ip, port, pUserPack, CO_NORMAL, (u8 *)&g_sAppConfig, sizeof(g_sAppConfig));
}

static void getUserReal(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    USER_Echo(ip, port, pUserPack, CO_NORMAL, (u8 *)&g_sAppUserReal, sizeof(g_sAppUserReal));
}

static void readMem(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    u32 dwAddr  = *(u32 *)&pUserPack->byData[0];
    u16 wLength = *(u16 *)&pUserPack->byData[4];
    
    if ((6 == pUserPack->wLen) && (wLength <= (UDP_SEND_BUFSIZE - APP_COM_HEAD_LEN)))
    {
        USER_Echo(ip, port, pUserPack, CO_NORMAL, (u8 *)dwAddr, wLength);
    }
    else
    {
        USER_Echo(ip, port, pUserPack, CO_ERR_PARAMETER, NULL, 0);
    }
}

static void reset(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    USER_Echo(ip, port, pUserPack, CO_NORMAL, NULL, 0);
    delay_ms(100);
    Reset_CPU();
}

static void setTime(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    if (pUserPack->wLen == sizeof(RTC_TIME_TYPE))
    {
        RTC_SetSysRTC((RTC_TIME_TYPE *)&pUserPack->byData[0]);
        RTC_GetSysRTC(&g_sSysRTC);
        USER_Echo(ip, port, pUserPack, CO_NORMAL, (u8 *)&g_sSysRTC, sizeof(g_sSysRTC));
    }
    else
    {
        RTC_GetSysRTC(&g_sSysRTC);
        USER_Echo(ip, port, pUserPack, CO_ERR_PARAMETER, (u8 *)&g_sSysRTC, sizeof(g_sSysRTC));
    }
}

static void setConfig(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    if (pUserPack->wLen != sizeof(g_sAppConfig))
    {
        return;
    }

    STMFLASH_EraseSector(STMFLASH_GetFlashSector(APP_CONFIG_ADDR));
    STMFLASH_Write(APP_CONFIG_ADDR, (u32 *)pUserPack->byData, pUserPack->wLen / 4);

    INTX_DISABLE();
    memcpy((u8 *)&g_sAppConfig, (u8 *)APP_CONFIG_ADDR, sizeof(g_sAppConfig));
    INTX_ENABLE();

    USER_GetConfig(&g_sAppConfig);
    USER_Echo(ip, port, pUserPack, CO_NORMAL, (u8 *)&g_sAppConfig, sizeof(g_sAppConfig));
}

static void setCan1Tx(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    if (pUserPack->wLen != sizeof(APP_CAN_MSG))
    {
        USER_Echo(ip, port, pUserPack, CO_ERR_PARAMETER, NULL, 0);
        return;
    }

    INTX_DISABLE();
    BOOLEAN res = CAN_SendFrame(CAN_DEV1, (APP_CAN_MSG *)&pUserPack->byData[0]);
    INTX_ENABLE();

    USER_Echo(
        ip,
        port,
        pUserPack,
        res ? CO_NORMAL : CO_ERR_EXECUTE,
        pUserPack->byData,
        sizeof(APP_CAN_MSG));
}

static void setCan2Tx(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    if (pUserPack->wLen != sizeof(APP_CAN_MSG))
    {
        USER_Echo(ip, port, pUserPack, CO_ERR_PARAMETER, NULL, 0);
        return;
    }

    INTX_DISABLE();
    BOOLEAN res = CAN_SendFrame(CAN_DEV2, (APP_CAN_MSG *)&pUserPack->byData[0]);
    INTX_ENABLE();

    USER_Echo(
        ip,
        port,
        pUserPack,
        res ? CO_NORMAL : CO_ERR_EXECUTE,
        pUserPack->byData,
        sizeof(APP_CAN_MSG));
}

static void updatePrepare(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    OS_ERR os_err = OS_ERR_NONE;
    APP_USER_REALOUT *pRealOut = &g_sAppUserRealOut;
    
    BOOLEAN res = UP_PrepareForUpdate();
    /* improve UDP thread priority */
    if (res)
    {
        pRealOut->stIpcRsp.statusCode |= AGV_STATUS_UPGRADING;
        upgrading_tick = HAL_GetTick();
        //OSTaskChangePrio(NULL, UPGRADING_PAIO_UDP_THREAD, &os_err);
        //OSTaskChangePrio(&UDPSendTaskTCB, UPGRADING_PAIO_UDP_THREAD - 1u, &os_err);
        //OSTimeDly(200, OS_OPT_TIME_DLY, &os_err);
    }
    USER_Echo(ip, port, pUserPack, res ? CO_NORMAL : CO_ERR_EXECUTE, NULL, 0);
}

static void updateDownload(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    BOOLEAN res = UP_DownCode(pUserPack->wTotal, pUserPack->wSeq, pUserPack->byData, pUserPack->wLen);
    USER_Echo(ip, port, pUserPack, res ? CO_NORMAL : CO_ERR_PARAMETER, NULL, 0);
    upgrading_tick = HAL_GetTick();
}

static void updateUpcheck(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    USER_Echo(ip, port, pUserPack, CO_NORMAL, (u8 *)(APPCODE_SUB_ADDR + (u32)pUserPack->wSeq * DOWN_PER_SIZE), DOWN_PER_SIZE);
}

static void updateReflash(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    OS_ERR os_err = OS_ERR_NONE;
    APP_USER_REALOUT *pRealOut = &g_sAppUserRealOut;
    BOOLEAN res = *(u16 *)&pUserPack->byData[0] == 
        CRC_GetCrc16((u8 *)APPCODE_SUB_ADDR, (*(u16 *)&(pUserPack->byData[2])) * DOWN_PER_SIZE);
    
    /* resume UDP thread priority */
    //OSTaskChangePrio(NULL, UDP_DEFAULT_PAIO, &os_err);
    //OSTaskChangePrio(&UDPSendTaskTCB, UDP_DEFAULT_PAIO, &os_err);
    USER_Echo(ip, port, pUserPack, res ? CO_NORMAL : CO_ERR_PARAMETER, NULL, 0);
    //OSTimeDly(100, OS_OPT_TIME_DLY, &os_err);
    pRealOut->stIpcRsp.statusCode &= ~(u32)AGV_STATUS_UPGRADING;

    if (res)
    {
#if DEBUG_OUTPUT
		CT_PRINTF("upgrading\n");
#endif
        OSTimeDly(200, OS_OPT_TIME_DLY, &os_err);
        //delay_ms(500);
        Reset_to_Upgrade();
    }
}

static void ipcSync(u32 ip, u16 port, APP_COM_PACKAGE* pUserPack)
{
    INTX_DISABLE();
    g_sAppUserDevice.stIpcData.uTick = HAL_GetTick();
    memcpy((u8 *)&(g_sAppUserDevice.stIpcData.stIpcReq), pUserPack->byData, sizeof(IPC_Req));
    INTX_ENABLE();
}
