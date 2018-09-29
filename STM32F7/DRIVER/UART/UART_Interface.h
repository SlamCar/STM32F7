
#ifndef __UART_INTERFACE_H
#define __UART_INTERFACE_H

#include "sys.h"

#define UART_NUM                0x08u

#define UART_DEV1               0x00u
#define UART_DEV2               0x01u
#define UART_DEV3               0x02u
#define UART_DEV4               0x03u
#define UART_DEV5               0x04u
#define UART_DEV6               0x05u
#define UART_DEV7               0x06u
#define UART_DEV8               0x07u

#define USART_RECLEN_TRIG_HOOK  0x01u   // HAL库里设定触发产生中断回调数据长度，
#define USART_REC_MAXLEN        0x100u  // max recive datalength  256 BYPE

typedef struct
{
    u32 dwTick;
    u8  byRsv0; 
    u8  byRsv1;
    u16 wRxdLen;
    u8  byRxdBuf[USART_REC_MAXLEN]; 
 }APP_UART_MSG;

extern APP_UART_MSG          g_sUartAppRxd[UART_NUM];
extern UART_HandleTypeDef    UART_Handler[];
extern DMA_HandleTypeDef     UART1RxDMA_Handler;

 
void UART_Init(u32 dwDevice, u32 dwbound, u32 WordLength, u32 dwStopBits, u32 dwParity);
void UART_SendByte(u32 dwDevice, u8 *pData, u16 wLen, u32 dwTimeOut);
void UART_RecvDataProc(u32 dwDevice, u8 *pRxdData, u16 wRxdLen);
 
void UART_DMA_init(UART_HandleTypeDef *uart_handler);
void DMA_USART_Transmit(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hmdatx, uint8_t *pData, uint16_t Size);
 
#endif
 
