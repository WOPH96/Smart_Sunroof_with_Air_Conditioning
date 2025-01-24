/*
 * LCD_Logic.h
 *
 *  Created on: Jan 22, 2025
 *      Author: USER
 */

#ifndef INC_LCD_LOGIC_H_
#define INC_LCD_LOGIC_H_

#include "can.h"

#define MAX_CHARGE_CAR 77400
#define MAX_CHARGE_ECO 4800

// 구조체 정의
typedef struct
{
    float car_battery;   // 차량 배터리 잔량 (%)
    float solar_battery; // 태양광 배터리 잔량 (%)

    /*아래 정보들은, db_msg.으로 접근 */
//    int sunroof_open;    // 선루프 상태 (0: 닫힘, 1: 열림)
//    int window_open;     // 창문 상태 (0: 닫힘, 1: 열림)
    int light_intensity; // 조도 센서 값 (0~100)
    int is_driving;      // 차량 주행 여부 (0: 정지, 1: 주행)
//    int ac_on;           // 에어컨 사용 여부 (0: 꺼짐, 1: 켜짐)
//    int heater_on;       // 히터 사용 여부 (0: 꺼짐, 1: 켜짐)
    int motor_speed;     // 차량 모터 속도 (0.0 ~ 1.0)
} VehicleState;

extern VehicleState vehicle;

void consume_car_battery();
void charge_solar_battery();
void consume_solar_battery();
void switching_battery();
void battery_out_check();
void update_vehicle_vehicle();
void init_vehicle_state();

#endif /* INC_LCD_LOGIC_H_ */
