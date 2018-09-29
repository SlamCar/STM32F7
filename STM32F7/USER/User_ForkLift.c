/**
 * Copyright (c) 2018 CoTEK Inc. All rights reserved.
 */
 
#include "User_ForkLift.h"
#include "User_Config.h"
#include "CurtisMotor.h"
#include "main.h"

//static float adaptSpeed2Obstacle(
//    float tar_speed,
//    float act_speed,
//    ObstacleLevel level,
//    CTColor *color,
//    AvoidStrategy stategy)
//{
//    float speed = 0.0;

//    switch (stategy)
//    {
//        case AVOID_STATEGY_NONE:
//        {
//            speed = tar_speed;
//            return speed;
//        }

//        case AVOID_STATEGY_I:
//        {
//            switch (level)
//            {
//                case Obstacle_WarnningI:
//                {
//                    speed = tar_speed * OBSTACLE_SPEED_I_FACTOR;
//                    *color = COLOR_YELLOW;
//                    break;
//                }

//                case Obstacle_WarnningII:
//                {
//                    speed = tar_speed * OBSTACLE_SPEED_II_FACTOR;
//                    *color = COLOR_YELLOW;
//                    break;
//                }

//                case Obstacle_Stop:
//                {
//                    speed = OBSTACLE_SPEED_III;                 
//                    *color = COLOR_RED;
//                    break;
//                }

//                case Obstacle_None:
//                {
//                    speed = tar_speed;
//                    *color = COLOR_GREEN;
//                    break;
//                }

//                default: break;
//            }
//            return speed;
//        }

//        case AVOID_STATEGY_II:
//        case AVOID_STATEGY_III:
//        {
//            switch (level)
//            {
//                case Obstacle_WarnningI:
//                {
//                    speed = tar_speed * OBSTACLE_SPEED_I_FACTOR;
//                    *color = COLOR_YELLOW;
//                    break;
//                }

//                case Obstacle_WarnningII:
//                {
//                    speed = tar_speed * OBSTACLE_SPEED_II_FACTOR;
//                    *color = COLOR_YELLOW;
//                    break;
//                }

//                case Obstacle_Stop:
//                {
//                    speed = OBSTACLE_SPEED_III;
//                    *color = COLOR_RED;
//                    break;
//                }

//                case Obstacle_None:
//                {
//                    speed = tar_speed;
//                    *color = COLOR_GREEN;
//                    break;
//                }

//                default: break;
//            }
//            return speed;
//        }

//        case AVOID_STATEGY_IV:
//        case AVOID_STATEGY_V:
//        {
//            switch (level)
//            {
//                case Obstacle_WarnningI:
//                {
//                    speed = tar_speed * OBSTACLE_SPEED_I_FACTOR;
//                    *color = COLOR_YELLOW;
//                    break;
//                }

//                case Obstacle_WarnningII:
//                {
//                    speed = tar_speed * OBSTACLE_SPEED_II_FACTOR;
//                    *color = COLOR_YELLOW;
//                    break;
//                }

//                case Obstacle_Stop:
//                {
//                    speed = OBSTACLE_SPEED_III;
//                    *color = COLOR_RED;
//                    break;
//                }

//                case Obstacle_None:
//                {
//                    speed = tar_speed;
//                    *color = COLOR_GREEN;
//                    break;
//                }
//                default: break;
//            }
//            return speed;
//        }

//        default: return 0;
//    }
//}

static float adaptSpeed2Obstacle(
    float tar_speed,
    float act_speed,
    ObstacleLevel level,
    AvoidStrategy stategy)
{
    float speed = 0.0;

	if (AVOID_STATEGY_NONE == stategy)
	{
		speed = tar_speed;
		return speed;
	}

	switch (level)
	{
		case Obstacle_WarnningI:
		{
			speed = tar_speed * OBSTACLE_SPEED_I_FACTOR;
			break;
		}

		case Obstacle_WarnningII:
		{
			speed = tar_speed * OBSTACLE_SPEED_II_FACTOR;
			break;
		}

		case Obstacle_Stop:
		{
			speed = OBSTACLE_SPEED_III;                 
			break;
		}

		case Obstacle_None:
		{
			speed = tar_speed;
			break;
		}

		default: break;
	}
	
	return speed;
}

