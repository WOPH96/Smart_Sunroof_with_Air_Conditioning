/**********************************************************************************************************************
 * @file    Cpu0_Main.c
 * @brief   Logic Inner TC275
 * @version 1.0
 * @date    2025-01-24
 *********************************************************************************************************************/

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

//#include "IfxAsclin_Asc.h"
//#include "IfxCpu_Irq.h"
//#include "Bsp.h"

#include "OurCan.h"

#include "Driver_Stm.h"
#include "Driver_Adc.h"
#include "Driver_Easy.h"
#include "Driver_Touch.h"
#include "Driver_Air.h"
#include "Driver_DHT11.h"

#include "sound_sensor.h"
#include "rain_sensor.h"

#include "ASCLIN_Shell_UART.h"

#include "queue.h"
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
#define AIR_PIN 4
#define SUN_TOUCH_PIN 0
#define WIN_TOUCH_PIN 1

#define CONTROL_MODULE 5

//test 모드
//#define ONE_MINUTE 9 // 10s 60
//#define FIVE_MINUTE 12 // 50s 300
//#define TEN_MINUTE 15 // 100s 600
//#define TWO_HOUR 20 // 300s 7200

#define ONE_MINUTE 18 // 18초
#define FIVE_MINUTE 90 // (1분 30초)
#define TEN_MINUTE 180 // (3분)
#define TWO_HOUR 1440 // (24분)

// 100ms 기준
#define TIME_1S 10
#define TIME_2S 20 // 20
#define TIME_1MIN 300 // (30초)
#define TIME_5MIN 1500 // (2분 30초)

#define ON 0
#define OFF 1

#define CLOSE 1
#define OPEN 2

#define FULL_OPEN 0
#define SEVENTY 70
#define NINTY 90
#define FULL_CLOSE 100

#define TURN_OFF 1
#define TURN_ON 2

#define WINTER
#define SUMMER
//#define TESTMODE

/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Data Structures---------------------------------------------------*/

typedef struct
{
    uint32 u32nuCnt1ms;
    uint32 u32nuCnt10ms;
    uint32 u32nuCnt100ms;
    uint32 u32nuCnt1000ms;
    uint32 u32nuCnt5000ms;
    uint32 u32nuCnt5000ms_2;
} Taskcnt;

typedef enum {
    ALL = 0,
    WINDOW,
    SUNROOF,
    HEATER,
    AIR,
    AUDIO
}ModuleType;

typedef enum {
    DRIVER_CONTROL = 0,
    SMART_CONTROL,
    ENGINE_CONTROL
}ControlType;

typedef enum {
    NO_RAIN = 0,
    RAIN
}RainState;

typedef enum {
    GOOD = 0,
    MODERATE,
    UNHEALTHY,
    VERY_UNHEALTHY
}DustType;

typedef enum {
    SAFETY = 1,
    DIRECT_CONTROL,
    WEATHER,
    TUNNEL,
    SOLAR,
    IN_CO2,
    DUST,
    IN_TEMP,
    NOISE,
    NONE
}PriorityType;

/*
 * queue.h 에 정의됨
typedef enum{
    CLOSE_WINDOW = 1,           // 창문을 닫습니다.
    OPEN_WINDOW = 2,                // 창문이 열립니다.
    CLOSE_SUNROOF = 3,              // 선루프를 닫습니다.
    OPEN_SUNROOF = 12,               // 선루프가 열립니다.
    ACTIVATE_AIRCON = 13,            // 에어컨을 작동합니다.
    DEACTIVATE_AIRCON = 14,          // 에어컨을 종료합니다.
    ACTIVATE_HEATER = 15,            // 히터를 작동합니다.
    DEACTIVATE_HEATER = 16,          // 히터를 종료합니다.
    CLOSE_WINDOW_AND_SUNROOF = 17,   // 창문과 선루프가 닫힙니다.
    OPEN_WINDOW_AND_SUNROOF = 52,    // 창문과 선루프가 열립니다.

    HIGH_FINE_DUST = 6,        // 미세먼지가 많습니다.
    IS_RAIN_DETECTED = 7,           // 비가 옵니다.
    INDOOR_HOT = 8,                 // 실내가 덥습니다.
    INDOOR_COLD = 9,                // 실내가 춥습니다.
    INDOOR_AIR_BAD = 10,             // 실내 공기가 안 좋습니다.
    ENTER_TUNNEL = 11,               // 터널에 진입합니다.
    EXIT_TUNNEL = 43,                // 터널을 통과했습니다.
    ENTER_TUNNEL_AND_CLOSE_WINDOW_AND_SUNROOF = 19, // 터널을 진입합니다. 창문과 선루프를 닫습니다.
    INDOOR_AIR_BAD_AND_OPEN_WINDOW_AND_SUNROOF = 20, // 실내 공기 안좋습니다. 창문과 선루프를 닫습니다.
    EXIT_TUNNEL_AND_OPEN_WINDOW_AND_SUNROOF = 21, // 터널 통과했습니다. 창문과 선루프가 열립니다.
    HIGH_FINE_DUST_AND_CLOSE_WINDOW_AND_SUNROOF = 22, // 미세먼지가 많습니다. 창문과 선루프를 닫습니다.
    IS_RAIN_DETECTED_AND_CLOSE_SUNROOF = 23, // 비가 옵니다. 선루프를 닫는다.
    INDOOR_HOT_AND_ACTIVATE_AIRCON = 24, // 실내가 덥습니다. 에어컨을 작동합니다.
    INDOOR_COLD_AND_ACTIVATE_HEATER = 25, // 실내가 춥습니다. 히터를 작동합니다.
    SAFETY_OPEN_WINDOW = 26,         // 끼임 방지를 위해 창문을 엽니다.
    SAFETY_OPEN_SUNROOF = 27,        // 끼임 방지를 위해 선루프를 엽니다.
    ENTER_TUNNEL_AND_CLOSE_WINDOW = 28, // 터널을 진입합니다. 창문을 닫습니다.
    ENTER_TUNNEL_AND_CLOSE_SUNROOF = 29, // 터널을 진입합니다. 선루프를 닫습니다.
    EXIT_TUNNEL_AND_OPEN_WINDOW = 30, // 터널을 통과했습니다. 창문이 열립니다.
    EXIT_TUNNEL_AND_OPEN_SUNROOF = 31, // 터널을 통과했습니다. 선루프가 열립니다.
    AIRCON_AND_CLOSE_WINDOW_AND_SUNROOF = 32, // 냉방 효율을 위해 창문과 선루프를 닫습니다.
    AIRCON_AND_CLOSE_WINDOW = 33, // 냉방 효율을 위해 창문을 닫습니다
    AIRCON_AND_CLOSE_SUNROOF = 34, // 냉방 효율을 위해 선루프를 닫습니다.
    HEATER_AND_CLOSE_WINDOW_AND_SUNROOF = 35, // 난방 효율을 위해 창문과 선루프를 닫습니다.
    HEATER_AND_CLOSE_WINDOW = 36, // 난방 효율을 위해 창문을 닫습니다
    HEATER_AND_CLOSE_SUNROOF = 37, // 난방 효율을 위해 선루프를 닫습니다
    SOLAR_AND_CLOSE_SUNROOF = 38, // 태양광 충전을 위해 선루프를 닫습니다.
    INDOOR_AIR_BAD_AND_OPEN_WINDOW = 39, // 실내 공기가 안좋습니다. 창문을 엽니다.
    INDOOR_AIR_BAD_AND_OPEN_SUNROOF = 40, // 실내 공기가 안좋습니다. 선루프를 엽니다
    FINISH_VENT_CLOSE_WINDOW_AND_SUNROOF = 41, // 환기가 끝났습니다. 창문과 선루프를 닫습니다
    FINISH_VENT_CLOSE_WINDOW = 42, // 환기가 끝났습니다. 창문을 닫습니다
    FINISH_VENT_CLOSE_SUNROOF = 44, // 환기가 끝났습니다. 선루프를 닫습니다
    SMART_CONTROL_PAUSE = 45,        // 스마트 제어가 종료되고 10분 후에 다시 작동합니다.
    TURN_OFF_SMART_CONTROL = 46,     // 스마트 제어를 종료합니다.
    TURN_ON_SMART_CONTROL = 47,      // 스마트 제어를 시작합니다.
    NOISE_AND_CLOSE_WINDOW_AND_SUNROOF = 48, // 소음을 감지하여 창문과 선루프를 닫습니다.
    NOISE_AND_CLOSE_WINDOW = 49, // 소음을 감지하여 창문을 닫습니다
    NOISE_AND_CLOSE_SUNROOF = 50, // 소음을 감지하여 선루프를 닫습니다.
    IS_RAIN_DETECTED_AND_OPEN_WINDOW = 51, // 비가 옵니다. 창문을 조금만 열겠습니다.

    DEFAULT = 63                // 초기값
}AudioControlState;
*/

