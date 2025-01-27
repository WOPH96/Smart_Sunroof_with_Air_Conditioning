#ifndef SUNROOF_H
#define SUNROOF_H

#include "main.h"

// 선루프 상태 정의
typedef enum {
    SUNROOF_STOPPED,     // 선루프 정지
    SUNROOF_OPENING,     // 선루프 열기 중
    SUNROOF_CLOSING      // 선루프 닫기 중
} SunroofState;

// 상수 정의
#define SUNROOF_PWM_SPEED 300          // PWM 기본 속도
#define SUNROOF_OPEN_TIME_MS 4000      // 선루프 완전히 열리는 데 걸리는 시간 (ms)
#define SUNROOF_CLOSE_TIME_MS 4000     // 선루프 완전히 닫히는 데 걸리는 시간 (ms)

// 전역 변수 선언
extern volatile uint32_t sunroof_timer_ms;       // 선루프 동작 시간(ms)
extern volatile uint32_t sunroof_pulse_count;    // 선루프 위치 (0~100%)
extern volatile uint8_t target_position_sun;         // 목표 위치 (%)
extern volatile uint8_t running_sunroof;         // 모터 상태 (0: 정지, 1: 열림, 2: 닫힘)
extern volatile SunroofState sunroof_state;      // 선루프 상태
// 전역 변수 참조 (extern)
extern int saftey_sun;
extern int override_flag_sun;
extern int safety_sun_flag;
extern int motor2_smart_flag;
extern int override_sun;
extern int motor2_smart;

// 함수 선언
void Sunroof_Init(void);                     // 선루프 초기화
void Sunroof_ControlMode(void);              // 선루프 제어 모드
void Sunroof_Open(int percent);              // 선루프 열기
void Sunroof_Close(int percent);             // 선루프 닫기
void Sunroof_UpdateState(void);              // 선루프 상태 업데이트
void Sunroof_Stop(void);                     // 선루프 정지

#endif // SUNROOF_H