static void voice_strategy_proc(
    float act_speed,
    float param_speed,
    spByf_Ctrl byf_ctrl)
{
    static float speed_buff[VOICE_SMOOTH_BUFFER_SIZE] = {0};

    static u8 cnt_act = 0;
    static u8 cnt_param = 0;

    if (act_speed <= F_ACCU && act_speed >= -F_ACCU)
    {
        cnt_act++;
        if (cnt_act >= VOICE_ZERO_SPEED_CNT)
        {
            cnt_act = 0;
            cnt_param = 0;
            byf_ctrl->cmd = BYF_CMD_STOP;
            return;
        }
    }
    else if (0 != cnt_act)
    {
        cnt_act = 0;
    }

    speed_buff[cnt_param++] = param_speed;

    if (cnt_param >= VOICE_SMOOTH_BUFFER_SIZE)
    {
        u8 strategy_0 = 0;
        u8 strategy_f = 0;
        u8 strategy_b = 0;
        int i = 0;
        
        for (i = 0; i < VOICE_SMOOTH_BUFFER_SIZE; ++i)
        {
            if (speed_buff[i] <= F_ACCU && speed_buff[i] >= -F_ACCU)
            {
                strategy_0++;
            }
            else if (speed_buff[i] > F_ACCU)
            {
                strategy_f++;
            }
            else
            {
                strategy_b++;
            }
        }

        if (strategy_0 >= VOICE_ZERO_SPEED_CNT)
        {
            byf_ctrl->cmd = BYF_CMD_STOP;
        }
        else if (VOICE_SMOOTH_BUFFER_SIZE == strategy_f)
        {
            byf_ctrl->cmd = BYF_CMD_SELECT;
            byf_ctrl->cast_type = v_normal;
        }
        else if (VOICE_SMOOTH_BUFFER_SIZE == strategy_b)
        {
            byf_ctrl->cmd = BYF_CMD_SELECT;
            byf_ctrl->cast_type = v_backward;
        }
    }
    cnt_param %= VOICE_SMOOTH_BUFFER_SIZE;
}