typedef struct
{
    uint16 action_counter_100ms;
    uint16 trigger_counter_1s;
    uint8 next_command;
    uint8 next_sub_command;
    PriorityType next_priority;
    boolean trigger_reserved;
}TimerInfo;

typedef struct
{
    boolean state;          // On, Off 상태
    uint8 flag;             // message out 상태 표시
                            //0 : 초기값
                            //1 : Open, Turn On 동작 대기, 2 : Open, Turn On 동작 중(메세지 보냄)
                            //3 : Close, Turn Off 동작 대기, 4 : Close, Turn Off 동작 중(메세지 보냄)

    uint8 control_command;  // Close, Open, Turn_Off, Turn_On
    uint8 sub_command;      // 명령 주체, 창문 여는 정도,
    PriorityType priority;  // 명령 우선 순위

    TimerInfo timer;
    //uint16 counter_1s;      // 닫힌 상태로 부터 시간, 환기시 시간 측정 용도, 스마트 제어 재진입 시간, 1초에 한 번 동작하도록
    //uint16 counter_action_100ms; // 동작 유지 카운터. 이 값이 0이 되면 명령 우선순위, 명령 초기화, 명령에 따라 동작 유지 카운터를 달리 설정
}ControlCommandInfo;

typedef struct
{
    uint8 motor1_state; // 정지, Close, Open 상태
    uint8 motor2_state; // 정지, Close, Open 상태
    boolean heater_state; // Turn_off, Turn_on
    boolean air_state;    // Turn_off, Turn_on 상태
    uint8 audio_state;  // 정지, 오디오 출력 중, 출력 안하고 있음
}SmartControlState;

/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
Taskcnt stTestCnt;
ControlCommandInfo command_info[CONTROL_MODULE + 1]; // ALL 추가
SmartControlState control_state;

IfxCpu_syncEvent g_cpuSyncEvent = 0;

Gas in_gas = {0, 0, 0, 0, 0};
IfxPort_State suntouch = 0;
IfxPort_State wintouch = 0;
Temp_Hum in_temp_hum = { 0, 20, 20, 20, 20 };

RainState weather_state = NO_RAIN;
DustType dust_state = GOOD;

boolean needs_ventilation = FALSE;

uint32 light_adc = 0;

uint8 light_pct = 0;
uint8 window_state = 0;
uint8 sunroof_state = 0;

uint8 window_and_sunroof_state[2] = {0, 0}; // 0 초기화, 1 열어야 하는 상태, 2 열린 상태, 3 닫아야 하는 상태, 4 닫은 상태

boolean tunnel_flag = FALSE;
uint8 tunnel_window_state = 0; //터널 진입 시 현재 창문 위치 기록
uint8 tunnel_sunroof_state = 0; //터널 진입 시 현재 선루프 위치 기록

boolean audio_flag = FALSE;
Queue audio_queue;
AudioControlState audio_file_num = DEFAULT;

boolean engine_flag = FALSE;

boolean audio_flag_5s = TRUE;

boolean safety_win = FALSE;
boolean safety_sun = FALSE;
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/

// 사용자 조작에 의한 off인지, 시동 off에 의한 off인지, 스마트 제어모드 off에 의한 off인지 구별
// 사용자 조작에 의한 off이면 counter 값 10분으로 설정
void smart_control_mode_off(ControlCommandInfo* command_info, ControlType control);

// 스마트 제어 모드 ON
void smart_control_mode_on(ControlCommandInfo* command_info, ControlType control);

// 우선순위가 높은 명령이 선점중일 때, 동작하지 않음
void command_function(ControlCommandInfo* command_info, uint8 command, uint8 sub_command, PriorityType priority);

// 100ms 주기로 CAN 메세지 생성
void make_can_message(void);

void AppScheduling(void);
void AppTask1ms(void);
void AppTask10ms(void);

// 날씨, 미세먼지 상태에 따른 기존 명령어와 우선순위 비교 실행
// CAN 메세지 생성
// 내부 센서 CAN 메세지 생성
void AppTask100ms(void);

// 동작에 대한 트리거
void AppTask1000ms(void);
void AppTask5000ms(void);
void AppTask5000ms_2(void);

void handle_action_timer(void);
void handle_trigger_timer(void);
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
void init_timer(TimerInfo* timer)
{
    timer->action_counter_100ms = 0;
    timer->trigger_counter_1s = 0;
    timer->next_command = 0;
    timer->next_sub_command = 0;
    timer->next_priority = NONE;
    timer->trigger_reserved = FALSE;
}

void core0_main(void)
{
    IfxCpu_enableInterrupts();

    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
    IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
    IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());

    /* Wait for CPU sync event */
    IfxCpu_emitEvent(&g_cpuSyncEvent);
    IfxCpu_waitEvent(&g_cpuSyncEvent, 1);

    // init sensor
    init_led();
    init_sensor_driver(AIR_PIN); //Air
    //init_sensor_driver(R_PIN); //Resistance
    init_gpio_touch(SUN_TOUCH_PIN);
    init_gpio_touch(WIN_TOUCH_PIN);

    init_sound_sensor();
    init_rain_sensor();

    // init CAN
    initCan();
    initCanDB();

    Driver_Stm_Init();
    initShellInterface();

    init_queue(&audio_queue);

#ifdef TESTMODE
    smart_control_mode_on(&command_info[ALL], ENGINE_CONTROL);
    engine_flag = TRUE;
    smart_control_mode_off(&command_info[SUNROOF], DRIVER_CONTROL);
