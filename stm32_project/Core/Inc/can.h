/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "OurCan_message.h"
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */
//extern CAN_TxHeaderTypeDef   TxHeader;
//extern uint8_t               TxData[8];
//extern uint32_t              TxMailbox;

extern CAN_FilterTypeDef  sFilterConfig;   // 필터 설정 구조체 변수
extern CAN_RxHeaderTypeDef   RxHeader;
extern uint8_t               RxData[8];

typedef struct
{
    OurCanMotor1Window motor1_window; //0x752
    OurCanMotor2Sunroof motor2_sunroof; //0x751
    OurCanHeater heater; //0x754
    OurCanAC ac; //0x755
    OurCanAudio audio; //0x753
    OurCanBattery battery; //0x76F
    OurCanDriverWindow driver_window; //722
    OurCanDriverSunroof driver_sunroof; //721
    OurCanDriverHeater driver_heater; //724
    OurCanDriverAir driver_air; //725
    OurCanDriverEngine driver_engine; //726
    OurCanDriverControl driver_control; // 720
    OurCanWeather weather; //0x74D
    OurCanDust dust; //0x74B
    OurCanLight light; //0x748
    OurCanRain rain; //0x747
    OurCanDB db; //0x74E
    OurCanTHSensor TH_sensor; //0x74C
    OurCanOutAirQuality out_air_quality; //0x74A
    OurCanInAirQuality in_air_quality; // 0x749
    OurCanSmartWindow smart_window; //732
    OurCanSmartSunroof smart_sunroof; //731
    OurCanSmartHeater smart_heater; //734
    OurCanSmartAC smart_ac; //735
    OurCanSmartAudio smart_audio; //733
    OurCanSafetyWindow safety_window; //712
    OurCanSafetySunroof safety_sunroof; // 711
} DBMessages;

extern DBMessages db_msg;



/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */
void can_send_test(void);
void start_can(void);
void set_can_filter(void);
void initCanDB(void);
void output_message(void *msg, uint32_t msgID);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *CanHandle);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

