
#include "UART_Interface.h"
#include "delay.h"
#include "Protocal.h"
#include "os.h"
#include <string.h>

extern SerialPakage g_SerialPackRX ;
extern SerialPakage g_SerialPackTX ;
uint8_t* p_pack =  (uint8_t *)&g_SerialPackRX;

u8 rec_flag = 0;


//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)	
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->ISR&0X40)==0);//循环发送,直到发送完毕   
	USART1->TDR=(u8)ch;      
	return ch;
}
#endif 

APP_UART_MSG g_sUartAppRxd[UART_NUM];

UART_HandleTypeDef      UART_Handler[UART_NUM];
DMA_HandleTypeDef       UART1RxDMA_Handler;

static USART_TypeDef    *pUART_BASE_ADDR[UART_NUM] = {USART1, USART2, USART3, UART4, UART5, USART6 ,UART7, UART8};
static u8               s_byUartRxdBuf[UART_NUM][USART_RECLEN_TRIG_HOOK];


void UART_Init(u32 dwDevice, u32 dwbound, u32 WordLength, u32 dwStopBits, u32 dwParity)
{
    memset((u8 *)&g_sUartAppRxd[dwDevice], 0, sizeof(APP_UART_MSG));
    
    UART_Handler[dwDevice].Instance         = pUART_BASE_ADDR[dwDevice];
    UART_Handler[dwDevice].Init.BaudRate    = dwbound;                  
    UART_Handler[dwDevice].Init.WordLength  = WordLength;
    UART_Handler[dwDevice].Init.StopBits    = dwStopBits;
    UART_Handler[dwDevice].Init.Parity      = dwParity;
    UART_Handler[dwDevice].Init.HwFlowCtl   = UART_HWCONTROL_NONE;
    UART_Handler[dwDevice].Init.Mode        = UART_MODE_TX_RX;
    HAL_UART_Init(&UART_Handler[dwDevice]);
    HAL_UART_Receive_IT(&UART_Handler[dwDevice], (u8 *)&s_byUartRxdBuf[dwDevice][0], USART_RECLEN_TRIG_HOOK);
}

static void USART1_MspInit(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_Initure.Pin        = GPIO_PIN_9 | GPIO_PIN_10; 
    GPIO_Initure.Mode       = GPIO_MODE_AF_PP;
    GPIO_Initure.Pull       = GPIO_PULLUP; 
    GPIO_Initure.Speed      = GPIO_SPEED_FAST;
    GPIO_Initure.Alternate  = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_Initure); 
    
    __HAL_UART_ENABLE_IT(&UART_Handler[UART_DEV1], UART_IT_RXNE);
    HAL_NVIC_EnableIRQ(USART1_IRQn); 
    HAL_NVIC_SetPriority(USART1_IRQn, 3, 3); 
}

static u32 UART_GetDevice(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        return UART_DEV1;
    }
    else if (huart->Instance == USART2)
    {
        return UART_DEV2;
    }
    else if (huart->Instance == USART3)
    {
        return UART_DEV3;
    }
    else if (huart->Instance == UART4)
    {
        return UART_DEV4;
    }
    else if (huart->Instance == UART5)
    {
        return UART_DEV5;
    }
    else if (huart->Instance == USART6)
    {
        return UART_DEV6;
    }
    else if (huart->Instance == UART7)
    {
        return UART_DEV7;
    }
    else if (huart->Instance == UART8)
    {
        return UART_DEV8;
    }
    else
    {
        return 0xFF;
    }
}  

