/*
 * LCD_Logic.c
 *
 *  Created on: Jan 22, 2025
 *      Author: USER
 */

/*
 * 100ms scheduling 내부에 넣기
 * 숫자는 일단 ㄱㄱ
 * */

#include <stdio.h>

#include "LCD_Logic.h"
#include "LCD_def.h"


VehicleState vehicle;

// 차량 배터리 소모 계산
void consume_car_battery()
{
	if (vehicle.is_driving) // 메시지 생기면 대체
	{
		// 메시지 생기면 대체
		int consumption = MOTOR_CONSUM * vehicle.motor_speed; // 차량 이동 속도 (모터) 값에 비례
		if (vehicle.car_battery >= consumption)
		{
			vehicle.car_battery -= consumption;
		}
		else
		{
			vehicle.car_battery = 0;
		}
	}
}

// 태양광 배터리 충전 계산
void charge_solar_battery()
{
	//하드웨어랑 합의보고, 선루프가 열리는 정도에 대한 비례식을 세울지 말지 고민 - Tick_count = 0~100
	// 선루프가 닫혔을 때
	if (db_msg.motor2_sunroof.B.motor2_alive == 1 && db_msg.motor2_sunroof.B.motor2_tick_counter>0)
	{																	// 선루프가 닫혀 있을 때만 충전
		//light.B.Light_pct = 일단 60정도면 어두운데, 임의로 값 조정
//		float charge_amount = (ECOBAT_CHARGING * db_msg.light.B.Light_pct * 0.01); // 조도에 비례한 충전
		float charge_amount = (ECOBAT_CHARGING * vehicle.light_intensity * 0.01); // 조도에 비례한 충전

		if (vehicle.solar_battery + charge_amount < MAX_CHARGE_ECO)
		{
			vehicle.solar_battery += charge_amount;
		}
		else
		{
			vehicle.solar_battery = MAX_CHARGE_ECO - 1;
		}
	}
}

// 태양광 배터리 소모 계산
void consume_solar_battery()
{
	float consumption = 0.0;
	if (db_msg.ac.B.AC_alive==1 && db_msg.ac.B.AC_running>=1) // 에어컨 사용
	{
		switch (db_msg.ac.B.AC_running){
		case (WEAK): // enum 데이터로 변경
		{
			consumption += AC_WEAK_CONSUM;
			break;
		}
		case (STRONG):
			consumption += AC_WEAK_CONSUM *1.5;
			break;
		}
	}
	if (db_msg.heater.B.Heater_alive==1 && db_msg.heater.B.Heater_running>=1) // 에어컨 사용
	{
		switch (db_msg.heater.B.Heater_running){
		case (WEAK): // enum 데이터로 변경
		{
			consumption += HEATER_WEAK_CONSUM; // 히터 사용
			break;
		}
		case (STRONG):
			consumption += HEATER_WEAK_CONSUM * 1.5;
			break;
		}
	}

	if (db_msg.motor1_window.B.motor1_alive == 1 && db_msg.motor1_window.B.motor1_running >= 1)
		consumption += WINDOW_CONSUM; // 창문 작동

	if (db_msg.motor2_sunroof.B.motor2_alive == 1 && db_msg.motor2_sunroof.B.motor2_running >=1)
		consumption += SUNROOF_CONSUM; // 선루프 작동

	if (db_msg.audio.B.Audio_alive == 1 && db_msg.audio.B.Audio_running ==1){
		consumption += AUDIO_CONSUM;
	}

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

// 상태 업데이트 함수
void update_vehicle_vehicle()
{

	consume_car_battery();
	charge_solar_battery();
	consume_solar_battery();

}

void init_vehicle_state()
{

	vehicle.car_battery = MAX_CHARGE_CAR / 2;	// 77.4k = 100% 초기값 38700
	vehicle.solar_battery = MAX_CHARGE_ECO / 2; // 4.8k = 100% 초기값 2400

	//임의 값들
	vehicle.light_intensity = 80;
	vehicle.is_driving = 1;
	vehicle.motor_speed = 6;
}

void battery_data_out(){
	db_msg.battery.B.Battery_alive = actuator_power;
	db_msg.battery.B.Battery_use = 3; // 2개 다씀

	output_message(&db_msg.battery, BATTERY_MSG_ID);
}
