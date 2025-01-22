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

VehicleState vehicle;

// 차량 배터리 소모 계산
void consume_car_battery()
{
	if (vehicle.is_driving)
	{
		int consumption = 10 * vehicle.motor_speed; // 모터 속도에 비례한 배터리 소모
		if (vehicle.car_battery > consumption)
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
	if (!vehicle.sunroof_open)
	{													 // 선루프가 닫혀 있을 때만 충전
		int charge_amount = (int) (40* vehicle.light_intensity * 0.01); // 조도에 비례한 충전
		if (vehicle.solar_battery + charge_amount < MAX_CHARGE_ECO)
		{
			vehicle.solar_battery += charge_amount;
		}
		else
		{
			vehicle.solar_battery = MAX_CHARGE_ECO-1;
		}
	}
}

// 태양광 배터리 소모 계산
void consume_solar_battery()
{
	int consumption = 0;
	if (vehicle.ac_on)
		consumption += 8; // 에어컨 사용
	if (vehicle.heater_on)
		consumption += 5; // 히터 사용
	if (vehicle.window_open)
		consumption += 2; // 창문 작동
	if (vehicle.sunroof_open)
		consumption += 3; // 선루프 작동

	if (vehicle.solar_battery > consumption)
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
	vehicle.sunroof_open = 0;
	vehicle.window_open = 0;
	vehicle.light_intensity = 80;
	vehicle.is_driving = 1;
	vehicle.ac_on = 1;
	vehicle.heater_on = 0;
	vehicle.motor_speed = 6;
}

// 메인 함수
// int main() {
//    // 초기 상태 설정
//    Vehiclevehicle vehicle = {
//        .car_battery = MAX_CHARGE_CAR, // 77.4k = 100%
//        .solar_battery = MAX_CHARGE_ECO, // 4.8k = 100%
//        .sunroof_open = 0,
//        .window_open = 0,
//        .light_intensity = 80,
//        .is_driving = 1,
//        .ac_on = 1,
//        .heater_on = 0,
//        .motor_speed = 6
//    };
//
//
//    // 상태 업데이트, LCD출력 100ms 주기에 넣기
//    update_vehicle_vehicle();
//    show_LCD();
//
//    // 결과 출력
//    return 0;
//}
