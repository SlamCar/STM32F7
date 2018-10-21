#include "sys.h"
#include "Data_Base.h"
#include "delay.h"
#include "includes.h"
#include "led.h"
#include "key.h"
#include "Protocal.h"
#include "UART_Interface.h"
#include "transport.h"

#define DEBUG 0
  
SerialPakage g_SerialPackRX = {0};
SerialPakage g_SerialPackTX = {0};

extern u8 rec_flag ;


//任务优先级
#define START_TASK_PRIO         3
//任务堆栈大小	
#define START_STK_SIZE          100
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);


/* Usart task */
#define COMMUNCIATE_TASK_PRIO         4
#define COMMUNCIATE_STK_SIZE          128
OS_TCB COMMUNCIATETaskTCB;
CPU_STK COMMUNCIATE_TASK_STK[COMMUNCIATE_STK_SIZE];
void Communicate_task(void *p_arg);
void Data_Process(SerialPakage *SerialPack);

/* Manual task */
#define MANUAL_TASK_PRIO        4
#define MANUAL_STK_SIZE         256
OS_TCB MANUALTaskTCB;
CPU_STK MANUAL_TASK_STK[MANUAL_STK_SIZE];
void Manual_task(void *p_arg);


int main(void)
{
    
    OS_ERR err;
    CPU_SR_ALLOC();
    
    Write_Through();                //透写
    Cache_Enable();                 //打开L1-Cache
    HAL_Init();				        //初始化HAL库
    Stm32_Clock_Init(432,25,2,9);   //设置时钟,216Mhz 
    delay_init(216);                //延时初始化

    OSInit(&err);		            //初始化UCOSIII
    OS_CRITICAL_ENTER();            //进入临界区 
    
    //创建开始任务
    OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
                (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP, //任务选项,为了保险起见，所有任务都保存浮点寄存器的值
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
    OS_CRITICAL_EXIT();	//退出临界区	 
    OSStart(&err);      //开启UCOSIII
    while(1)
    {
    } 
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,设置默认的时间片长度s
	OSSchedRoundRobinCfg(DEF_ENABLED,10,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//进入临界区
         
    /* Usart task */
    OSTaskCreate((OS_TCB 	* )&COMMUNCIATETaskTCB,
                 (CPU_CHAR	* )"Communicate_task", 
                 (OS_TASK_PTR )Communicate_task,
                 (void		* )0,
                 (OS_PRIO	  )COMMUNCIATE_TASK_PRIO,     
                 (CPU_STK   * )&COMMUNCIATE_TASK_STK[0],
                 (CPU_STK_SIZE)COMMUNCIATE_STK_SIZE/10,
                 (CPU_STK_SIZE)COMMUNCIATE_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK	  )0,
                 (void   	* )0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP, 
                 (OS_ERR 	* )&err);
                 
    /* Manual task */
    OSTaskCreate((OS_TCB 	* )&MANUALTaskTCB,
                 (CPU_CHAR	* )" Manual_task", 
                 (OS_TASK_PTR )Manual_task,
                 (void		* )0,
                 (OS_PRIO	  )MANUAL_TASK_PRIO,     
                 (CPU_STK   * )&MANUAL_TASK_STK[0],
                 (CPU_STK_SIZE)MANUAL_STK_SIZE/10,
                 (CPU_STK_SIZE)MANUAL_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK	  )0,
                 (void   	* )0,
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP, 
                 (OS_ERR 	* )&err);

    OS_CRITICAL_EXIT();//进入临界区 
    OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);//挂起开始任务
}

