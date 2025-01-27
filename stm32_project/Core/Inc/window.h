#ifndef WINDOW_H
#define WINDOW_H

#include "main.h"

// 창문 상태 정의
typedef enum {
    WINDOW_STOPPED,   // 정지 상태
    WINDOW_OPENING,   // 열리는 중
    WINDOW_CLOSING    // 닫히는 중
} WindowState;

// 전역 변수 선언
extern volatile uint32_t window_timer_ms;    // 창문 동작 시간(ms)
extern volatile uint32_t window_pulse_count; // 창문 위치 (0~100%)
extern volatile WindowState window_state;            // 창문 상태
extern volatile uint8_t running_win;
// 창문 관련 변수
extern int safety_win;
extern int motor1_smart;
extern int motor1_smart_pct;
extern int override_win;
extern int car_mode;
extern int motor1_smart_flag;
extern int override_flag_win;
extern int safety_win_flag;


// 상수 정의
#define WINDOW_OPEN_TIME_MS 1400    // 창문이 완전히 열리는데 걸리는 시간 (ms)
#define WINDOW_CLOSE_TIME_MS 6000   //4000 창문이 완전히 닫히는데 걸리는 시간 (ms)
#define WINDOW_PWM_SPEED 400     // 모터 PWM 기본 속도

// 함수 선언
void Window_Init(void);                    // 창문 초기화
void OpenWindow(int percent);  // 창문 열기
void CloseWindow(int percent); // 창문 닫기
void Window_UpdateState(void);             // 창문 상태 업데이트
void Window_Stop(void);                    // 창문 정지
void Window_ControlMode(void);
#endif // WINDOW_H
