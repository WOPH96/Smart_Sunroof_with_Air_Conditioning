/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "DFPlayer.h"
#include "hvac.h"
#include "window.h"
#include "sunroof.h"
#include "STM_LCD16X2.h"
#include "LCD_Logic.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MOTOR_SHIELD_IN_PIN GPIO_PIN_8 // 예: PA0 핀
#define MOTOR_SHIELD_IN_PORT GPIOA

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void AppScheduling(void);
void AppTask1ms(void);
void AppTask10ms(void);
void AppTask100ms(void);
void AppTask1000ms(void);

typedef struct
{
    uint32_t u32nuCnt1ms;
    uint32_t u32nuCnt10ms;
    uint32_t u32nuCnt100ms;
    uint32_t u32nuCnt1000ms;

} Taskcnt;

uint8_t actuator_power = 0;  //시동 켜지면 1

//창문
int safety_win=0;
int motor1_smart=0;
int motor1_smart_pct=0;
int override_win=0;
int car_mode=0;
int motor1_smart_flag=0;
int override_flag_win=0;
int safety_win_flag=0;
//썬루프
int saftey_sun=0;
int override_flag_sun=0;
int safety_sun_flag=0;
int motor2_smart_flag=0;
int override_sun=0;
int motor2_smart=0;

Taskcnt stTestCnt;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart2, (uint8_t*) ptr, (uint16_t) len, 100);

	return (len);
}




//extern CAN_TxHeaderTypeDef TxHeader;
//extern uint8_t TxData[8];
//extern uint32_t TxMailbox;
//
//extern CAN_FilterTypeDef sFilterConfig;   // 필터 설정 구조체 변수
//extern CAN_RxHeaderTypeDef RxHeader;
//extern uint8_t RxData[8];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_CAN_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim4);//sub timer 1sec
	DF_Init(16);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


	set_can_filter();

	//CAN
	start_can();
	initCanDB();

	//LCD
	init_LCD();
	init_vehicle_state();

	Window_Init();
	Sunroof_Init();

	printf("Start\r\n");

	while (1) {
		AppScheduling();

		//
	      if (db_msg.driver_window.B.Flag == 1) // on message TH_sensor 느낌
	      {                                 // ISR이 센서 값 받았다면.
	    	  override_flag_win=1;
	    	  db_msg.driver_window.B.Flag = 0;  // 필수 처리!
	          override_win=db_msg.driver_window.B.driver_window;
         // 처리 로직 예시

	      }
		//721
		if (db_msg.driver_sunroof.B.Flag == 1) // on message TH_sensor 느낌
		{                                 // ISR이 센서 값 받았다면.
			override_flag_sun=1;
			db_msg.driver_sunroof.B.Flag = 0;  // 필수 처리!
			override_sun = db_msg.driver_window.B.driver_window;
			// 처리 로직 예시

		}
		  // 724
		  if (db_msg.driver_heater.B.Flag == 1)
		  {
			  db_msg.driver_heater.B.Flag = 0;
			  if (actuator_power == 1)
				  driver_heater(db_msg.driver_heater.B.driver_heater);
		  }
		  // 725
		  if (db_msg.driver_air.B.Flag == 1)
		  {
			  db_msg.driver_air.B.Flag = 0;
			  if (actuator_power == 1)
				  driver_ac(db_msg.driver_air.B.driver_air);
		  }
		  // 726 시동
		  if (db_msg.driver_engine.B.Flag == 1)
		  {
			  db_msg.driver_engine.B.Flag = 0;

			  // 시동 OFF
			  if (db_msg.driver_engine.B.engine_mode == 0)
			  {
				  off_heater();
				  off_ac();
				  actuator_power = 0;
			  }
			  // 저전력
			  else if (db_msg.driver_engine.B.engine_mode == 1)
			  {
				  actuator_power = 1;
			  }
			  // 시동 ON
			  else if (db_msg.driver_engine.B.engine_mode == 2)
			  {
				  actuator_power = 1;
			  }

		  }
			if (db_msg.driver_control.B.Flag == 1)
			{
				db_msg.driver_control.B.Flag = 0;
				car_mode=db_msg.driver_control.B.mode_smart; //0이면 스마트제어모드 on 1이면 off

				// 처리 로직 예시
			}
//		// 748
//		if (db_msg.light.B.Flag == 1)
//		{
//			db_msg.light.B.Flag == 0;
//			// 처리 로직 예시
//		}
			// 732
			if (db_msg.smart_window.B.Flag == 1)
			{
				motor1_smart_flag=1;
				db_msg.smart_window.B.Flag = 0;
				motor1_smart=db_msg.smart_window.B.motor1_smart_state;
				motor1_smart_pct=db_msg.smart_window.B.motor1_state;
	//			// 처리 로직 예시
			}
			// 731
			if (db_msg.smart_sunroof.B.Flag == 1)
			{
				db_msg.smart_sunroof.B.Flag = 0;
				motor2_smart_flag=1;
				db_msg.smart_sunroof.B.Flag = 0;
				motor2_smart=db_msg.smart_sunroof.B.motor2_smart_state;
				// 처리 로직 예시
			}
		// 734
		if (db_msg.smart_heater.B.Flag == 1)
		{
			db_msg.smart_heater.B.Flag = 0;
			if (actuator_power == 1)
				smart_heater(db_msg.smart_heater.B.Heater_state);
		}
		// 735
		if (db_msg.smart_ac.B.Flag == 1)
		{
			db_msg.smart_ac.B.Flag = 0;
			if (actuator_power == 1)
				smart_ac(db_msg.smart_ac.B.Air_state);
		}

//		// 733
//		if (db_msg.smart_audio.B.Flag == 1)
//		{
//			db_msg.smart_audio.B.Flag == 0;
//			// 처리 로직 예시
//		}
//		// 712
		if (db_msg.safety_window.B.Flag == 1)
		{
			safety_win_flag=1;
			db_msg.safety_window.B.Flag = 0;
			safety_win=db_msg.safety_window.B.motor1_smart_state;

		}
//		// 711
		if (db_msg.safety_sunroof.B.Flag == 1)
		{
			safety_sun_flag=1;
			db_msg.safety_sunroof.B.Flag = 0;
			// 처리 로직 예시
	        saftey_sun=db_msg.safety_sunroof.B.motor2_smart_state;
		}


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}
void AppTask1ms(void)
{
    stTestCnt.u32nuCnt1ms++;
    {
        // 스케쥴링 예시코드 1ms 주기로 공기질 메시지 전송
//        db_msg.heater.B.Heater_alive = 1;
//        db_msg.heater.B.Heater_running = 1;
//        output_message(&db_msg.heater,HEATER_MSG_ID);
    }
}