void Communicate_task(void *p_arg)
{
    u8 t = 0;
    OS_ERR err = OS_ERR_NONE;
    //UNSED(p_arg);
    LED_Init();
    UART_Init(UART_DEV1, 115200u, UART_WORDLENGTH_8B, UART_STOPBITS_1, UART_PARITY_NONE);
    // wait for serial communication to be established successfully(IPC running)
   // OSTimeDlyHMSM(0, 0, 3, 0, OS_OPT_TIME_DLY, &err);
    //printf("\r\n wait IPC...... \r\n");
    
    while(1) 
    {
        OSTimeDly(10, OS_OPT_TIME_PERIODIC, &err);
        #if 1
        
        if(feedMsgPack(db_feedbackMsg))
        {
            // 大小端转化
            //EndianTrans();
            //  send data 
//            while(HAL_UART_Transmit(&UART_Handler[UART_DEV1],(uint8_t *)&g_SerialPackTX, 
//                              HEAD_BYTESIZE + BODY_MAX_BYTESIZE + CRC_BYTESIZE ,1000) != HAL_OK);
           HAL_UART_Transmit(&UART_Handler[UART_DEV1],(uint8_t *)&g_SerialPackTX, 
                              HEAD_BYTESIZE + BODY_MAX_BYTESIZE + CRC_BYTESIZE ,1000);
            
           while(__HAL_UART_GET_FLAG(&UART_Handler[UART_DEV1],UART_FLAG_TC)!=SET);    //wait  untill send end 
            memset(&g_SerialPackTX, 0 , sizeof(SerialPakage));     //clear data
        }
        #endif
//        if(db_cmd_update)
//        {    
//            //大小端转化
//            EndianTrans();
//            switch(g_SerialPackRX.head_.dataId)
//            {
//                case CMD_IPC_COMMOND:
//                    //g_SerialPackRX --> db_controlMsg
//                    memcpy((uint8_t *)&db_controlMsg, &g_SerialPackRX.byData_, sizeof(Control_Msg));
//                    break;
//                
//                case DEBUG_QT_COMMOND:
//                    //g_SerialPackRX --> db_controlMsg
//                    memcpy((uint8_t *)&db_controlMsg, &g_SerialPackRX.byData_, sizeof(Control_Msg));
//                    break;
//            }                
//            //database finish update
//            db_cmd_update = FALSE;
//        }
#if DEBUG
        if(rec_flag)
        {
            #if 1
            EndianTrans();
            printf("\r\n********************\r\n");
            printf("\r\n ____Head____ \r\n\r\n"); 
            printf(" moduleId: %02X\r\n",g_SerialPackRX.head_.moduleId);
            printf(" dataId: %01X\r\n",g_SerialPackRX.head_.dataId);
            printf(" dataLen: %01X\r\n",g_SerialPackRX.head_.dataLen);
            
            printf("\r\n ____Data____ \r\n\r\n");
            printf(" byData:");           
            for(t=0; t<g_SerialPackRX.head_.dataLen; ++t)
            {
                printf("%01X",g_SerialPackRX.byData_[t]);
            }
            printf("\r\n");
            printf("\r\n ____Crc____ \r\n\r\n");
            printf(" check: %02X\r\n",g_SerialPackRX.check_);
            printf("\r\n********************\r\n");            
            #endif
            
            #if 0
            HAL_UART_Transmit(&UART_Handler[UART_DEV1],(uint8_t *)&g_SerialPackRX,g_SerialPackRX.head_.dataLen+6,1000); //  send data
            HAL_UART_Transmit(&UART_Handler[UART_DEV1],(uint8_t *)&g_SerialPackRX.check_,2,1000);       //send  crc
            while(__HAL_UART_GET_FLAG(&UART_Handler[UART_DEV1],UART_FLAG_TC)!=SET);               //wait  untill send end
            #endif
            
            rec_flag = 0;
            memset(&g_SerialPackRX, 0 , sizeof(SerialPakage));     //clear data
        }
#endif
        OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_HMSM_STRICT,&err); //延时200ms
    }
}

void Manual_task(void *p_arg)
{
    OS_ERR err = OS_ERR_NONE;
    u8 key = 0;
    
    db_feedbackMsg.now_speed = 0.0;
    db_feedbackMsg.now_angle = 0.0;
    
    KEY_Init();
    while(1)
    {
        
        OSTimeDly(10u, OS_OPT_TIME_PERIODIC, &err);
        key = KEY_Scan(0);
        switch(key)
        {
            /***feedback test***/
            case KEY0_PRES: db_feedbackMsg.now_speed ++;
                break;
            case KEY1_PRES: db_feedbackMsg.now_angle ++;
                break;
            case KEY2_PRES: db_feedbackMsg.now_speed --;
                break;
            case WKUP_PRES: db_feedbackMsg.now_angle --;
                break;  
        }
    }
}

void Data_Process(SerialPakage *SerialPackRX)
{
    ASSERT_NULL_VOID(SerialPackRX);
        
    switch(SerialPackRX->head_.dataId)        
    {
        case CMD_IPC_COMMOND:
            LED0(0);
            //memcpy(&g_APP_USER_Input.control_msg,SerialPack->byData_,SerialPack->head_.dataLen);
            break; 
        
        case DEBUG_QT_COMMOND:
            LED0(1);
            break;
        
        //default:
        //    printf("\r\nInvalid Command !!!\r\n");
    }
    SerialPackRX = NULL;
}