#endif

    while (1)
    {
        runShellInterface();
        AppScheduling();

        ////////////////////////////////////////////////////////////////////
        //사용자 안전 + 현재 제어 상태 받아오는 곳
        if (db_msg.motor1_window.B.Flag == 1) // 창문 메세지를 받을 때,
        {
            db_msg.motor1_window.B.Flag = 0;
            control_state.motor1_state = db_msg.motor1_window.B.motor1_running;
            window_state = db_msg.motor1_window.B.motor1_tick_counter;
            if (db_msg.motor1_window.B.motor1_running == 1) // 닫히고 있을 때,
            {
                if (window_state >= 70) // 80% 정도 닫힌 상황일 때,
                {
                    if (wintouch == 1) // 터치 센서 감지, 끼임 발생하면 안전제어 명령 수행
                    {
                        command_function(&command_info[WINDOW], OPEN, SEVENTY, SAFETY);
                        if (safety_win == FALSE)
                        {
                            db_msg.safety_window.B.motor1_smart_state = command_info[WINDOW].control_command;
                            output_message(&db_msg.safety_window, SAFETY_WINDOW_MSG_ID);
                            safety_win = TRUE;
                        }
                    }
                }
                else if (window_state <= 30)
                {
                    safety_win = FALSE;
                }
            }
        }

        if (db_msg.motor2_sunroof.B.Flag == 1) // 선루프 메세지를 받을 때,
        {
            db_msg.motor2_sunroof.B.Flag = 0;
            control_state.motor2_state = db_msg.motor2_sunroof.B.motor2_running;
            sunroof_state = db_msg.motor2_sunroof.B.motor2_tick_counter;
            if (db_msg.motor2_sunroof.B.motor2_running == 1) // 닫히고 있을 때,
            {
                if (sunroof_state >= 70) // 80% 정도 닫힌 상황일 때,
                {
                    if (suntouch == 1) // 터치 센서 감지, 끼임 발생하면 안전제어 명령 수행
                    {
                        command_function(&command_info[SUNROOF], OPEN, OPEN, SAFETY);
                        if (safety_sun == FALSE)
                        {
                            db_msg.safety_sunroof.B.motor2_smart_state = command_info[SUNROOF].control_command;
                            output_message(&db_msg.safety_sunroof, SAFETY_SUNROOF_MSG_ID);
                            safety_sun = TRUE;
                        }
                    }
                }
                else if (sunroof_state <= 30)
                {
                    safety_sun = FALSE;
                }
            }
        }

        if (db_msg.heater.B.Flag == 1) // 히터 메세지를 받을 때,
        {
            db_msg.heater.B.Flag = 0;
            control_state.heater_state = db_msg.heater.B.Heater_running;
        }

        if (db_msg.ac.B.Flag == 1) // 에어컨 메세지를 받을 때,
        {
            db_msg.ac.B.Flag = 0;
            control_state.air_state = db_msg.ac.B.AC_running;
        }

        if (db_msg.audio.B.Flag == 1) // 오디오 메세지를 받을 때,
        {
            db_msg.audio.B.Flag = 0;
            control_state.audio_state = db_msg.audio.B.Audio_running; // 오디오 동작중인지 확인
        }
        ////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////
        //사용자 제어
        if (db_msg.driver_window.B.Flag == 1) // 사용자 창문 제어일 때
        {
            //db_msg.driver_window.B.Flag = 0;
            if (db_msg.driver_window.B.driver_window == 0 || db_msg.driver_window.B.driver_window == 2) // 사용자 제어 중
            {
                //창문에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[WINDOW], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_sunroof.B.Flag == 1) // 사용자 선루프 제어일 때
        {
            db_msg.driver_sunroof.B.Flag = 0;
            if (db_msg.driver_sunroof.B.driver_sunroof == 0 || db_msg.driver_sunroof.B.driver_sunroof == 2) // 사용자 제어 중
            {
                //선루프에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[SUNROOF], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_heater.B.Flag == 1) // 사용자 히터 제어일 때
        {
            db_msg.driver_heater.B.Flag = 0;
            //if (db_msg.driver_heater.B.driver_heater == 1) // 사용자 제어 중
            //{
                //히터에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[HEATER], DRIVER_CONTROL);
            //}
        }

        if (db_msg.driver_air.B.Flag == 1) // 사용자 에어컨 제어일 때
        {
            db_msg.driver_air.B.Flag = 0;
            //if (db_msg.driver_air.B.driver_air == 1) // 사용자 제어 중
            //{
                //에어컨에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[AIR], DRIVER_CONTROL);
            //}
        }

        if (db_msg.driver_engine.B.Flag == 1) // 사용자 엔진 조작일 때
        {
            db_msg.driver_engine.B.Flag = 0;
            if (db_msg.driver_engine.B.engine_mode == 0) // 시동 끔
            {
                engine_flag = FALSE;
                smart_control_mode_off(&command_info[ALL], ENGINE_CONTROL);
            }
            else if (db_msg.driver_engine.B.engine_mode == 1) // Utility 모드
            {
                engine_flag = TRUE;
                smart_control_mode_on(&command_info[ALL], ENGINE_CONTROL);
            }
            else if (db_msg.driver_engine.B.engine_mode == 2) // 시동 On
            {
                engine_flag = TRUE;
                smart_control_mode_on(&command_info[ALL], ENGINE_CONTROL);
            }
        }

        if (db_msg.driver_control.B.Flag == 1) // 사용자 스마트 제어모드 조작일 때
        {
            db_msg.driver_control.B.Flag = 0;
            if (db_msg.driver_control.B.mode_smart == 0) // 스마트 제어 수행
            {
                //스마트 제어 모드 일괄 활성화
                smart_control_mode_on(&command_info[ALL], SMART_CONTROL);
            }
            if (db_msg.driver_control.B.mode_smart == 1) // 스마트 제어 종료
            {
                //스마트 제어 모드 일괄 비활성화
                smart_control_mode_off(&command_info[ALL], SMART_CONTROL);
            }
        }
        ////////////////////////////////////////////////////////////////////


#ifndef TESTMODE
        ////////////////////////////////////////////////////////////////////
        // 눈/비
        if (db_msg.rain.B.Flag == 1) // 빗물 감지 센서로부터 수신받음
        {
            db_msg.rain.B.Flag = 0;

            if (db_msg.rain.B.raining_status == 0) // 비 안온다.
            {
                //현재 날씨 상태 맑음으로 변경
                weather_state = NO_RAIN;
                //눈/비에 대한 로직은 창문/선루프를 열어야 할 때, 별도의 동작을 취하도록 함
            }

            else if (db_msg.rain.B.raining_status == 1) // 비 온다.
            {
                //현재 날씨 상태 비로 변경
                weather_state = RAIN;
                //눈/비에 대한 로직은 창문/선루프를 열어야 할 때, 별도의 동작을 취하도록 함
            }
        }
        ////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////
        // 터널/광량
        if (db_msg.light.B.Flag == 1)
        {
            db_msg.light.B.Flag = 0;
            light_pct = db_msg.light.B.Light_pct;

            if (tunnel_flag == FALSE && light_pct < 40) // 변경 필요
            {
                tunnel_window_state = window_state;
                tunnel_sunroof_state = sunroof_state;
                tunnel_flag = TRUE;
            }
            else if (tunnel_flag == TRUE && light_pct >= 75)
            {
                tunnel_flag = FALSE;
                if (tunnel_window_state <95)
                {
                    command_function(&command_info[WINDOW], OPEN, tunnel_window_state, TUNNEL);
                }
                if (tunnel_sunroof_state < 95)
                {
                    command_function(&command_info[SUNROOF], OPEN, OPEN, TUNNEL); // 실제로는 동작하지 못함
                }
            }
        }

        if (tunnel_flag == TRUE)
        {
            command_function(&command_info[WINDOW], CLOSE, FULL_CLOSE, TUNNEL);
            command_function(&command_info[SUNROOF], CLOSE, CLOSE, TUNNEL);
        }

        if (light_pct >= 80)
        {
            command_function(&command_info[SUNROOF], CLOSE, CLOSE, SOLAR);
        }

        ////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////
        //공기질
        if (db_msg.motor1_window.B.motor1_tick_counter >= 95 &&
                db_msg.motor2_sunroof.B.motor2_tick_counter >= 95) // 창문, 선루프 95% 정도 닫힌 상황일 때, 환기 타이머 시작
        {
            if (command_info[WINDOW].timer.trigger_counter_1s == 0 && command_info[SUNROOF].timer.trigger_counter_1s == 0) //타이머가 돌아가지 않을 때,
            {
                command_info[WINDOW].timer.trigger_counter_1s = TWO_HOUR;
                command_info[WINDOW].timer.next_command = OPEN;
                command_info[WINDOW].timer.next_sub_command = FULL_OPEN;
                command_info[WINDOW].timer.next_priority = IN_CO2;
                command_info[WINDOW].timer.trigger_reserved = TRUE;

                command_info[SUNROOF].timer.trigger_counter_1s = TWO_HOUR;
                command_info[SUNROOF].timer.next_command = OPEN;
                command_info[SUNROOF].timer.next_sub_command = OPEN;
                command_info[SUNROOF].timer.next_priority = IN_CO2;
                command_info[SUNROOF].timer.trigger_reserved = TRUE;

                command_info[HEATER].timer.trigger_counter_1s = 0;
                command_info[HEATER].timer.next_command = 0;
                command_info[HEATER].timer.next_sub_command = 0;
                command_info[HEATER].timer.next_priority = NONE;
                command_info[HEATER].timer.trigger_reserved = FALSE;

                command_info[AIR].timer.trigger_counter_1s = 0;
                command_info[AIR].timer.next_command = 0;
                command_info[AIR].timer.next_sub_command = 0;
                command_info[AIR].timer.next_priority = NONE;
                command_info[AIR].timer.trigger_reserved = FALSE;
            }
        }

        else if (db_msg.motor1_window.B.motor1_tick_counter < 95 ||
                db_msg.motor2_sunroof.B.motor2_tick_counter < 95) // 창문, 선루프 둘 중 하나라도 94%라도 열린 일 때, 환기 했다고 판단.
        {
            if (command_info[WINDOW].timer.trigger_counter_1s == 0 && command_info[SUNROOF].timer.trigger_counter_1s == 0) //타이머가 돌아가지 않을 때,
            {
                command_info[WINDOW].timer.trigger_counter_1s = 0;
                command_info[WINDOW].timer.next_command = 0;
                command_info[WINDOW].timer.next_sub_command = 0;
                command_info[WINDOW].timer.next_priority = NONE;
                command_info[WINDOW].timer.trigger_reserved = FALSE;

                command_info[SUNROOF].timer.trigger_counter_1s = 0;
                command_info[SUNROOF].timer.next_command = 0;
                command_info[SUNROOF].timer.next_sub_command = 0;
                command_info[SUNROOF].timer.next_priority = NONE;
                command_info[SUNROOF].timer.trigger_reserved = FALSE;

                needs_ventilation = FALSE; // 환기가 필요 없음
            }
            if (control_state.heater_state == 1 && (command_info[WINDOW].state == ON ||
                    command_info[SUNROOF].state == ON))
                     //타이머가 돌아가지 않을 때,
            {
                command_function(&command_info[HEATER], TURN_ON, TURN_ON, IN_TEMP);
            }
            if (control_state.air_state == 1 && (command_info[WINDOW].state == ON ||
                    command_info[SUNROOF].state == ON))
                     //타이머가 돌아가지 않을 때,
            {
                command_function(&command_info[HEATER], TURN_ON, TURN_ON, IN_TEMP);
            }
        }


        //공기질도 can_message 만들 때 처리
        if (db_msg.dust.B.Flag == 1)
        {
            db_msg.dust.B.Flag = 0;

            //미세먼지 상태 기록
            dust_state = db_msg.dust.B.weather_dust;
        }

        ////////////////////////////////////////////////////////////////////


        ////////////////////////////////////////////////////////////////////
        //내부 온도
#ifdef WINTER
        if (in_temp_hum.temperaturehigh < 22) // 실내 공기 25도 정도 측정됨 test
        {
            command_function(&command_info[HEATER], TURN_ON, TURN_ON, IN_TEMP);
            //밑에서 히터가 동작하면 타이머 시작, 1분 후에 창문/선루프 제어레벨 (7) 이하면 닫는다.
        }
        else if (in_temp_hum.temperaturehigh >= 24)//test
        {
            command_function(&command_info[HEATER], TURN_OFF, TURN_OFF, IN_TEMP);
        }
#endif
#ifdef SUMMER
        if (in_temp_hum.temperaturehigh > 26) // 실내 공기 25도 정도 측정됨 test
        {
            command_function(&command_info[AIR], TURN_ON, TURN_ON, IN_TEMP);
            //밑에서 에어컨이 동작하면 타이머 시작, 1분 후에 창문/선루프 제어레벨 (7) 이하면 닫는다.
        }
        else if (in_temp_hum.temperaturehigh <= 24) // test
        {
            command_function(&command_info[AIR], TURN_OFF, TURN_OFF, IN_TEMP);
        }
#endif
        ////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////
        //소음
        if (db_msg.db.B.Flag == 1)
        {
            db_msg.db.B.Flag = 0;

            if (db_msg.db.B.db_outside >= 80) // 80dB 이상일 때 창문, 선루프 닫음
            {
                command_function(&command_info[WINDOW], CLOSE, FULL_CLOSE, NOISE);
                command_function(&command_info[SUNROOF], CLOSE, CLOSE, NOISE);
            }
        }
#endif
        ////////////////////////////////////////////////////////////////////
    } //while(1)

    return;
}


/**
 * @brief  스마트 제어모드 끄는 함수
 * @param  * command_info: 끄는 동작, ALL의 경우 모든 모듈 스마트 제어모드 비활성화, control: 종료의 주체
 */
void smart_control_mode_off(ControlCommandInfo* command_info, ControlType control)
{
    if (control == ENGINE_CONTROL || control == SMART_CONTROL) // 스마트 제어모드 종료, 엔진 종료일 때,
    {
        for (int index = 1; index <= CONTROL_MODULE; index++)
        {
            (command_info + index)->state = OFF;
            (command_info + index)->flag = 0;
            (command_info + index)->control_command = 0;// OPEN
            (command_info + index)->sub_command = 0;
            (command_info + index)->priority = NONE;
            
            (command_info + index)->timer.action_counter_100ms = 0;
            (command_info + index)->timer.trigger_counter_1s = 0;
            (command_info + index)->timer.next_command = 0;
            (command_info + index)->timer.next_sub_command = 0;
            (command_info + index)->timer.next_priority = NONE;
            (command_info + index)->timer.trigger_reserved = FALSE;
        }

        while(queue_isempty(&audio_queue) == FALSE) // 스마트 제어 모드 끌 때, 모든 audio_queue를 비운다.
        {
            dequeue_queue(&audio_queue, &audio_file_num);
        }

        if (engine_flag == TRUE && control == SMART_CONTROL)
        {
            enqueue_queue(&audio_queue, TURN_OFF_SMART_CONTROL);
        }
    }

    else if (control == DRIVER_CONTROL) // 사용자 조작에 의한 일시적 비활성화일 때,
    {
        if (command_info->state == ON)
        {
            enqueue_queue(&audio_queue, SMART_CONTROL_PAUSE);
        }
        command_info->state = OFF;
        command_info->flag = 0;

        command_info->control_command = 0;
        command_info->sub_command = 0;
        command_info->priority = NONE;

        command_info->timer.action_counter_100ms = 0;
        command_info->timer.trigger_counter_1s = TEN_MINUTE;
        command_info->timer.next_command = 0;
        command_info->timer.next_sub_command = 0;
        command_info->timer.next_priority = NONE;
        command_info->timer.trigger_reserved = TRUE;
        
    }
}


/**
 * @brief  스마트 제어모드 켜는 함수
 * @param  * command_info: 켜는 동작, ALL의 경우 모든 모듈 스마트 제어모드 활성화, control: 활성화의 주체
 * @note   최초의 이 동작이 수행해야 스마트 제어 명령 수행
 */
void smart_control_mode_on(ControlCommandInfo* command_info, ControlType control)
{
    if (control == ENGINE_CONTROL || control == SMART_CONTROL)
    {
        for (int index = 1; index <= CONTROL_MODULE; index++)
        {
            (command_info + index)->state = ON;
            (command_info + index)->flag = 0;
            (command_info + index)->control_command = 0;
            (command_info + index)->sub_command = 0;
            (command_info + index)->priority = NONE;

            (command_info + index)->timer.action_counter_100ms = 0;
            (command_info + index)->timer.trigger_counter_1s = 0;
            (command_info + index)->timer.next_command = 0;
            (command_info + index)->timer.next_sub_command = 0;
            (command_info + index)->timer.next_priority = NONE;
            (command_info + index)->timer.trigger_reserved = FALSE;
        }
        enqueue_queue(&audio_queue, TURN_ON_SMART_CONTROL);
    }

    else if (control == DRIVER_CONTROL) // 사용자 조작에 의한 활성화일 때,
    {
        command_info->state = ON;
        command_info->flag = 0;
        command_info->control_command = 0;
        command_info->sub_command = 0;
        command_info->priority = NONE;

        command_info->timer.action_counter_100ms = 0;
        command_info->timer.trigger_counter_1s = 0;
        command_info->timer.next_command = 0;
        command_info->timer.next_sub_command = 0;
        command_info->timer.next_priority = NONE;
        command_info->timer.trigger_reserved = FALSE;
    }
}


/**
 * @brief  모듈에 대한 스마트 제어 명령어를 교체하는 함수
 * @param  command_info: 동작 수행 모듈, command: 동작(On, Off, Turn_on, Turn_off)
 *                                  sub_command: 창문/선루프의 경우 열고 닫는 정도, 오디오 모듈의 경우 재생 번호
 *                                  priority: 새로 들어온 동작의 우선순위
 */
void command_function(ControlCommandInfo* command_info, uint8 command, uint8 sub_command, PriorityType priority)
{
    if (priority == SAFETY || command_info->state == ON) // 안전 관련 동작이거나, 스마트 제어 모드가 켜져있는 상태에서
    {
        if (command_info->priority >= priority) // 우선순위가 더 높거나 같은 새로운 명령이 들어오면, 교체 (우선순위는 숫자가 낮을수록 높음)
                                                // 새로운 명령어로 교체되어도 기존 동작과 같은 동작을 수행하면 메세지는 보내지 않는다. flag 교체 x
        {
            if (command_info->control_command == command && 
                command_info->sub_command == sub_command && 
                command_info->priority == priority &&
                command_info->timer.trigger_reserved)
            {
                ;//아무런 변경하지 않음
            }
            else
            {
                command_info->priority = priority;
                command_info->control_command = command;
                command_info->sub_command = sub_command;

                if (priority == SAFETY)
                {
                    command_info->timer.action_counter_100ms = TIME_1S;
                } 
                else if (priority != IN_TEMP)
                {
                    command_info->timer.action_counter_100ms = TIME_1MIN;
                }
                else
                {
                    command_info->timer.action_counter_100ms = TIME_5MIN;
                }

                command_info->timer.trigger_reserved = FALSE;

                if (priority == IN_CO2 && command == OPEN)
                {
                    if (priority == IN_CO2)
                    {
                        command_info->timer.trigger_counter_1s = FIVE_MINUTE;
                        command_info->timer.next_command = CLOSE;
                        command_info->timer.next_sub_command = FULL_CLOSE;
                        command_info->timer.next_priority = IN_CO2;
                        command_info->timer.trigger_reserved = TRUE;
                    }
                }
                else if (priority == IN_TEMP && command == TURN_ON)
                {
                    if (priority == IN_TEMP)
                    {
                    // 히터나 에어컨을 켰을 경우
                        command_info->timer.trigger_counter_1s = ONE_MINUTE;
                        command_info->timer.next_command = CLOSE;
                        command_info->timer.next_sub_command = FULL_CLOSE;
                        command_info->timer.next_priority = IN_TEMP;
                        command_info->timer.trigger_reserved = TRUE;
                    }
                }
            }
        }
    }
}


/**
 * @brief  스마트 제어 모드 동작 후, 동일한 원인으로 다시 동작이 실행되기까지의 대기 시간을 계산하는 함수
 * @note   모듈에 대한 재실행 대기 시간이 다름
 *          안전 : 500ms, 창문/선루프 : 1min, 에어컨/히터 : 5min
 */
void handle_action_timer()
{
    for (int index = 1; index <= CONTROL_MODULE; index++)
    {
        if (command_info[index].timer.action_counter_100ms > 0) //현재 동작에 대한 카운트다운
        {
            command_info[index].timer.action_counter_100ms--;

            if (command_info[index].timer.action_counter_100ms == 0) // 현재 동작에 대한 대기 시간이 끝나면 동작 및 우선순위 초기화
            {
                command_info[index].flag = 0;
                command_info[index].control_command = 0;
                command_info[index].sub_command = 0;
                command_info[index].priority = NONE;
            }
        }
    }
}


void handle_trigger_timer()
{
    for (int index = 1; index <= CONTROL_MODULE; index++)
    {
        if (command_info[index].timer.trigger_reserved && 
            command_info[index].timer.trigger_counter_1s > 0)
        {
            command_info[index].timer.trigger_counter_1s--;
            
            if (command_info[index].timer.trigger_counter_1s == 0)
            {
                if (command_info[index].timer.next_command == 0)
                {
                    smart_control_mode_on(&command_info[index], DRIVER_CONTROL);
                }

                else if (command_info[index].timer.next_command != 0)
                {
                    // 예약된 다음 동작 실행
                    if (command_info[index].timer.next_priority == IN_TEMP)
                    {
                        if (command_info[WINDOW].state == ON)
                        {
                            command_function(&command_info[WINDOW],
                                        command_info[index].timer.next_command,
                                        command_info[index].timer.next_sub_command,
                                        command_info[index].timer.next_priority);
                        }
                        if (command_info[SUNROOF].state == ON)
                        {
                            command_function(&command_info[SUNROOF],
                                        command_info[index].timer.next_command,
                                        CLOSE,
                                        command_info[index].timer.next_priority);
                        }
                    }

                    else if (command_info[index].timer.next_priority == IN_CO2)
                    {
                        if (command_info[index].state == ON)
                        {
                            command_function(&command_info[index],
                                        command_info[index].timer.next_command,
                                        command_info[index].timer.next_sub_command,
                                        command_info[index].timer.next_priority);
                        }
                    }
                    // 트리거 정보 초기화
                    command_info[index].timer.next_command = 0;
                    command_info[index].timer.next_sub_command = 0;
                    command_info[index].timer.next_priority = NONE;
                    command_info[index].timer.trigger_reserved = FALSE;
                }
            }
        }
    }
}

/**
 * @brief  스마트 제어, 내부 센서에 대한 CAN 메세지 만들어서 output 하는 함수
 * @note   기존 동작과 반대 성질의 동작을 수행하거나, 초기화된 동작에서 새로운 동작을 수행
 */
void make_can_message()
{
    for (int index = 1; index < CONTROL_MODULE; index++)
    {
        if (command_info[index].priority == SAFETY || command_info[index].state == ON) // 안전 동작이거나 스마트 제어가 켜져있을 때,
        {
            ///////닫거나 꺼야하는 동작을 수행해야 할 때,
            if (command_info[index].control_command == CLOSE || command_info[index].control_command == TURN_OFF) //같은 값이긴 하다.
            {
                if (command_info[index].flag == 0 || command_info[index].flag == 1 || command_info[index].flag == 2) //초기화된 상태거나 현재 동작과 반대되는 상태였다면,
                {
                    if ((index == WINDOW && (window_state >= 95 || control_state.motor1_state == CLOSE)) ||
                            (index == SUNROOF && (sunroof_state >= 95 || control_state.motor2_state == CLOSE)) ||
                            (index == HEATER && control_state.heater_state == 0) ||
                            (index == AIR && control_state.air_state == 0))
                    {
                        command_info[index].flag = 4; //닫혀있는 상태
                    }
                    else
                    {
                        command_info[index].flag = 3; // 닫거나 꺼야하는 상태
                    }
                }

            }
            ///////열거나 켜야하는 동작을 수행해야 할 때,
            else if (command_info[index].control_command == OPEN || command_info[index].control_command == TURN_ON) //같은 값이긴 하다.
            {
                if (command_info[index].flag == 0 || command_info[index].flag == 3 || command_info[index].flag == 4) //초기화된 상태거나 현재 동작과 반대되는 상태였다면,
                {
                    if ((index == WINDOW && (window_state < 5 || control_state.motor1_state == OPEN)) ||
                            (index == SUNROOF && (sunroof_state < 5 || control_state.motor2_state == OPEN)) ||
                            (index == HEATER && control_state.heater_state == 1) ||
                            (index == AIR && control_state.air_state == 1))
                    {
                        command_info[index].flag = 2; // 켜져있는 상태
                    }
                    else
                    {
                        command_info[index].flag = 1; // 켜야하는 상태
                    }
                }
            }
            /////////이벤트 처리 하기 위한 flag on
            if (command_info[index].flag == 1 || command_info[index].flag == 3) // 메세지 출력해야 하는 상태
            {
                //창문 제어 명령 생성
                if (index == WINDOW)
                {
                    window_and_sunroof_state[0] = command_info[index].flag;
                    if (command_info[index].priority == SAFETY) //안전 명령어라면,
                    {
                        ;
                    }
                    else
                    {
                        db_msg.smart_window.B.motor1_smart_state = command_info[index].control_command;
                        db_msg.smart_window.B.motor1_state = command_info[index].sub_command; // 창문 열거나 닫아야 하는 정도
                        output_message(&db_msg.smart_window, SMART_WINDOW_MSG_ID);
                    }
                }

                //선루프 제어 명령 생성
                else if (index == SUNROOF)
                {
                    window_and_sunroof_state[1] = command_info[index].flag;
                    if (command_info[index].priority == SAFETY) //안전 명령어라면,
                    {
                        ;
                    }
                    else
                    {
                        db_msg.smart_sunroof.B.motor2_smart_state = command_info[index].control_command;
                        output_message(&db_msg.smart_sunroof, SMART_SUNROOF_MSG_ID);
                    }
                }

                //히터 제어 명령 생성
                else if (index == HEATER)
                {
                    //오디오 번호 입력
                    if (command_info[index].control_command == TURN_OFF)
                    {
                        enqueue_queue(&audio_queue, DEACTIVATE_HEATER);
                    }
                    else if (command_info[index].control_command == TURN_ON)
                    {
                        enqueue_queue(&audio_queue, INDOOR_COLD_AND_ACTIVATE_HEATER);
                    }

                    db_msg.smart_heater.B.Heater_state = command_info[index].control_command;
                    output_message(&db_msg.smart_heater, SMART_HEAT_MSG_ID);
                }

                //에어컨 제어 명령 생성
                else if (index == AIR)
                {
                    //오디오 번호 입력
                    if (command_info[index].control_command == TURN_OFF)
                    {
                        enqueue_queue(&audio_queue, DEACTIVATE_AIRCON);
                    }
                    else if (command_info[index].control_command == TURN_ON)
                    {
                        enqueue_queue(&audio_queue, INDOOR_HOT_AND_ACTIVATE_AIRCON);
                    }

                    db_msg.smart_ac.B.Air_state = command_info[index].control_command;
                    output_message(&db_msg.smart_ac, SMART_AC_MSG_ID);
                }
                command_info[index].flag += 1; // 메세지 출력 상태로 전환
            } // 메세지 출력해야 하는 상태 처리
        }
    }

    if (window_and_sunroof_state[0] == 1 && window_and_sunroof_state[1] == 1) // 둘 다 열면,
    {
        boolean enqueue_audio = FALSE;
        // 원인
        if (command_info[WINDOW].priority == TUNNEL && command_info[SUNROOF].priority == TUNNEL)
        {
            enqueue_queue(&audio_queue, EXIT_TUNNEL_AND_OPEN_WINDOW_AND_SUNROOF);
            enqueue_audio = TRUE;
        }
        // 원인
        else if (command_info[WINDOW].priority == IN_CO2 && command_info[SUNROOF].priority == IN_CO2)
        {
            enqueue_queue(&audio_queue, INDOOR_AIR_BAD_AND_OPEN_WINDOW_AND_SUNROOF);
            enqueue_audio = TRUE;
        }
        if (enqueue_audio == FALSE)
        {
            enqueue_queue(&audio_queue, OPEN_WINDOW_AND_SUNROOF);
        }
    }

    if (window_and_sunroof_state[0] == 3 && window_and_sunroof_state[1] == 3) // 둘 다 닫으면
    {
        boolean enqueue_audio = FALSE;
        // 원인
        if (command_info[WINDOW].priority == TUNNEL && command_info[SUNROOF].priority == TUNNEL)
        {
            enqueue_queue(&audio_queue, ENTER_TUNNEL_AND_CLOSE_WINDOW_AND_SUNROOF);
            enqueue_audio = TRUE;
        }
        // 원인
        else if (command_info[WINDOW].priority == DUST && command_info[SUNROOF].priority == DUST)
        {
            enqueue_queue(&audio_queue, HIGH_FINE_DUST_AND_CLOSE_WINDOW_AND_SUNROOF);
            enqueue_audio = TRUE;
        }

        else if (command_info[WINDOW].priority == IN_TEMP && command_info[SUNROOF].priority == IN_TEMP)
        {
            if (control_state.heater_state == 1)
            {
                enqueue_queue(&audio_queue, HEATER_AND_CLOSE_WINDOW_AND_SUNROOF);
                enqueue_audio = TRUE;
            }
            else if (control_state.air_state == 1)
            {
                enqueue_queue(&audio_queue, AIRCON_AND_CLOSE_WINDOW_AND_SUNROOF);
                enqueue_audio = TRUE;
            }
        }
        else if (command_info[WINDOW].priority == IN_CO2 && command_info[SUNROOF].priority == IN_CO2)
        {
            enqueue_queue(&audio_queue, FINISH_VENT_CLOSE_WINDOW_AND_SUNROOF);
            enqueue_audio = TRUE;
        }
        else if (command_info[WINDOW].priority == NOISE && command_info[SUNROOF].priority == NOISE)
        {
            enqueue_queue(&audio_queue, NOISE_AND_CLOSE_WINDOW_AND_SUNROOF);
            enqueue_audio = TRUE;
        }
        if (enqueue_audio == FALSE)
        {
            enqueue_queue(&audio_queue, CLOSE_WINDOW_AND_SUNROOF);
        }
    }

    else if (window_and_sunroof_state[0] == 1)
    {
        boolean enqueue_audio = FALSE;
        if (command_info[WINDOW].priority == SAFETY)
        {
            enqueue_queue(&audio_queue, SAFETY_OPEN_WINDOW);
            enqueue_audio = TRUE;
        }
        else if (command_info[WINDOW].priority == TUNNEL)
        {
            enqueue_queue(&audio_queue, EXIT_TUNNEL_AND_OPEN_WINDOW);
            enqueue_audio = TRUE;
        }
        else if (command_info[WINDOW].priority == IN_CO2)
        {
            enqueue_queue(&audio_queue, INDOOR_AIR_BAD_AND_OPEN_WINDOW);
            enqueue_audio = TRUE;
        }
        else if (command_info[WINDOW].priority == WEATHER)
        {
            enqueue_queue(&audio_queue, IS_RAIN_DETECTED_AND_OPEN_WINDOW);
            enqueue_audio = TRUE;
        }
        if (enqueue_audio == FALSE)
        {
            enqueue_queue(&audio_queue, OPEN_WINDOW);
        }
    }
    else if (window_and_sunroof_state[0] == 3)
    {
        boolean enqueue_audio = FALSE;
        if (command_info[WINDOW].priority == TUNNEL)
        {
            enqueue_queue(&audio_queue, ENTER_TUNNEL_AND_CLOSE_WINDOW);
            enqueue_audio = TRUE;
        }
        else if (command_info[WINDOW].priority == IN_TEMP)
        {
            if (control_state.heater_state == 1)
            {
                enqueue_queue(&audio_queue, HEATER_AND_CLOSE_WINDOW);
                enqueue_audio = TRUE;
            }
            else if (control_state.air_state == 1)
            {
                enqueue_queue(&audio_queue, AIRCON_AND_CLOSE_WINDOW);
                enqueue_audio = TRUE;
            }
        }
        else if (command_info[WINDOW].priority == IN_CO2)
        {
            enqueue_queue(&audio_queue, FINISH_VENT_CLOSE_WINDOW);
            enqueue_audio = TRUE;
        }
        else if (command_info[WINDOW].priority == WEATHER)
        {
            enqueue_queue(&audio_queue, IS_RAIN_DETECTED_AND_OPEN_WINDOW);
            enqueue_audio = TRUE;
        }
        else if (command_info[WINDOW].priority == NOISE)
        {
            enqueue_queue(&audio_queue, NOISE_AND_CLOSE_WINDOW);
            enqueue_audio = TRUE;
        }
        if (enqueue_audio == FALSE)
        {
            enqueue_queue(&audio_queue, CLOSE_WINDOW);
        }
    }
    else if (window_and_sunroof_state[1] == 1)
    {
        boolean enqueue_audio = FALSE;
        if (command_info[SUNROOF].priority == SAFETY)
        {
            enqueue_queue(&audio_queue, SAFETY_OPEN_SUNROOF);
            enqueue_audio = TRUE;
        }
        else if (command_info[SUNROOF].priority == TUNNEL)
        {
            enqueue_queue(&audio_queue, EXIT_TUNNEL_AND_OPEN_SUNROOF);
            enqueue_audio = TRUE;
        }
        else if (command_info[SUNROOF].priority == IN_TEMP)
        {
            if (control_state.heater_state == 1)
            {
                enqueue_queue(&audio_queue, HEATER_AND_CLOSE_SUNROOF);
                enqueue_audio = TRUE;
            }
            else if (control_state.air_state == 1)
            {
                enqueue_queue(&audio_queue, AIRCON_AND_CLOSE_SUNROOF);
                enqueue_audio = TRUE;
            }
        }
        else if (command_info[SUNROOF].priority == IN_CO2)
        {
            enqueue_queue(&audio_queue, INDOOR_AIR_BAD_AND_OPEN_SUNROOF);
            enqueue_audio = TRUE;
        }
        if (enqueue_audio == FALSE)
        {
            enqueue_queue(&audio_queue, OPEN_SUNROOF);
        }
    }
    else if (window_and_sunroof_state[1] == 3)
    {
        boolean enqueue_audio = FALSE;
        if (command_info[SUNROOF].priority == WEATHER)
        {
            enqueue_queue(&audio_queue, IS_RAIN_DETECTED_AND_CLOSE_SUNROOF);
            enqueue_audio = TRUE;
        }
        else if (command_info[SUNROOF].priority == TUNNEL)
        {
            enqueue_queue(&audio_queue, ENTER_TUNNEL_AND_CLOSE_SUNROOF);
            enqueue_audio = TRUE;
        }
        else if (command_info[SUNROOF].priority == SOLAR)
        {
            enqueue_queue(&audio_queue, SOLAR_AND_CLOSE_SUNROOF);
            enqueue_audio = TRUE;
        }
        else if (command_info[SUNROOF].priority == IN_CO2)
        {
            enqueue_queue(&audio_queue, FINISH_VENT_CLOSE_SUNROOF);
            enqueue_audio = TRUE;
        }
        else if (command_info[SUNROOF].priority == NOISE)
        {
            enqueue_queue(&audio_queue, NOISE_AND_CLOSE_SUNROOF);
            enqueue_audio = TRUE;
        }
        if (enqueue_audio == FALSE)
        {
            enqueue_queue(&audio_queue, CLOSE_SUNROOF);
        }
    }

    window_and_sunroof_state[0] = 0;
    window_and_sunroof_state[1] = 0;


    //오디오 제어 명령 생성
    if (audio_flag_5s == TRUE) // 오디오 출력 안하고 있을 때,
    {
        while (audio_flag == FALSE && queue_isempty(&audio_queue) == FALSE) // 오디오 출력 목록이 있을 경우
        {
            dequeue_queue(&audio_queue, &audio_file_num);
            audio_flag = TRUE;
        }
        if (audio_flag_5s == TRUE && audio_flag == TRUE) // 유효한 오디오 파일이라고 판단하면,
        {
            audio_flag_5s = FALSE;

            db_msg.smart_audio.B.Audio_file = audio_file_num;
            command_info[AUDIO].timer.action_counter_100ms = TIME_2S; //2초 동안 우선순위 점유
            output_message(&db_msg.smart_audio, SMART_AUDIO_MSG_ID);
            u32nuCounter1ms_2 = 0;
        }
        audio_flag = FALSE;
    }

#ifndef TESTMODE
    //내부 공기질 센서 데이터 output
    db_msg.in_air_quality.B.AQ_alive = engine_flag;
    db_msg.in_air_quality.B.air_CO2 = in_gas.CO2;
    db_msg.in_air_quality.B.air_CO = in_gas.CO;
    db_msg.in_air_quality.B.air_NH4 = in_gas.NH4;
    db_msg.in_air_quality.B.air_alch = in_gas.Alcohol;

    output_message(&db_msg.in_air_quality, IN_AIR_QUAILITY_MSG_ID);

    //내부 온습도 센서 데이터 output
    db_msg.TH_sensor.B.Temp_Hum_alive = engine_flag;
    db_msg.TH_sensor.B.Temperature = in_temp_hum.temperaturehigh;
    db_msg.TH_sensor.B.Humiditiy = in_temp_hum.huminityhigh;

    output_message(&db_msg.TH_sensor, TH_SENSOR_MSG_ID);
#endif
}

void AppTask1ms(void)
{
    stTestCnt.u32nuCnt1ms++;
}

void AppTask10ms(void)
{
    stTestCnt.u32nuCnt10ms++;
}

void AppTask100ms(void)
{
    stTestCnt.u32nuCnt100ms++;

    handle_action_timer();

    suntouch = get_touch_condition(SUN_TOUCH_PIN);
    wintouch = get_touch_condition(WIN_TOUCH_PIN);

    //내부공기질센서

#ifndef TESTMODE
    //내부 이산화탄소 1500이상
    if (in_gas.CO2 >= 1500)//Co2 나쁜 상태 매크로 상수) // 환기 필요
    {
        command_function(&command_info[WINDOW], OPEN, FULL_OPEN, IN_CO2);
        command_function(&command_info[SUNROOF], OPEN, FULL_OPEN, IN_CO2);
    }


     if (needs_ventilation == TRUE)
     {
         command_function(&command_info[SUNROOF], OPEN, FULL_OPEN, IN_CO2);
         command_function(&command_info[WINDOW], OPEN, FULL_OPEN, IN_CO2);
     }

     if (dust_state >= UNHEALTHY)
     {
         command_function(&command_info[SUNROOF], CLOSE, FULL_CLOSE, DUST);
         command_function(&command_info[WINDOW], CLOSE, FULL_CLOSE, DUST);
     }

     if (weather_state == RAIN) //비가 오는 상태일 때,
     {
         command_function(&command_info[SUNROOF], CLOSE, CLOSE, WEATHER); //선루프는 닫는다.
         if (window_state < 90)
         {
            command_function(&command_info[WINDOW], CLOSE, NINTY, WEATHER); // 90% 정도 연다.
         }
         if(command_info[WINDOW].control_command == OPEN) // 창문을 열어야 하는 상황이면,
         {
            command_function(&command_info[WINDOW], OPEN, NINTY, WEATHER); // 90% 정도 연다.
         }
     }
     else if (weather_state == NO_RAIN) // 비가 오지 않는 상태일 때,
     {
      //if (차량 속도 xkm/h 이상이면)
      //{
      //    if(command_info[WINDOW].control_command == OPEN || command_info[SUNROOF].contorl_command == OPEN) // 창문을 열거나, 선루프를 열어야 하는 상황일 때,
      //    {
      //        command_function(&command_info[SUNROOF], OPEN, FULL_OPEN, WEATHER); // 안전을 위해 선루프를 연다. 안전이지만 사용자 직접 조작보다는 우선순위가 떨어짐
      //        command_function(&command_info[WINDOW], CLOSE, FULL_CLOSE, WEATHER); // 안전을 위해 창문을 닫는다. 안전이지만 사용자 직접 조작보다는 우선순위가 떨어짐
      //    }
      //}
     }

#endif

#ifdef TESTMODE
     //test
     //control_state
     //if (control_state.motor1_state == 0 && window_state <= 80)
     if ((control_state.motor1_state == 0 && control_state.motor2_state == 0) &&
             (window_state <= 80 && sunroof_state <= 80))
     {
         command_function(&command_info[WINDOW], CLOSE, FULL_CLOSE, IN_CO2);//FINISH_VENT_CLOSE_WINDOW
         command_function(&command_info[SUNROOF], CLOSE, CLOSE, TUNNEL);

     }
//     else if (control_state.motor1_state == 0 && window_state > 20)
     else if ((control_state.motor1_state == 0 && control_state.motor2_state == 0) &&
             (window_state > 20 && sunroof_state > 20))
     {
         command_function(&command_info[WINDOW], OPEN, FULL_OPEN, SAFETY);
         command_function(&command_info[SUNROOF], OPEN, OPEN, TUNNEL);

     }
//
//     if (control_state.heater_state == 0)
//     {
//         command_function(&command_info[HEATER], TURN_ON, TURN_ON, IN_TEMP);
//     }
     //
#endif

     make_can_message();
}


/**
 * @brief  1s 주기인 온습도 센서 데이터 받고, 동작에 대한 트리거 카운터를 계산하는 함수
 * @note   창문 열린 상태에서 에어컨, 히터 동작, 환기가 필요한 시점 계산, 환기 동작 후 다시 닫아줌
 */
void AppTask1000ms(void)
{
    stTestCnt.u32nuCnt1000ms++;

    in_gas = get_air_condition();

    //스마트 제어 모드가 켜져있거나, 꺼져있는 모듈에 대한 출력
    db_msg.smart_ctrl_state.B.motor1_smart_control = command_info[WINDOW].state;
    db_msg.smart_ctrl_state.B.motor2_smart_control = command_info[SUNROOF].state;
    db_msg.smart_ctrl_state.B.heater_smart_control = command_info[HEATER].state;
    db_msg.smart_ctrl_state.B.air_smart_control = command_info[AIR].state;
    db_msg.smart_ctrl_state.B.audio_smart_control = command_info[AUDIO].state;

    output_message(&db_msg.smart_ctrl_state, SMART_CONTROL_STATE_MSG_ID);

    handle_trigger_timer();
}

void AppTask5000ms(void)
{
    stTestCnt.u32nuCnt5000ms++;
    in_temp_hum = get_temp_hum();
}

void AppTask5000ms_2(void)
{
    stTestCnt.u32nuCnt5000ms_2++;

    if (audio_flag_5s == FALSE)
    {
        audio_flag_5s = TRUE;
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
        if (stSchedulingInfo.u8nuScheduling5000msFlag == 1u)
        {
            stSchedulingInfo.u8nuScheduling5000msFlag = 0u;
            AppTask5000ms();
        }
        if (stSchedulingInfo.u8nuScheduling5000ms_2Flag == 1u)
        {
            stSchedulingInfo.u8nuScheduling5000ms_2Flag = 0u;
            AppTask5000ms_2();
        }
    }
}
/*********************************************************************************************************************/
