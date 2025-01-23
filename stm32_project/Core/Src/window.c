#include "window.h"
#include "can.h"

// 전역 변수 정의
volatile uint32_t window_timer_ms = 0;       // 창문 동작 시간(ms)
volatile uint32_t window_pulse_count = 0;    // 창문 위치 (0~100%)
volatile uint8_t target_position = 100;               // 목표 위치 (%)
volatile uint8_t running_win=0;

//전역변수 can
int safety_win=0;
int motor1_smart=0;
int motor1_smart_pct=0;
int override_win=0;
int car_mode=0;
int motor1_smart_flag=0;
int override_flag_win=0;
int safety_win_flag=0;

// 창문 상태 전역 변수
volatile uint8_t window_position = 0;   // 창문 열림 정도 (%)
volatile WindowState window_state = WINDOW_STOPPED; // 창문 상태


extern TIM_HandleTypeDef htim3;              // TIM3 핸들러 (PWM 제어용)
extern uint8_t actuator_power;

// 내부 함수 선언
static void Window_SetPWM(uint16_t speed);
static void Window_SetDirection(int direction);

// 창문 초기화
void Window_Init(void) {
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    window_timer_ms = 0;
    window_pulse_count = 0; // 창문 닫힘 상태로 초기화
    window_state = WINDOW_STOPPED;
    Window_SetPWM(0); // 초기 PWM 듀티를 0으로 설정

}

// 창문 제어 모드
void Window_ControlMode(void) {
    // 1. 손 끼임 발생 시 즉시 모터 정지
    if (safety_win_flag == 1 && safety_win == 1) {
        Window_Stop();       // 창문 정지
        OpenWindow(70);      // 창문 70% 열기
        safety_win_flag = 0; // 이벤트 처리 완료, 플래그 초기화
        return;
    }

    // 2. 운전자 개입 발생 시
    if (override_flag_win == 1) {
        if (override_win == 0) { // 창문 열기 버튼 누름
            OpenWindow(0); // 창문 완전히 열기
        } else if (override_win == 2) { // 창문 닫기 버튼 누름
            CloseWindow(100); // 창문 완전히 닫기
        } else if (override_win == 1 || override_win == 3) { // 버튼 떼면 정지
            Window_Stop();
        }
        override_flag_win = 0; // 이벤트 처리 완료, 플래그 초기화
        return;
    }

    // 3. 스마트 제어 발생 시
    if (motor1_smart_flag == 1) {
        if (motor1_smart == 2) { // 창문 열기 명령
            OpenWindow(motor1_smart_pct); // 목표 열림 비율만큼 열기
        } else if (motor1_smart == 1) { // 창문 닫기 명령
            CloseWindow(motor1_smart_pct); // 목표 닫힘 비율만큼 닫기
        }
        motor1_smart_flag = 0; // 이벤트 처리 완료, 플래그 초기화
        return;
    }
    // 4. 전원off
    if (actuator_power==0){
    	Window_Stop();
    }
}

// 창문 특정 위치까지 열기
void OpenWindow(int percent) {
    if (percent < 0) percent = 0; // 최소값
    if (percent > 100) percent = 100; // 최대값

    // 창문 동작 조건 확인 및 시작
    if (window_state == WINDOW_STOPPED) {
        running_win = 2;                 // 모터 상태: 열림
        window_state = WINDOW_OPENING;   // 창문 상태 변경
        target_position = percent;       // 목표 위치 설정
        window_timer_ms = (100 - window_pulse_count) * WINDOW_OPEN_TIME_MS / 100;             // 타이머 초기화
        Window_SetDirection(1);          // 정방향 설정
        Window_SetPWM(WINDOW_PWM_SPEED); // PWM 활성화
    }
}

// 창문 특정 위치까지 닫기
void CloseWindow(int percent) {
    if (percent < 0) percent = 0; // 최소값
    if (percent > 100) percent = 100; // 최대값

    // 창문 동작 조건 확인 및 시작
    if (window_state == WINDOW_STOPPED) {
        running_win = 1;                 // 모터 상태: 닫힘
        window_state = WINDOW_CLOSING;   // 창문 상태 변경
        target_position = percent;       // 목표 위치 설정
        window_timer_ms = window_pulse_count * WINDOW_CLOSE_TIME_MS / 100;
        Window_SetDirection(-1);         // 역방향 설정
        Window_SetPWM(WINDOW_PWM_SPEED); // PWM 활성화
    }
}

// 창문 상태 업데이트
void Window_UpdateState(void) {
    if (window_state == WINDOW_OPENING) {
        window_timer_ms += 10; // 10ms 단위로 시간 누적
        window_pulse_count = 100 - (window_timer_ms * 100) / WINDOW_OPEN_TIME_MS; // 닫힘 기준 100 → 열림 기준 0

        // 목표 위치 도달 시 동작 멈춤
        if (window_pulse_count <= target_position) { // 목표 위치와 같거나 작으면 정지
            Window_Stop();
            return;
        }
    } else if (window_state == WINDOW_CLOSING) {
        window_timer_ms += 10; // 10ms 단위로 시간 누적
        window_pulse_count = (window_timer_ms * 100) / WINDOW_CLOSE_TIME_MS; // 열림 기준 0 → 닫힘 기준 100

        // 목표 위치 도달 시 동작 멈춤
        if (window_pulse_count >= target_position) { // 목표 위치와 같거나 크면 정지
            Window_Stop();
            return;
        }
    }
}

// 창문 정지
void Window_Stop(void) {
    window_state = WINDOW_STOPPED;
    Window_SetPWM(0); // PWM 듀티를 0으로 설정하여 모터 정지
    running_win = 0;  // 모터 상태 초기화
    window_timer_ms = 0;
}

// PWM 듀티 사이클 설정
static void Window_SetPWM(uint16_t speed) {
    if (speed > 1000) speed = 1000; // 최대 듀티 사이클 제한
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, speed); // TIM3 채널 2 PWM 설정
}

// 모터 방향 설정
static void Window_SetDirection(int direction) {
    if (direction == 1) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // 정방향
    } else if (direction == -1) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // 역방향
    }
}