extern SerialPakage SerialPackRX;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    u16 wLen;
    u32 dwDevice;
    
    dwDevice = UART_GetDevice(huart);
    if (dwDevice >= UART_NUM)
    {
        return;
    }

    g_sUartAppRxd[dwDevice].dwTick = HAL_GetTick();
    wLen = g_sUartAppRxd[dwDevice].wRxdLen;
    
    if ((wLen+USART_RECLEN_TRIG_HOOK) <= USART_REC_MAXLEN)
    {
        if (USART1 == huart->Instance)
        {
            uint8_t data = s_byUartRxdBuf[dwDevice][0];
            static uint8_t flag = 0;
            static uint8_t data_len = 0;
            static uint8_t recv_len = 0;
            switch(flag)
            {
            /***moduleId***/
            case 0:
                //memset(&g_SerialPackRX, 0 , sizeof(SerialPakage));
                if(0x03 == data)
                {
                   p_pack =  (uint8_t *)&g_SerialPackRX;
                   memcpy(p_pack, &data, 1);
                   flag ++;
                }
                else 
                    flag = 0;
                break;
            case 1:
                if(0x9c == data)
                {
                   memcpy(++p_pack, &data, 1); 
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
                memcpy(++p_pack, &data, 1); 
                flag ++;
                break;
            case 3:
                memcpy(++p_pack, &data, 1);
                flag ++;                
                break;
            /***dataLen***/
            case 4:
                memcpy(++p_pack, &data, 1);
                data_len = data; 
                flag ++;
                break;
            /***recvLen***/
            case 5:
                memcpy(++p_pack, &data, 1); 
                if(0 == data_len)
                {
                   flag = 7;
                }
                else flag ++;  
                break;
            /***data***/
            case 6:
                if(recv_len < data_len)
                {
                   //g_SerialPackRX.head_.recvLen++; 
                    recv_len ++;
                    memcpy(++p_pack, &data, 1);
                }
                
                if(recv_len == data_len)
                {
                   flag = 7;
                }
                else if(recv_len > data_len)
                {
                  flag = 0;    //data error
                  recv_len = 0;
                  data_len = 0;
                  memset(&g_SerialPackRX, 0 , sizeof(SerialPakage));
                }
                break;                 
            /***crc L***/
            case 7:
                //memcpy(++p_pack, &data, 1);
                g_SerialPackRX.check_ |= (uint16_t)data;
                flag ++;
                break;
            /***crc H***/
            case 8:
                //memcpy(++p_pack, &data, 1);
                g_SerialPackRX.check_ |= (uint16_t)data << 8 ;
                recv_len = 0;
                data_len = 0;
                flag = 0;
                rec_flag = 1;
                break;
            case 53:
                break;
            }
        }
        else
        {
            ;
        }
    }
    else
    {
        ;
    }
}

void USART1_IRQHandler()                    
{
    OSIntEnter();
    HAL_UART_IRQHandler(&UART_Handler[UART_DEV1]);  
    HAL_UART_Receive_IT(&UART_Handler[UART_DEV1], (u8 *)&s_byUartRxdBuf[UART_DEV1][0], USART_RECLEN_TRIG_HOOK);
    OSIntExit();
} 

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        USART1_MspInit();
    }
//    else if (huart->Instance == USART2)
//    {
//        USART2_MspInit();
//    }
//    else if (huart->Instance == USART3)
//    {
//        USART3_MspInit();
//    }
//    else if (huart->Instance == UART4)
//    {
//        UART4_MspInit();
//    }
//    else if (huart->Instance == UART5)
//    {
//        UART5_MspInit();
//    }
//    else if (huart->Instance == USART6)
//    {
//        USART6_MspInit();
//    }
//    else
//    {
//        ;
//    }
}


void UART_DMA_init(UART_HandleTypeDef *uart_handler)
{
    if (NULL == uart_handler)
        return;
    if (USART1 == uart_handler->Instance)
    {
        __HAL_RCC_DMA2_CLK_ENABLE();
        
        //Tx DMA
//        __HAL_LINKDMA(uart_handler, hdmatx, UART1TxDMA_Handler);
//        UART2TxDMA_Handler.Instance=DMA1_Stream6;
//        UART2TxDMA_Handler.Init.Channel=DMA_CHANNEL_4;
//        UART2TxDMA_Handler.Init.Direction=DMA_MEMORY_TO_PERIPH;  //m->p
//        UART2TxDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;
//        UART2TxDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;
//        UART2TxDMA_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;
//        UART2TxDMA_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;
//        UART2TxDMA_Handler.Init.Mode=DMA_NORMAL;
//        UART2TxDMA_Handler.Init.Priority=DMA_PRIORITY_MEDIUM;
//        UART2TxDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;           
//        UART2TxDMA_Handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;    
//        UART2TxDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;
//        UART2TxDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;
//        
//        HAL_DMA_DeInit(&UART2TxDMA_Handler);   
//        HAL_DMA_Init(&UART2TxDMA_Handler);
//        
//        HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
//        HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 3, 4);

        //Rx DMA
      __HAL_LINKDMA(uart_handler, hdmarx, UART1RxDMA_Handler);
      UART1RxDMA_Handler.Instance=DMA2_Stream5;
      UART1RxDMA_Handler.Init.Channel=DMA_CHANNEL_4;
      UART1RxDMA_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;  //p->m
      UART1RxDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;
      UART1RxDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;
      UART1RxDMA_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;
      UART1RxDMA_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;
      UART1RxDMA_Handler.Init.Mode=DMA_NORMAL;
      UART1RxDMA_Handler.Init.Priority=DMA_PRIORITY_MEDIUM;
      UART1RxDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;           
      UART1RxDMA_Handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;    
      UART1RxDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;
      UART1RxDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;
      
      HAL_DMA_DeInit(&UART1RxDMA_Handler);   
      HAL_DMA_Init(&UART1RxDMA_Handler);
      
      HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
      HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 3, 4);
    }