void AppTask10ms(void)
{
    stTestCnt.u32nuCnt10ms++;
    Window_ControlMode();
    Window_UpdateState();
    {


		//753
//		db_msg.audio.B.Audio_alive = 0;
//		db_msg.audio.B.Audio_running = 0;
//		output_message(&db_msg.audio,AUDIO_MSG_ID);

		//76F
//		db_msg.battery.B.Battery_alive = 0;
//		db_msg.battery.B.Battery_spare_state = 0;
//		db_msg.battery.B.Battery_state = 0;
//		db_msg.battery.B.Battery_use = 0;
//		output_message(&db_msg.battery,BATTERY_MSG_ID);



    }
}

void AppTask100ms(void)
{
    stTestCnt.u32nuCnt100ms++;
    {
    	if(actuator_power == 1){
			update_vehicle_vehicle();
			show_LCD();
    	}
    }
    {
		// 754
		db_msg.heater.B.Heater_alive = actuator_power;
		db_msg.heater.B.Heater_running = heater_led_state;
		output_message(&db_msg.heater,HEATER_MSG_ID);

		// 755
		db_msg.ac.B.AC_alive = actuator_power;
		db_msg.ac.B.AC_running = ac_led_state;
		output_message(&db_msg.ac,AIRCONDITIONER_MSG_ID);
    }

    {
		db_msg.motor1_window.B.motor1_alive = actuator_power;
		db_msg.motor1_window.B.motor1_running = running_win;
		db_msg.motor1_window.B.motor1_tick_counter = window_pulse_count;

		output_message(&db_msg.motor1_window,MOTOR1_WINDOW_MSG_ID);

		db_msg.motor2_sunroof.B.motor2_alive = actuator_power;
		db_msg.motor2_sunroof.B.motor2_running = running_sunroof;
		db_msg.motor2_sunroof.B.motor2_tick_counter = sunroof_pulse_count;

		output_message(&db_msg.motor2_sunroof,MOTOR2_SUNROOF_MSG_ID);
    }
}

void AppTask1000ms(void)
{

    stTestCnt.u32nuCnt1000ms++;
    {
    	battery_data_out();
    }
}

void AppScheduling(void)
{

    if (stSchedulingInfo.u8nuScheduling1msFlag == 1u)
    {
        stSchedulingInfo.u8nuScheduling1msFlag = 0u;

        AppTask1ms();

        if (stSchedulingInfo.u8nuScheduling10msFlag == 1u)
        {
            stSchedulingInfo.u8nuScheduling10msFlag = 0u;
            AppTask10ms();
        }

        if (stSchedulingInfo.u8nuScheduling100msFlag == 1u)
        {
            stSchedulingInfo.u8nuScheduling100msFlag = 0u;
            AppTask100ms();
        }
        if (stSchedulingInfo.u8nuScheduling1000msFlag == 1u)
        {
            stSchedulingInfo.u8nuScheduling1000msFlag = 0u;
            AppTask1000ms();
        }
    }
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
//  __disable_irq();
//  while (1)
//  {
//  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
