/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "OurCan_message.h"
//CAN_TxHeaderTypeDef   TxHeader;
//uint8_t               TxData[8];
//uint32_t              TxMailbox;

CAN_FilterTypeDef  sFilterConfig;   // 필터 설정 구조체 변수
CAN_RxHeaderTypeDef   RxHeader;
uint8_t               RxData[8];
DBMessages db_msg;

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 1;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void initCanDB(void)
{
    // Basic Control Messages
    db_msg.motor1_window.U = 0;
    db_msg.motor2_sunroof.U = 0;
    db_msg.heater.U = 0;
    db_msg.ac.U = 0;
    db_msg.audio.U = 0;

    // Battery Message
    db_msg.battery.U = 0;

    // Driver Control Messages
    db_msg.driver_window.U = 0;
    db_msg.driver_sunroof.U = 0;
    db_msg.driver_heater.U = 0;
    db_msg.driver_air.U = 0;
    db_msg.driver_engine.U = 0;
    db_msg.driver_control.U = 0;

    // Environment Sensor Messages
    db_msg.weather.U = 0;
    db_msg.dust.U = 0;
    db_msg.light.U = 0;
    db_msg.rain.U = 0;
    db_msg.db.U = 0;
    db_msg.TH_sensor.U = 0; // 모든 값 (플래그 포함) 초기화

    // Air Quality Messages
    db_msg.out_air_quality.U = 0;
    db_msg.in_air_quality.U = 0;

    // Smart Control Messages
    db_msg.smart_window.U = 0;
    db_msg.smart_sunroof.U = 0;
    db_msg.smart_heater.U = 0;
    db_msg.smart_ac.U = 0;
    db_msg.smart_audio.U = 0;

    // Safety Control Messages
    db_msg.safety_window.U = 0;
    db_msg.safety_sunroof.U = 0;
}


void set_can_filter(void)
{
	// 첫 번째 필터 뱅크
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
	sFilterConfig.FilterIdHigh = (0x748 << 5);
	sFilterConfig.FilterIdLow = (0x722 << 5);
	sFilterConfig.FilterMaskIdHigh = (0x721 << 5);
	sFilterConfig.FilterMaskIdLow = (0x724 << 5);
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
	{
	    Error_Handler();
	}

	// 두 번째 필터 뱅크
	sFilterConfig.FilterBank = 1;
	sFilterConfig.FilterIdHigh = (0x725 << 5);
	sFilterConfig.FilterIdLow = (0x726 << 5);
	sFilterConfig.FilterMaskIdHigh = (0x720 << 5);
	sFilterConfig.FilterMaskIdLow = (0x732 << 5);

	if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
	{
	    Error_Handler();
	}

	// 세 번째 필터 뱅크
	sFilterConfig.FilterBank = 2;
	sFilterConfig.FilterIdHigh = (0x731 << 5);
	sFilterConfig.FilterIdLow = (0x734 << 5);
	sFilterConfig.FilterMaskIdHigh = (0x735 << 5);
	sFilterConfig.FilterMaskIdLow = (0x733 << 5);

	if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
	{
	    Error_Handler();
	}

	// 네 번째 필터 뱅크
	sFilterConfig.FilterBank = 3;
	sFilterConfig.FilterIdHigh = (0x712 << 5);
	sFilterConfig.FilterIdLow = (0x711 << 5);
	sFilterConfig.FilterMaskIdHigh = (0x751 << 5);
	sFilterConfig.FilterMaskIdLow = 0x0000;

	if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
	{
	    Error_Handler();
	}
}

void start_can(void)
{
	if (HAL_CAN_Start(&hcan) != HAL_OK) {
		/* Start Error */
		//printf("CAN Start Error Code: %d\n", hcan.ErrorCode);
		Error_Handler();
	}

	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING)
			!= HAL_OK)

	{
		/* Notification Error */
		//printf("CAN Start Error Code: %d\n", hcan.ErrorCode);
		Error_Handler();
	}
	printf("Can Ready!!\r\n");

}