void USER_RunForkLift(APP_USER_REALIN *pRealIn, APP_USER_REALOUT *pRealOut)
{
    static BOOLEAN derail_flag = FALSE;
    TaskState proc_state = pRealIn->eCommand;

    //pRealOut->stIpcRsp.statusCode = AGV_STATUS_NORMAL;
    pRealOut->stIpcRsp.errorCode = AGV_NORMAL;
    pRealOut->stCurtisCtrlMsg.fSpeed = 0;
    pRealOut->stCurtisCtrlMsg.fAngle = 0;
    pRealOut->stCurtisCtrlMsg.bForkDown = 0;
    pRealOut->stCurtisCtrlMsg.bForkUp = 0;
    pRealOut->eColor = COLOR_GREEN;

    if (SET == pRealIn->bBumper)
    {
        pRealOut->stIpcRsp.errorCode = AGV_BUMPER_STOP;
        pRealOut->eColor = COLOR_RED;
        pRealOut->stByfCtrl.cmd = BYF_CMD_SELECT;
        pRealOut->stByfCtrl.cast_type = v_fault;
        return;
    }
	
	if (AGV_CONTROL_MODE_MANUAL == pRealIn->eAutoManual)
    {
        pRealOut->stIpcRsp.statusCode |= AGV_STATUS_MANUAL;
        pRealOut->eColor = COLOR_GREEN;
        pRealOut->stByfCtrl.cmd = BYF_CMD_STOP;
        pRealOut->bManual = TRUE;
        return;
    }

    else if (AGV_CONTROL_MODE_AUTO == pRealIn->eAutoManual)
    {
        pRealOut->bManual = FALSE;
        pRealOut->stIpcRsp.statusCode &= ~(u32)AGV_STATUS_MANUAL;
    }

    if (0 != pRealIn->bIpcErrorCode)
    {
        derail_flag = TRUE;
        pRealOut->stIpcRsp.errorCode = pRealIn->bIpcErrorCode;
        pRealOut->eColor = COLOR_RED;
        pRealOut->stByfCtrl.cmd = BYF_CMD_SELECT;
        pRealOut->stByfCtrl.cast_type = v_fault;
        return;
    }
    else if (derail_flag)
    {
        derail_flag = FALSE;
        pRealOut->stByfCtrl.cmd = BYF_CMD_STOP;
    }

    if (CURTIS_FATAL_ERROR_CODE == pRealIn->uCurtisError)
    {
        pRealOut->stIpcRsp.errorCode = AGV_CURTIS_FAULT;
        pRealOut->eColor = COLOR_RED;
        pRealOut->stByfCtrl.cmd = BYF_CMD_SELECT;
        pRealOut->stByfCtrl.cast_type = v_fault;
#if DEBUG_OUTPUT
		CT_PRINTF("curtis status code: %d\n", pRealIn->uCurtisError);
#endif
        return;
    }
    
    if (TRUE == pRealIn->conn_timeo)
    {
        pRealOut->stIpcRsp.statusCode |= AGV_STATUS_CONN_TIMEOUT;
    }
    else
    {
        pRealOut->stIpcRsp.statusCode &= ~(u32)AGV_STATUS_CONN_TIMEOUT;
    }
	
	if (Obstacle_Stop == pRealIn->eObstacleLevel && \
        (AVOID_STATEGY_NONE != pRealIn->eAvoidStrategy && 0 != pRealIn->eAvoidStrategy))
    {
        pRealOut->stIpcRsp.statusCode |= AGV_STATUS_OBSTACLE_STOP;
        pRealOut->eColor = COLOR_RED;
        pRealOut->stByfCtrl.cmd = BYF_CMD_SELECT;
        pRealOut->stByfCtrl.cast_type = v_obstacle_stop;
        return;
    }
    else
    {
        pRealOut->stIpcRsp.statusCode &= ~(u32)AGV_STATUS_OBSTACLE_STOP;
    }

    if (pRealIn->fCmdSpeed < 0 && FORK_SENSOR_OBSTACLE == pRealIn->eForkSensorStatus)
    {
        pRealOut->stIpcRsp.statusCode |= AGV_STATUS_OBSTACLE_STOP;
        pRealOut->eColor = COLOR_RED;
        pRealOut->stByfCtrl.cmd = BYF_CMD_SELECT;
        pRealOut->stByfCtrl.cast_type = v_obstacle_stop;
        return;
    }
    else
    {
        pRealOut->stIpcRsp.statusCode &= ~(u32)AGV_STATUS_OBSTACLE_STOP;
    }

    if (pRealIn->uResidualElectricity != 0 && pRealIn->uResidualElectricity < LOW_BATTERY_THRESHOLD)
    {
        pRealOut->stIpcRsp.errorCode = AGV_LOW_BATTERY;
        pRealOut->eColor = COLOR_YELLOW;
        pRealOut->stByfCtrl.cmd = BYF_CMD_SELECT;
        pRealOut->stByfCtrl.cast_type = v_low_battery;
		return;
    }
	
	if ((Obstacle_WarnningI == pRealIn->eObstacleLevel || Obstacle_WarnningII == pRealIn->eObstacleLevel) && \
		AVOID_STATEGY_NONE != pRealIn->eAvoidStrategy)
    {
        pRealOut->eColor = COLOR_YELLOW;
        //pRealOut->stByfCtrl.cmd = BYF_CMD_SELECT;
        //pRealOut->stByfCtrl.cast_type = v_obstacle_stop;
    }

    switch (proc_state)
    {
		case AGV_STATUS_TASK_RESTING:
        case AGV_STATUS_TASK_WAITING:
        {
            // Curtis reset
            if (pRealOut->bRelayResetFlag && IS_ZERO_F(pRealIn->fActualSpeed))
            {
                pRealOut->bRelayResetFlag = FALSE;
                pRealOut->bRelayCtrlFlag = TRUE;
                pRealOut->stIpcRsp.statusCode |= AGV_STATUS_CURTIS_RESET;
                CURTIS_RELAY_RESET();
            }
            if (pRealOut->bChargingFlag)
            {
                pRealOut->bChargingFlag = FALSE;
                DO_ChnOutput(DI_CHARGING, TRUE);
            }
            
			voice_strategy_proc(pRealIn->fActualSpeed,
                pRealOut->stCurtisCtrlMsg.fSpeed,
                &pRealOut->stByfCtrl);
        }

        case AGV_STATUS_TASK_DOING:
        {
            pRealOut->stCurtisCtrlMsg.fAngle = pRealIn->fCmdAngle;
            pRealOut->stCurtisCtrlMsg.fSpeed = pRealIn->fCmdSpeed;
            
            //pRealOut->stCurtisCtrlMsg.fAngle = pRealIn->fCmdAngle;

//            // param count should no more than 3
//            pRealOut->stCurtisCtrlMsg.fSpeed = adaptSpeed2Obstacle(
//                pRealIn->fCmdSpeed,
//                pRealIn->fActualSpeed,
//                pRealIn->eObstacleLevel,
//                pRealIn->eAvoidStrategy);

//            voice_strategy_proc(
//				pRealIn->fActualSpeed,
//                pRealOut->stCurtisCtrlMsg.fSpeed,
//                &pRealOut->stByfCtrl);
//            
//            if (pRealOut->bChargingFlag)
//            {
//                pRealOut->bChargingFlag = FALSE;
//                DO_ChnOutput(DI_CHARGING, TRUE);
//            }

            break;
        }

        case AGV_STATUS_ACTING_DOWNLOAD:
        {
            pRealOut->stCurtisCtrlMsg.fAngle = pRealIn->fCmdAngle;
            pRealOut->stCurtisCtrlMsg.fSpeed = adaptSpeed2Obstacle(
				pRealIn->fCmdSpeed,
                pRealIn->fActualSpeed,
                pRealIn->eObstacleLevel,
                pRealIn->eAvoidStrategy);

            if (IS_ZERO_F(pRealIn->fCmdSpeed) /*&& IS_ZERO_F(pRealIn->fActualSpeed)*/ \
                && FORK_DOWN != pRealIn->eForkliftStatus)
            {
                pRealOut->stCurtisCtrlMsg.bForkDown = 1;
                pRealOut->stCurtisCtrlMsg.bForkUp = 0;
            }

            voice_strategy_proc(
				pRealIn->fActualSpeed,
                pRealOut->stCurtisCtrlMsg.fSpeed,
                &pRealOut->stByfCtrl);

            break;
        }

        case AGV_STATUS_ACTING_UPLOAD:
        {
            static BOOLEAN up_timing_flag = FALSE;
            static u32 up_ticks = 0;
            
            pRealOut->stCurtisCtrlMsg.fAngle = pRealIn->fCmdAngle;
            pRealOut->stCurtisCtrlMsg.fSpeed = adaptSpeed2Obstacle(
                    pRealIn->fCmdSpeed,
                    pRealIn->fActualSpeed,
                    pRealIn->eObstacleLevel,
                    pRealIn->eAvoidStrategy);

            if (POSITION_SENSOR_ARRIVE == pRealIn->ePositionSensorStatus)
            {
                pRealOut->stCurtisCtrlMsg.fSpeed = 0;
                pRealOut->stCurtisCtrlMsg.fAngle = 0;

                // 1 sec brake
                pRealOut->stCurtisCtrlMsg.usDec = 1000;

                if (FORK_UP != pRealIn->eForkliftStatus)
                {
                    pRealOut->stCurtisCtrlMsg.bForkDown  = 0;
                    pRealOut->stCurtisCtrlMsg.bForkUp = 1;
                }
                if (up_timing_flag)
                {
                    up_timing_flag = FALSE;
                    pRealOut->stByfCtrl.cmd = BYF_CMD_STOP;
                    pRealOut->stIpcRsp.statusCode &= ~(u32)AGV_STATUS_FORKPOS_ABNORMAL;
                }
                
            }
            else if (FALSE == up_timing_flag)
            {
                up_timing_flag = TRUE;
                up_ticks = HAL_GetTick();
            }
            else if (up_timing_flag && (HAL_GetTick() - up_ticks > UPLOAD_TIMEOUT_TIME))
            {
                pRealOut->stIpcRsp.statusCode |= AGV_STATUS_FORKPOS_ABNORMAL;
                pRealOut->eColor = COLOR_YELLOW;
                pRealOut->stByfCtrl.cmd = BYF_CMD_SELECT;
                pRealOut->stByfCtrl.cast_type = v_obstacle_stop;
            }
            if (!(pRealOut->stIpcRsp.statusCode & AGV_STATUS_FORKPOS_ABNORMAL))
            {
                voice_strategy_proc(
                    pRealIn->fActualSpeed,
                    pRealOut->stCurtisCtrlMsg.fSpeed,
                    &pRealOut->stByfCtrl);
            }

            break;
        }
        
        case AGV_STATUS_CHARGING:
        {
            if (FALSE == pRealOut->bChargingFlag)
            {
                pRealOut->bChargingFlag = TRUE;
                DO_ChnOutput(DI_CHARGING, FALSE);
            }
            
            voice_strategy_proc(
				pRealIn->fActualSpeed,
                pRealOut->stCurtisCtrlMsg.fSpeed,
                &pRealOut->stByfCtrl);
            break;
        }

        default:
        {
            pRealOut->stIpcRsp.errorCode = AGV_INVALID_CMD;
#if DEBUG_OUTPUT
            CT_PRINTF("cmd: %d\n", proc_state);
#endif
            break;
        }
    }
}

