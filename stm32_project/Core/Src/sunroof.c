#include "sunroof.h"
#include "can.h"

// 전역 변수 정의
volatile uint32_t sunroof_timer_ms = 0;       // 선루프 동작 시간(ms)
volatile uint32_t sunroof_pulse_count = 0;    // 선루프 위치 (0~100%)
volatile uint8_t target_position_sun = 100;   // 목표 위치 (%)
volatile uint8_t running_sunroof = 0;         // 모터 동작 상태 (0: 정지, 1: 닫, 2: 열)

//전역변수 can
int saftey_sun=0;
int override_flag_sun=0;
int safety_sun_flag=0;
int motor2_smart_flag=0;
int override_sun=0;
int motor2_smart=0;

// 선루프 상태 전역 변수
volatile uint8_t sunroof_position = 0;        // 선루프 열림 정도 (%)
volatile SunroofState sunroof_state = SUNROOF_STOPPED; // 선루프 상태

extern TIM_HandleTypeDef htim2; // 선루프 PWM 제어용 TIM 핸들러
extern uint8_t actuator_power;

// 내부 함수 선언
static void Sunroof_SetPWM(uint16_t speed);
static void Sunroof_SetDirection(int direction);

// 선루프 초기화
void Sunroof_Init(void) {
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    sunroof_timer_ms = 0;
    sunroof_pulse_count = 0; // 선루프 닫힘 상태로 초기화
    sunroof_state = SUNROOF_STOPPED;
    Sunroof_SetPWM(0); // 초기 PWM 듀티를 0으로 설정

}

// 선루프 제어 모드
void Sunroof_ControlMode(void) {
    // 1. 손 끼임 발생 시 즉시 모터 정지
    if (safety_sun_flag == 1 && saftey_sun == 1) {
        Sunroof_Stop();         // 선루프 정지
        Sunroof_Open(0);       // 선루프 70% 열기
        safety_sun_flag = 0;    // 이벤트 처리 완료, 플래그 초기화
        saftey_sun=0;
        return;
    }

    // 2. 운전자 개입 발생 시
    if (override_flag_sun == 1 && saftey_sun == 0) {
        if (override_sun == 0) { // 선루프 열기 버튼 누름
            Sunroof_Open(0);  // 선루프 완전히 열기
        } else if (override_sun == 2) { // 선루프 닫기 버튼 누름
            Sunroof_Close(100);   // 선루프 완전히 닫기
        } else if (override_sun == 1 || override_sun == 3) { // 버튼 떼면 정지
            Sunroof_Stop();
        }
        override_flag_sun = 0; // 이벤트 처리 완료, 플래그 초기화
        return;
    }

    // 3. 스마트 제어 발생 시
    if (motor2_smart_flag == 1 && saftey_sun == 0) {
        if (motor2_smart == 2) { // 선루프 열기 명령
            Sunroof_Open(0); // 목표 열림 비율만큼 열기
        } else if (motor2_smart == 1) { // 선루프 닫기 명령
            Sunroof_Close(100); // 목표 닫힘 비율만큼 닫기
        }
        motor2_smart_flag = 0; // 이벤트 처리 완료, 플래그 초기화
        motor2_smart=0;
        return;
    }
    // 4. 전원off
    if (actuator_power==0){
    	Sunroof_Stop();
    }
}

// 선루프 특정 위치까지 열기
void Sunroof_Open(int percent) {
    if (percent < 0) percent = 0; // 최소값
    if (percent > 100) percent = 100; // 최대값

    // 선루프 동작 조건 확인 및 시작
    if (sunroof_state == SUNROOF_STOPPED) {
        running_sunroof = 2;                // 모터 상태: 열림
        sunroof_state = SUNROOF_OPENING;    // 선루프 상태 변경
        target_position_sun = percent;     // 목표 위치 설정
        sunroof_timer_ms =(100-sunroof_pulse_count) * SUNROOF_OPEN_TIME_MS / 100;           // 타이머 초기화
        Sunroof_SetDirection(1);           // 정방향 설정
        Sunroof_SetPWM(SUNROOF_PWM_SPEED); // PWM 활성화
    }
}

// 선루프 특정 위치까지 닫기
void Sunroof_Close(int percent) {
    if (percent < 0) percent = 0; // 최소값
    if (percent > 100) percent = 100; // 최대값

    // 선루프 동작 조건 확인 및 시작
    if (sunroof_state == SUNROOF_STOPPED) {
        running_sunroof = 1;                // 모터 상태: 닫힘
        sunroof_state = SUNROOF_CLOSING;    // 선루프 상태 변경
        target_position_sun = percent;     // 목표 위치 설정
        sunroof_timer_ms = sunroof_pulse_count * SUNROOF_CLOSE_TIME_MS / 100;              // 타이머 초기화
        Sunroof_SetDirection(-1);          // 역방향 설정
        Sunroof_SetPWM(SUNROOF_PWM_SPEED); // PWM 활성화
    }
}

// 선루프 상태 업데이트
void Sunroof_UpdateState(void) {
    if (sunroof_state == SUNROOF_OPENING) {
        sunroof_timer_ms += 10; // 10ms 단위로 시간 누적
        sunroof_pulse_count = 100-(sunroof_timer_ms * 100) / SUNROOF_CLOSE_TIME_MS;

        // 목표 위치 도달 시 동작 멈춤
        if (sunroof_pulse_count <= target_position_sun) {
            Sunroof_Stop();
            return;
        }
    } else if (sunroof_state == SUNROOF_CLOSING) {
        sunroof_timer_ms += 10; // 10ms 단위로 시간 누적
        sunroof_pulse_count = (sunroof_timer_ms * 100) / SUNROOF_OPEN_TIME_MS;
        // 목표 위치 도달 시 동작 멈춤
        if (sunroof_pulse_count >= target_position_sun) {
            Sunroof_Stop();
            return;
        }
    }
}

// 선루프 정지
void Sunroof_Stop(void) {
    sunroof_state = SUNROOF_STOPPED;
    Sunroof_SetPWM(0); // PWM 듀티를 0으로 설정하여 모터 정지
    running_sunroof = 0; // 모터 상태 초기화
    sunroof_timer_ms=0;
}

// PWM 듀티 사이클 설정
static void Sunroof_SetPWM(uint16_t speed) {
    if (speed > 1000) speed = 1000; // 최대 듀티 사이클 제한
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, speed); // TIM2 채널 2 PWM 설정
}

// 모터 방향 설정
static void Sunroof_SetDirection(int direction) {
    if (direction == 1) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // 정방향
    } else if (direction == -1) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // 역방향
    }
}
