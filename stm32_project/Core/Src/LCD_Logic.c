/*
 * LCD_Logic.c
 *
 *  Created on: Jan 28, 2025
 *      Author: Wonphil
 */

#include <stdio.h>

#include "LCD_Logic.h"
#include "Common_def.h"
#include "DFPlayer.h"

VehicleState vehicle;
float *use_battery = NULL;
float *other_battery = NULL;
int is_solar_battery_charging = 0;
// 차량 배터리 소모 계산
void consume_car_battery()
{
	if (vehicle.is_driving) // 메시지 생기면 대체
	{
		// 메시지 생기면 대체
		int consumption = MOTOR_CONSUM * vehicle.motor_speed; // 차량 이동 속도 (모터) 값에 비례
		if(0)//is_solar_battery_charging)// 충전중이면, 원래 로직으로 사용
		{
			if (vehicle.car_battery >= consumption)
			{
				vehicle.car_battery -= consumption;
			}
			else
			{
				vehicle.car_battery = 0;
			}
		}
		else
		{
			if (*use_battery >= consumption)
			{
				*use_battery -= consumption;
			}
			else
			{
				if(*other_battery <= consumption)
				{
					*use_battery = 0;
					*other_battery = 0;
				}
				else
				{
					*other_battery -= (consumption - *use_battery);
					*use_battery = 0;
				}
			}
		}
	}
}

// 태양광 배터리 충전 계산
void charge_solar_battery()
{
	//하드웨어랑 합의보고, 선루프가 열리는 정도에 대한 비례식을 세울지 말지 고민 - Tick_count = 0~100
	// 선루프가 닫혔을 때
	if (db_msg.motor2_sunroof.B.motor2_alive == ALIVE && db_msg.motor2_sunroof.B.motor2_tick_counter!=OPEN)
	{														//선루프가 OPEN = 0, 풀로 닫힘 == 100
		//light.B.Light_pct = 일단 60정도면 어두운데, 임의로 값 조정
		float charge_amount = (db_msg.light.B.Light_pct * 0.0001 * // 조도, 선루프 닫힘에 비례한 충전
				db_msg.motor2_sunroof.B.motor2_tick_counter * ECOBAT_CHARGING);

		is_solar_battery_charging = charge_amount >0 ? 1 : 0;
		if (vehicle.solar_battery + charge_amount < MAX_CHARGE_ECO)
		{
			vehicle.solar_battery += charge_amount;
		}
		else
		{
			vehicle.solar_battery = MAX_CHARGE_ECO - 0.01;
		}
	}
}

// 태양광 배터리 소모 계산
void consume_solar_battery()
{
	float consumption = 0.0;
	if (db_msg.ac.B.AC_alive==ALIVE && db_msg.ac.B.AC_running>=WEAK) // 에어컨 사용
	{
		switch (db_msg.ac.B.AC_running){
		case (WEAK):
		{
			consumption += AC_WEAK_CONSUM;
			break;
		}
		case (STRONG):
			consumption += AC_WEAK_CONSUM *1.5;
			break;
		}
	}
	if (db_msg.heater.B.Heater_alive==ALIVE && db_msg.heater.B.Heater_running>=WEAK) // 에어컨 사용
	{
		switch (db_msg.heater.B.Heater_running){
		case (WEAK): // 약풍 제어 명령일 때
		{
			consumption += HEATER_WEAK_CONSUM; // 히터 사용
			break;
		}
		case (STRONG): // 강풍 제어 명령일 때
			consumption += HEATER_WEAK_CONSUM * 1.5;
			break;
		}
	}

	if (db_msg.motor1_window.B.motor1_alive == ALIVE && db_msg.motor1_window.B.motor1_running >= 1)
		consumption += WINDOW_CONSUM; // 창문 작동

	if (db_msg.motor2_sunroof.B.motor2_alive == ALIVE && db_msg.motor2_sunroof.B.motor2_running >=1)
		consumption += SUNROOF_CONSUM; // 선루프 작동

	if (db_msg.audio.B.Audio_alive == ALIVE && audio_flag == 1){
		consumption += AUDIO_CONSUM;
	}

	if (0)//is_solar_battery_charging) // 충전중이면, 원래 로직으로 사용
	{
		if (vehicle.solar_battery >= consumption)
			vehicle.solar_battery -= consumption;
		else
		{
			// 모자란 부분 car_battery에서 감소
			vehicle.car_battery -= (consumption - vehicle.solar_battery);
			// 태양광 에너지 = 0
			vehicle.solar_battery = 0;
		}
	}
	else
	{
		if (*use_battery >= consumption)
			*use_battery -= consumption;
		else
		{
			if(*other_battery <= consumption)
			{
				*use_battery = 0;
				*other_battery = 0;
			}
			else
			{
				*other_battery -= (consumption - *use_battery);
				*use_battery = 0;
			}
		}
	}
}
void switching_battery()
{
	if (vehicle.car_battery < MAX_CHARGE_CAR*0.3) // 차량 배터리가 30% 미만이면,
	{
		use_battery = &vehicle.solar_battery; // 메인 배터리를 태양광 배터리로 사용
		other_battery = &vehicle.car_battery;
	}
	else
	{
		// More nuanced battery selection
		if (vehicle.solar_battery >= MAX_CHARGE_ECO*0.5) // 태양광 배터리가 50% 이상이면,
		{
			use_battery = &vehicle.solar_battery; // 메인 배터리를 태양광 배터리로 사용
			other_battery = &vehicle.car_battery;
		}
		else
		{
			use_battery = &vehicle.car_battery; // 차량 배터리도 30퍼 이상이고, 태양광배터리도 50퍼 미만이면
			other_battery = &vehicle.solar_battery; // 메인 배터리를 차량 배터리로 사용
		}
	}
}
void battery_out_check()
{
	if(db_msg.battery.B.Battery_spare_state == 0 &&
			db_msg.battery.B.Battery_state == 0)
	{
		// 배터리가 없어 종료됩니다!
		Sound_Track(53);
	}
}
// 상태 업데이트 함수
void update_vehicle_state()
{
	vehicle.is_driving = (db_msg.driver_engine.B.engine_mode == DRIVING);
	switching_battery();
	consume_car_battery();
	charge_solar_battery();
	consume_solar_battery();
	battery_out_check();

}

void init_vehicle_state()
{

	vehicle.car_battery = MAX_CHARGE_CAR * INIT_CAR_BATTERY * 0.01 ;	// 77.4k = 100%
	vehicle.solar_battery = MAX_CHARGE_ECO * INIT_SOLAR_BATTERY * 0.01 ; // 4.8k = 100% 초기값 70%

	db_msg.battery.B.Battery_state = INIT_CAR_BATTERY;
	db_msg.battery.B.Battery_spare_state = INIT_SOLAR_BATTERY;

	//임의 값들
//	vehicle.light_intensity = 80;
	vehicle.is_driving = (db_msg.driver_engine.B.engine_mode == DRIVING);
	vehicle.motor_speed = 3; //6->3
}