ObstacleLevel USER_GetObstacleLevel(BOOLEAN bLvlI, BOOLEAN bLvlII, BOOLEAN bLvlIII)
{
    if (FALSE == bLvlIII)
    {
        return Obstacle_Stop;
    }

    if (FALSE == bLvlII)
    {
        return Obstacle_WarnningII;
    }

    if (FALSE == bLvlI)
    {
        return Obstacle_WarnningI;
    }

    return Obstacle_None;
}

ForkSensorStatus USER_GetForkSensorStatus(BOOLEAN bLeft, BOOLEAN bRight)
{
    return TRUE == bLeft || TRUE == bRight ? FORK_SENSOR_OBSTACLE : FORK_SENSOR_NORMAL;
}

PositionSensorStatus USER_GetPositionSensorStatus(BOOLEAN bLeft, BOOLEAN bRight)
{
    return TRUE == bLeft && TRUE == bRight? POSITION_SENSOR_ARRIVE : POSITION_SENSOR_NONE;
}

ForkliftStatus USER_GetForkStatus(BOOLEAN bUpSide, BOOLEAN bDownSide)
{
    if (TRUE == bDownSide)
    {
        return FORK_DOWN;
    }

    if (FALSE == bUpSide)
    {
        return FORK_UP;
    }

    return FORK_ERROR;
}

BOOLEAN USER_CurtisSendData(APP_USER_REALOUT *pRealOut)
{
	APP_CAN_MSG pCanTxd = {0};
	
	// send heartbeat
    Curtis_setHeartbeatMsg(&pCanTxd);

    if (FALSE == CAN_SendFrame(CURTIS_DEV, &pCanTxd))
    {
        return FALSE;
    }

    if (TRUE == pRealOut->bManual)
    {
        return TRUE;
    }

    // set curtis configurations
    Curtis_setConfigMsg(&(pRealOut->stCurtisCtrlMsg), &pCanTxd);

    if (FALSE == CAN_SendFrame(CURTIS_DEV, &pCanTxd))
    {
        return FALSE;
    }

    // send motor control param
    Curtis_setCtrlMsg(&(pRealOut->stCurtisCtrlMsg), &pCanTxd);

    if (FALSE == CAN_SendFrame(CURTIS_DEV, &pCanTxd))
    {
        return FALSE;
    }

    return TRUE;
}