void can_send_test(void)
{
	CAN_TxHeaderTypeDef   TxHeader = {0};
	uint8_t               TxData[8] = {0};
	uint32_t              TxMailbox = 0;

	TxHeader.StdId = 0x434;                 // Standard Identifier, 0 ~ 0x7FF
	TxHeader.ExtId = 0x01;                // Extended Identifier, 0 ~ 0x1FFFFFFF
	TxHeader.RTR = CAN_RTR_DATA;            // 전송하는 메세지의 프레임 타입, DATA or REMOTE
	TxHeader.IDE = CAN_ID_STD;              // 전송하는 메세지의 식별자 타입, STD or EXT
	TxHeader.DLC = 8;                       // 송신 프레임 길이, 0 ~ 8 byte
	TxHeader.TransmitGlobalTime = DISABLE; // 프레임 전송 시작될 때 timestamp counter 값을 capture.

	/* Set the data to be transmitted */
	TxData[0] = 1;
	TxData[1] = 2;
	TxData[2] = 3;
	TxData[3] = 4;
	TxData[4] = 5;
	TxData[5] = 6;
	TxData[6] = 7;
	TxData[7] = 8;

	/* Start the Transmission process */

	if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
		printf("Can Send Fail\r\n");
		Error_Handler();
	}
	printf("Can Send Success\r\n");

}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *CanHandle) {
	//printf("%s\r\n", __FUNCTION__);
	/* Get RX message */
	if (HAL_CAN_GetRxMessage(CanHandle, CAN_RX_FIFO0, &RxHeader, RxData)
			!= HAL_OK) {
		/* Reception Error */
		Error_Handler();
	}

	//printf("StdID: %04lx, IDE: %ld, DLC: %ld\r\n", RxHeader.StdId, RxHeader.IDE,
			//RxHeader.DLC);
//	printf("Data: %d %d %d %d %d %d %d %d\r\n", RxData[0], RxData[1], RxData[2],
//			RxData[3], RxData[4], RxData[5], RxData[6], RxData[7]);

	uint32_t msg_id = RxHeader.StdId;

	switch(msg_id)
	{
		case SAFETY_SUNROOF_MSG_ID: //711
		{
			db_msg.safety_sunroof.U = 0;
			for(int i=0;i<8;i++){
				db_msg.safety_sunroof.U |= RxData[i] >> (i * 8);

			}
			db_msg.safety_sunroof.B.Flag = 1;

			break;
		}
		case SAFETY_WINDOW_MSG_ID: //712
		{
			db_msg.safety_window.U = 0;
			for(int i=0;i<8;i++){
				db_msg.safety_window.U |= RxData[i] >> (i * 8);

			}
			db_msg.safety_window.B.Flag = 1;

			break;
		}
		case DRIVER_CTRL_MSG_ID: //720
		{
			db_msg.driver_control.U = 0;
			for(int i=0;i<8;i++){
				db_msg.driver_control.U |= RxData[i] >> (i * 8);

			}
			db_msg.driver_control.B.Flag = 1;

			break;
		}
		case DRIVER_SUNROOF_MSG_ID: // 721
		{
			db_msg.driver_sunroof.U = 0;
			for(int i=0;i<8;i++){
				db_msg.driver_sunroof.U |= RxData[i] >> (i * 8);

			}
			db_msg.driver_sunroof.B.Flag = 1;

			break;
		}
		case DRIVER_WINDOW_MSG_ID: //722
		{
			db_msg.driver_window.U = 0;
			for(int i=0;i<8;i++){
				db_msg.driver_window.U |= RxData[i] >> (i * 8);

			}
			db_msg.driver_window.B.Flag = 1;
			//printf("dd %d \r\n", db_msg.driver_window.B.driver_window);

			break;
		}
		case DRIVER_HEATER_MSG_ID: // 724
		{
			db_msg.driver_heater.U = 0;
			for(int i=0;i<8;i++){
				db_msg.driver_heater.U |= RxData[i] >> (i * 8);

			}
			db_msg.driver_heater.B.Flag = 1;

			break;
		}
		case DRIVER_AIR_MSG_ID: //725
		{
			db_msg.driver_air.U = 0;
			for(int i=0;i<8;i++){
				db_msg.driver_air.U |= RxData[i] >> (i * 8);

			}
			db_msg.driver_air.B.Flag = 1;

			break;
		}
		case DRIVER_ENGINE_MSG_ID: //726
		{
			db_msg.driver_engine.U = 0;
			for(int i=0;i<8;i++){
				db_msg.driver_engine.U |= RxData[i] >> (i * 8);

			}
			db_msg.driver_engine.B.Flag = 1;
			break;
		}
		case SMART_SUNROOF_MSG_ID: //731
		{
			db_msg.smart_sunroof.U = 0;
			for(int i=0;i<8;i++){
				db_msg.smart_sunroof.U |= RxData[i] >> (i * 8);

			}
			db_msg.smart_sunroof.B.Flag = 1;

			break;
		}
		case SMART_WINDOW_MSG_ID: //732
		{
			db_msg.smart_window.U = 0;
			for(int i=0;i<8;i++){
				db_msg.smart_window.U |= RxData[i] >> (i * 8);

			}
			db_msg.smart_window.B.Flag = 1;

			break;
		}
		case SMART_AUDIO_MSG_ID: //733
		{
			db_msg.smart_audio.U = 0;
			for(int i=0;i<8;i++){
				db_msg.smart_audio.U |= RxData[i] >> (i * 8);

			}
			db_msg.smart_audio.B.Flag = 1;

			break;
		}
		case SMART_HEAT_MSG_ID: //734
		{
			db_msg.smart_heater.U = 0;
			for(int i=0;i<8;i++){
				db_msg.smart_heater.U |= RxData[i] >> (i * 8);
			}
			db_msg.smart_heater.B.Flag = 1;

			break;
		}
		case SMART_AC_MSG_ID: //735
		{
			db_msg.smart_ac.U = 0;
			for(int i=0;i<8;i++){
				db_msg.smart_ac.U |= RxData[i] >> (i * 8);
			}
			db_msg.smart_ac.B.Flag = 1;

			break;
		}
		case LIGHT_MSG_ID: //748
		{
			db_msg.light.U = 0;
			for(int i=0;i<8;i++){
				db_msg.light.U |= RxData[i] >> (i * 8);

			}
			db_msg.light.B.Flag = 1;

			//printf("dd %d %d \r\n", db_msg.light.B.Light_alive,db_msg.light.B.Light_pct);
			break;
		}
//		case 0x751:
//		{
//
//			for(int i=0;i<8;i++){
//				msgTest.U |= RxData[i]>>(i*8);
//			}
//
//
//			printf("Data: %d %d %d\r\n", msgTest.B.motor1_alive, msgTest.B.motor1_running, msgTest.B.motor1_tick_counter);
//
//
//			break;
//		}

	}
}
void output_message(void *msg, uint32_t msgID) {
	CAN_TxHeaderTypeDef   TxHeader = {0};
	uint8_t               TxData[8] = {0};
	uint32_t              TxMailbox = 0;

	TxHeader.StdId = msgID;                // Standard Identifier, 0 ~ 0x7FF
	TxHeader.ExtId = 0x01;            // Extended Identifier, 0 ~ 0x1FFFFFFF
	TxHeader.RTR = CAN_RTR_DATA;         // 전송하는 메세지의 프레임 타입, DATA or REMOTE
	TxHeader.IDE = CAN_ID_STD;              // 전송하는 메세지의 식별자 타입, STD or EXT
	TxHeader.DLC = 8;                       // 송신 프레임 길이, 0 ~ 8 byte
	TxHeader.TransmitGlobalTime = DISABLE; // 프레임 전송 시작될 때 timestamp counter 값을 capture.

	//printf("Can Send Success\r\n");

	switch (msgID) {
	case MOTOR1_WINDOW_MSG_ID: //752
	{
		OurCanMotor1Window *motor_msg = (OurCanMotor1Window*) msg;
		for (int i = 0; i < 8; i++) {
			TxData[i] = (motor_msg->U >> (i) * 8) & 0xFF;
		}
		break;
	}
	case MOTOR2_SUNROOF_MSG_ID: //751
	{
		OurCanMotor2Sunroof *motor_msg = (OurCanMotor2Sunroof*) msg;
		for (int i = 0; i < 8; i++) {
			TxData[i] = (motor_msg->U >> (i) * 8) & 0xFF;
		}
		break;
	}
	case HEATER_MSG_ID: //754
	{
		OurCanHeater *heat_msg = (OurCanHeater*) msg;
		for (int i = 0; i < 8; i++) {
			TxData[i] = (heat_msg->U >> (i) * 8) & 0xFF;
		}
		break;
	}
	case AIRCONDITIONER_MSG_ID: //755
	{
		OurCanAC *ac_msg = (OurCanAC*) msg;
		for (int i = 0; i < 8; i++) {
			TxData[i] = (ac_msg->U >> (i) * 8) & 0xFF;
		}
		break;
	}
	case AUDIO_MSG_ID: //753
	{
		OurCanAudio *audio_msg = (OurCanAudio*) msg;
		for (int i = 0; i < 8; i++) {
			TxData[i] = (audio_msg->U >> (i) * 8) & 0xFF;
		}
		break;
	}
	case BATTERY_MSG_ID: //76F
	{
		OurCanBattery *battery_msg = (OurCanBattery*) msg;
		for (int i = 0; i < 8; i++) {
			TxData[i] = (battery_msg->U >> (i) * 8) & 0xFF;
		}
		break;
	}
	default: {
		return;
	}
	}

	if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox)
			!= HAL_OK) {
		printf("Can Send Fail\r\n");
		Error_Handler();
	}

}

/* USER CODE END 1 */
