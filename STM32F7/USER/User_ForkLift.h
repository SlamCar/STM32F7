/**
 * Copyright (c) 2018 CoTEK Inc. All rights reserved.
 */
 
#ifndef __USER_FORKLIFT_H
#define __USER_FORKLIFT_H

#include "User_Config.h"

// obstacle default speed parameter
#define OBSTACLE_SPEED_III          0u

#define OBSTACLE_SPEED_I_FACTOR     0.2f
#define OBSTACLE_SPEED_II_FACTOR    0.1f

#define CURTIS_MIN_SPEED            0.08f

#define VOICE_SMOOTH_BUFFER_SIZE    10u
#define VOICE_ZERO_SPEED_CNT        6u

//#define LOW_BATTERY_THRESHOLD       10u  //10%
#define LOW_BATTERY_THRESHOLD       23u  //23V

#define CURTIS_FATAL_ERROR_CODE     0x12u

#define UPLOAD_TIMEOUT_TIME         45000u  // 45s

void                    USER_RunForkLift(APP_USER_REALIN *pRealIn, APP_USER_REALOUT *pRealOut);
ObstacleLevel           USER_GetObstacleLevel(BOOLEAN bLvlI, BOOLEAN bLvlII, BOOLEAN bLvlIII);
ForkSensorStatus        USER_GetForkSensorStatus(BOOLEAN bLeft, BOOLEAN bRight);
PositionSensorStatus    USER_GetPositionSensorStatus(BOOLEAN bLeft, BOOLEAN bRight);
ForkliftStatus          USER_GetForkStatus(BOOLEAN bUpSide, BOOLEAN bDownSide);
BOOLEAN                 USER_CurtisSendData(APP_USER_REALOUT *pRealOut);

#endif