//    else if (USART6 == uart_handler->Instance)
//    {
//        __HAL_RCC_DMA2_CLK_ENABLE();
//        //Tx DMA
//        __HAL_LINKDMA(uart_handler, hdmatx, UART6TxDMA_Handler);
//        UART6TxDMA_Handler.Instance=DMA2_Stream6;
//        UART6TxDMA_Handler.Init.Channel=DMA_CHANNEL_5;
//        UART6TxDMA_Handler.Init.Direction=DMA_MEMORY_TO_PERIPH;
//        UART6TxDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;
//        UART6TxDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;
//        UART6TxDMA_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;
//        UART6TxDMA_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;
//        UART6TxDMA_Handler.Init.Mode=DMA_NORMAL;
//        UART6TxDMA_Handler.Init.Priority=DMA_PRIORITY_HIGH;
//        UART6TxDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;           
//        UART6TxDMA_Handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;    
//        UART6TxDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;
//        UART6TxDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;
//        
//        HAL_DMA_DeInit(&UART6TxDMA_Handler);   
//        HAL_DMA_Init(&UART6TxDMA_Handler);
//        
//        HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
//        HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 3, 3);
        //Rx DMA
//      __HAL_LINKDMA(uart_handler, hdmarx, UART6RxDMA_Handler);
//      UART6RxDMA_Handler.Instance=DMA2_Stream1;
//      UART6RxDMA_Handler.Init.Channel=DMA_CHANNEL_5;
//      UART6RxDMA_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;  //p->m
//      UART6RxDMA_Handler.Init.PeriphInc=DMA_PINC_DISABLE;
//      UART6RxDMA_Handler.Init.MemInc=DMA_MINC_ENABLE;
//      UART6RxDMA_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;
//      UART6RxDMA_Handler.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;
//      UART6RxDMA_Handler.Init.Mode=DMA_NORMAL;
//      UART6RxDMA_Handler.Init.Priority=DMA_PRIORITY_HIGH;
//      UART6RxDMA_Handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;           
//      UART6RxDMA_Handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;    
//      UART6RxDMA_Handler.Init.MemBurst=DMA_MBURST_SINGLE;
//      UART6RxDMA_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;
//      
//      HAL_DMA_DeInit(&UART6RxDMA_Handler);   
//      HAL_DMA_Init(&UART6RxDMA_Handler);
//    }
}

void DMA_USART_Transmit(UART_HandleTypeDef *huart, DMA_HandleTypeDef *hmdatx, uint8_t *pData, uint16_t Size)
{
    u32 flag = 0;
    
    if (USART1 == huart->Instance)
    {
        flag = DMA_FLAG_TCIF3_7;
    }
//    else if (USART2 == huart->Instance)
//    {
//        flag = DMA_FLAG_TCIF2_6;
//    }
//    else if (USART3 == huart->Instance)
//    {
//        if (DMA1_Stream3 == hmdatx->Instance)
//        {
//            flag = DMA_FLAG_TCIF3_7;
//        }
//        else if (DMA1_Stream4 == hmdatx->Instance)
//        {
//            flag = DMA_FLAG_TCIF0_4;
//        }
//    }
//    else if (UART4 == huart->Instance)
//    {
//        flag = DMA_FLAG_TCIF0_4;
//    }
//    else if (UART5 == huart->Instance)
//    {
//        flag = DMA_FLAG_TCIF3_7;
//    }
//    else if (USART6 == huart->Instance)
//    {
//        if (DMA2_Stream6 == hmdatx->Instance)
//        {
//            flag = DMA_FLAG_TCIF2_6;
//        }
//        else if (DMA2_Stream7 == hmdatx->Instance)
//        {
//            flag = DMA_FLAG_TCIF3_7;
//        }
//    }
//    else
//    {
//    }
//  if(__HAL_DMA_GET_FLAG(&UART2TxDMA_Handler, flag))
//  {
//      __HAL_DMA_CLEAR_FLAG(&UART2TxDMA_Handler, flag);
//      HAL_UART_DMAStop(&UART_Handler[UART_DEV2]);
//  }   
    __HAL_DMA_CLEAR_FLAG(hmdatx, flag);
    HAL_UART_DMAStop(huart);
    //HAL_DMA_Start(huart->hdmatx, (u32)pData, (uint32_t)&huart->Instance->RDR, Size);
   
    //huart->Instance->CR3 |= USART_CR3_DMAT;
}


