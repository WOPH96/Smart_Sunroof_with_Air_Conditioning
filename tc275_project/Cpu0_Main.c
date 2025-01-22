/**********************************************************************************************************************
 * @file    Cpu0_Main.c
 * @brief   Logic Inner TC275
 * @version 0.7
 * @date    2025-01-22
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

#define ONE_MINUTE 60
#define FIVE_MINUTE 300
#define TEN_MINUTE 600
#define TWO_HOUR 7200

// 100ms 기준
#define TIME_500MS 5
#define TIME_2S 20
#define TIME_1MIN 600
#define TIME_5MIN 3000
//

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
//#define SUMMER

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
    OPEN_WINDOW,                // 창문이 열립니다.
    CLOSE_SUNROOF,              // 선루프를 닫습니다.
    OPEN_SUNROOF,               // 선루프가 열립니다.
    ACTIVATE_AIRCON,            // 에어컨을 작동합니다.
    DEACTIVATE_AIRCON,          // 에어컨을 종료합니다.
    ACTIVATE_HEATER,            // 히터를 작동합니다.
    DEACTIVATE_HEATER,          // 히터를 종료합니다.
    OPEN_WINDOW_AND_SUNROOF,    // 창문과 선루프가 열립니다.
    CLOSE_WINDOW_AND_SUNROOF,   // 창문과 선루프가 닫힙니다.
    SMART_CONTROL_PAUSE,        // 스마트 제어가 종료되고, 10분 후에 다시 작동합니다.
    HIGH_FINE_DUST,             // 미세먼지가 많습니다.
    IS_RAIN_DETECTED,           // 비가 옵니다.
    INDOOR_HOT,                 // 실내가 덥습니다.
    INDOOR_COLD,                // 실내가 춥습니다.
    INDOOR_AIR_BAD,             // 실내 공기가 안 좋습니다.
    ENTER_TUNNEL,               // 터널에 진입합니다.
    EXIT_TUNNEL,                // 터널을 통과했습니다.
    EXIT_TUNNEL_AND_OPEN_WINDOW_AND_SUNROOF, // 터널 통과 + 창문과 선루프가 열린다.
    INDOOR_AIR_BAD_AND_OPEN_WINDOW_AND_SUNROOF, // 실내 공기 안좋아 + 창문과 선루프가 열린다.
    ENTER_TUNNEL_AND_CLOSE_WINDOW_AND_SUNROOF, // 터널 진입 + 창문과 선루프를 닫는다.
    HIGH_FINE_DUST_AND_CLOSE_WINDOW_AND_SUNROOF, // 미세먼지 많아 + 창문과 선루프를 닫는다.
    IS_RAIN_DETECTED_AND_CLOSE_SUNROOF, // 비가 온다 + 선루프를 닫는다.
    INDOOR_HOT_AND_ACTIVATE_AIRCON, // 실내가 덥다 + 에어컨을 작동한다.
    INDOOR_COLD_AND_ACTIVATE_HEATER, // 실내가 춥다 + 히터를 작동한다.
}AudioControlState;
*/

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
    uint16 counter_1s;      // 닫힌 상태로 부터 시간, 환기시 시간 측정 용도, 스마트 제어 재진입 시간, 1초에 한 번 동작하도록
    uint16 counter_action_100ms; // 동작 유지 카운터. 이 값이 0이 되면 명령 우선순위, 명령 초기화, 명령에 따라 동작 유지 카운터를 달리 설정
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

Gas in_gas = {0, 0, 0, 0};
IfxPort_State suntouch = 0;
IfxPort_State wintouch = 0;
Temp_Hum in_temp_hum = { 0, 20, 20, 20, 20 };

RainState weather_state = NO_RAIN;
DustType dust_state = GOOD;

boolean needs_ventilation = FALSE;

uint32 light_adc = 0;

uint8 light_pct = 0;
uint8 window_state = 100;
uint8 sunroof_state = 100;

uint8 window_and_sunroof_state[2] = {0, 0}; // 0 초기화, 1 열어야 하는 상태, 2 열린 상태, 3 닫아야 하는 상태, 4 닫은 상태

boolean tunnel_flag = FALSE;
uint8 tunnel_window_state = 100; //터널 진입 시 현재 창문 위치 기록
uint8 tunnel_sunroof_state = 100; //터널 진입 시 현재 선루프 위치 기록

boolean audio_flag = FALSE;
QUEUE *audio_queue;
AudioControlState audio_file_num = DEFAULT;

boolean engine_flag = FALSE;
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
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
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
    init_sensor_driver(LIGHT_PIN); //Light
    init_sensor_driver(R_PIN); //Resistance
    init_gpio_touch(SUN_TOUCH_PIN);
    init_gpio_touch(WIN_TOUCH_PIN);

    init_sound_sensor();
    init_rain_sensor();

    // init CAN
    initCan();
    initCanDB();

    Driver_Stm_Init();
    initShellInterface();

    audio_queue = queue_create();

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
            if (db_msg.motor1_window.B.motor1_running == 2) // 닫히고 있을 때,
            {
                if (db_msg.motor1_window.B.motor1_tick_counter >= 80) // 80% 정도 닫힌 상황일 때,
                {
                    if (wintouch == 1) // 터치 센서 감지, 끼임 발생하면 안전제어 명령 수행
                    {
                        command_function(&command_info[WINDOW], OPEN, OPEN, SAFETY);
                    }
                }
            }
        }

        if (db_msg.motor2_sunroof.B.Flag == 1) // 선루프 메세지를 받을 때,
        {
            db_msg.motor2_sunroof.B.Flag = 0;
            control_state.motor2_state = db_msg.motor2_sunroof.B.motor2_running;
            sunroof_state = db_msg.motor2_sunroof.B.motor2_tick_counter;
            if (db_msg.motor2_sunroof.B.motor2_running == 2) // 닫히고 있을 때,
            {
                if (db_msg.motor2_sunroof.B.motor2_tick_counter >= 80) // 80% 정도 닫힌 상황일 때,
                {
                    if (suntouch == 1) // 터치 센서 감지, 끼임 발생하면 안전제어 명령 수행
                    {
                        command_function(&command_info[SUNROOF], OPEN, OPEN, SAFETY);
                    }
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
            db_msg.driver_window.B.Flag = 0;
            if (db_msg.driver_window.B.driver_window == 1) // 사용자 제어 중
            {
                //창문에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[WINDOW], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_sunroof.B.Flag == 1) // 사용자 선루프 제어일 때
        {
            db_msg.driver_sunroof.B.Flag = 0;
            if (db_msg.driver_sunroof.B.driver_sunroof == 1) // 사용자 제어 중
            {
                //선루프에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[SUNROOF], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_heater.B.Flag == 1) // 사용자 히터 제어일 때
        {
            db_msg.driver_heater.B.Flag = 0;
            if (db_msg.driver_heater.B.driver_heater == 1) // 사용자 제어 중
            {
                //히터에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[HEATER], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_air.B.Flag == 1) // 사용자 에어컨 제어일 때
        {
            db_msg.driver_air.B.Flag = 0;
            if (db_msg.driver_air.B.driver_air == 1) // 사용자 제어 중
            {
                //에어컨에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[AIR], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_engine.B.Flag == 1) // 사용자 엔진 조작일 때
        {
            db_msg.driver_engine.B.Flag = 0;
            if (db_msg.driver_engine.B.engine_mode == 0) // 시동 끔
            {
                smart_control_mode_off(&command_info[ALL], ENGINE_CONTROL);
                engine_flag = FALSE;
            }
            else if (db_msg.driver_engine.B.engine_mode == 1) // Utility 모드
            {
                smart_control_mode_on(&command_info[ALL], ENGINE_CONTROL);
                engine_flag = TRUE;
            }
            else if (db_msg.driver_engine.B.engine_mode == 2) // 시동 On
            {
                smart_control_mode_on(&command_info[ALL], ENGINE_CONTROL);
                engine_flag = TRUE;
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

            if (tunnel_flag == FALSE && light_pct < 70)
            {
                tunnel_window_state = window_state;
                tunnel_sunroof_state = sunroof_state;
                tunnel_flag = TRUE;

                command_function(&command_info[WINDOW], CLOSE, FULL_CLOSE, TUNNEL);
                command_function(&command_info[SUNROOF], CLOSE, FULL_CLOSE, TUNNEL);
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
            if (command_info[WINDOW].counter_1s == 0 || command_info[SUNROOF].counter_1s == 0) //타이머가 돌아가지 않을 때,
            {
                command_info[WINDOW].counter_1s = TWO_HOUR;
                command_info[SUNROOF].counter_1s = TWO_HOUR;

                command_info[HEATER].counter_1s = 0; // 히터, 에어컨 카운터 초기화(문열린 상태에서 작동중일 때 일정시간뒤 닫아주기 위한 카운터)
                command_info[AIR].counter_1s = 0;
            }
        }

        else if (db_msg.motor1_window.B.motor1_tick_counter < 95 ||
                db_msg.motor2_sunroof.B.motor2_tick_counter < 95) // 창문, 선루프 둘 중 하나라도 94%라도 열린 일 때, 환기 했다고 판단.
        {
            command_info[WINDOW].counter_1s = 0;
            command_info[SUNROOF].counter_1s = 0;

            needs_ventilation = FALSE; // 환기가 필요 없음
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
        if (in_temp_hum.temperaturehigh < 19)
        {
            command_function(&command_info[HEATER], TURN_ON, TURN_ON, IN_TEMP);
            //밑에서 히터가 동작하면 타이머 시작, 1분 후에 창문/선루프 제어레벨 (7) 이하면 닫는다.
        }
        else if (in_temp_hum.temperaturehigh >= 21)
        {
            command_function(&command_info[HEATER], TURN_OFF, TURN_OFF, IN_TEMP);
        }
#endif
#ifdef SUMMER
        if (in_temp_hum.temperaturehigh > 27)
        {
            command_function(&command_info[AIR], TURN_ON, TURN_ON, IN_TEMP);
            //밑에서 에어컨이 동작하면 타이머 시작, 1분 후에 창문/선루프 제어레벨 (7) 이하면 닫는다.
        }
        else if (in_temp_hum.temperaturehigh <= 25)
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
            (command_info + index)->control_command = 0;
            (command_info + index)->sub_command = 0;
            (command_info + index)->priority = NONE;
            (command_info + index)->counter_1s = 0;
            (command_info + index)->counter_action_100ms = 0;
        }

        while(queue_isempty(audio_queue) == FALSE) // 스마트 제어 모드 끌 때, 모든 audio_queue를 비운다.
        {
            queue_dequeue(audio_queue, &audio_file_num);
        }
    }

    else if (control == DRIVER_CONTROL) // 사용자 조작에 의한 일시적 비활성화일 때,
    {
        if (command_info->state == ON)
        {
            queue_enqueue(audio_queue, SMART_CONTROL_PAUSE);
        }
        command_info->state = OFF;
        command_info->flag = 0;
        command_info->counter_1s = TEN_MINUTE; //10분 후 재시작을 위함
        command_info->counter_action_100ms = 0;
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
            (command_info + index)->counter_1s = 0;
            (command_info + index)->counter_action_100ms = 0;
        }
    }

    else if (control == DRIVER_CONTROL) // 사용자 조작에 의한 활성화일 때,
    {
        command_info->state = ON;
        command_info->flag = 0;
        command_info->counter_1s = 0;
        command_info->counter_action_100ms = 0;
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
            command_info->priority = priority;
            command_info->control_command = command;
            command_info->sub_command = sub_command;
        }
    }
}


/**
 * @brief  스마트 제어 모드 동작 후, 동일한 원인으로 다시 동작이 실행되기까지의 대기 시간을 계산하는 함수
 * @note   모듈에 대한 재실행 대기 시간이 다름
 *          안전 : 500ms, 창문/선루프 : 1min, 에어컨/히터 : 5min
 */
void countdown_action_counter()
{
    for (int index = 1; index <= CONTROL_MODULE; index++)
    {
        if (command_info[index].counter_action_100ms > 0) //현재 동작에 대한 카운트다운
        {
            command_info[index].counter_action_100ms--;

            if (command_info[index].counter_action_100ms == 0) // 현재 동작에 대한 대기 시간이 끝나면 동작 및 우선순위 초기화
            {
                command_info[index].flag = 0;
                command_info[index].control_command = 0;
                command_info[index].sub_command = 0;
                command_info[index].priority = NONE;
                command_info[index].counter_1s = 0;
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
    for (int index = 1; index <= CONTROL_MODULE; index++)
    {
        if (command_info[index].priority == SAFETY || command_info[index].state == ON) // 안전 동작이거나 스마트 제어가 켜져있을 때,
        {
            ///////닫거나 꺼야하는 동작을 수행해야 할 때,
            if (command_info[index].control_command == CLOSE || command_info[index].control_command == TURN_OFF) //같은 값이긴 하다.
            {
                if (command_info[index].flag == 0 || command_info[index].flag == 1 || command_info[index].flag == 2) //초기화된 상태거나 현재 동작과 반대되는 상태였다면,
                {
                    if ((index == WINDOW && window_state >= 95) || (index == SUNROOF && sunroof_state >= 95) ||
                            (index == HEATER && control_state.heater_state == 0) || (index == AIR && control_state.air_state == 0))
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
                    if ((index == WINDOW && window_state < 95) || (index == SUNROOF && sunroof_state < 95) ||
                            (index == HEATER && control_state.heater_state == 1) || (index == AIR && control_state.air_state == 1))
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
                        db_msg.safety_window.B.motor1_smart_state = command_info[index].control_command;
                        command_info[index].counter_action_100ms = TIME_500MS; //500ms 동안 우선순위 점유
                        output_message(&db_msg.safety_window, SAFETY_WINDOW_MSG_ID);
                    }
                    else
                    {
                        db_msg.smart_window.B.motor1_smart_state = command_info[index].control_command;
                        db_msg.smart_window.B.motor1_state = command_info[index].sub_command; // 창문 열거나 닫아야 하는 정도
                        command_info[index].counter_action_100ms = TIME_1MIN; //1min 동안 우선순위 점유
                        output_message(&db_msg.smart_window, SMART_WINDOW_MSG_ID);

                        if (command_info[index].priority == IN_CO2) // 동작 원인 이산화탄소(환기)일 때,
                        {
                            if (command_info[index].control_command == OPEN) // 5분간 환기 시작
                            {
                                command_info[index].counter_1s = FIVE_MINUTE; // 5분 타이머 시작, 5분 후에 창문 닫는 동작 수행
                            }
                        }
                    }
                }

                //선루프 제어 명령 생성
                else if (index == SUNROOF)
                {
                    window_and_sunroof_state[1] = command_info[index].flag;
                    if (command_info[index].priority == SAFETY) //안전 명령어라면,
                    {
                        db_msg.safety_sunroof.B.motor2_smart_state = command_info[index].control_command;
                        command_info[index].counter_action_100ms = TIME_500MS; //500ms 동안 우선순위 점유
                        output_message(&db_msg.safety_sunroof, SAFETY_SUNROOF_MSG_ID);
                    }
                    else
                    {
                        db_msg.smart_sunroof.B.motor2_smart_state = command_info[index].control_command;
                        command_info[index].counter_action_100ms = TIME_1MIN; //1min 동안 우선순위 점유
                        output_message(&db_msg.smart_sunroof, SMART_SUNROOF_MSG_ID);

                        if (command_info[index].priority == IN_CO2) // 동작 원인 이산화탄소(환기)일 때,
                        {
                            if (command_info[index].control_command == OPEN) // 5분간 환기 시작
                            {
                                command_info[index].counter_1s = FIVE_MINUTE; // 5분 타이머 시작, 5분 후에 창문 닫는 동작 수행
                            }
                        }
                    }
                }

                //히터 제어 명령 생성
                else if (index == HEATER)
                {
                    //오디오 번호 입력
                    if (command_info[index].control_command == TURN_OFF)
                    {
                        queue_enqueue(audio_queue, DEACTIVATE_HEATER);
                    }
                    else if (command_info[index].control_command == TURN_ON)
                    {
                        queue_enqueue(audio_queue, INDOOR_COLD_AND_ACTIVATE_HEATER);
                    }

                    db_msg.smart_heater.B.Heater_state = command_info[index].control_command;
                    command_info[index].counter_action_100ms = TIME_5MIN; //5min 동안 우선순위 점유
                    output_message(&db_msg.smart_heater, SMART_HEAT_MSG_ID);

                    if (command_info[index].control_command == TURN_ON) { // 히터를 키고,
                        if (db_msg.motor1_window.B.motor1_tick_counter < 95 ||
                                db_msg.motor2_sunroof.B.motor2_tick_counter < 95) // 창문, 선루프 하나라도 95% 이상 열려있을 때,
                        {
                            if (command_info[HEATER].counter_1s == 0) // 타이머가 동작중이지 않을 때.
                            {
                                command_info[HEATER].counter_1s = ONE_MINUTE; //1분 타이머 시작
                            }
                        }
                    }
                }

                //에어컨 제어 명령 생성
                else if (index == AIR)
                {
                    //오디오 번호 입력
                    if (command_info[index].control_command == TURN_OFF)
                    {
                        queue_enqueue(audio_queue, DEACTIVATE_AIRCON);
                    }
                    else if (command_info[index].control_command == TURN_ON)
                    {
                        queue_enqueue(audio_queue, INDOOR_HOT_AND_ACTIVATE_AIRCON);
                    }

                    db_msg.smart_ac.B.Air_state = command_info[index].control_command;
                    command_info[index].counter_action_100ms = TIME_5MIN; //5min 동안 우선순위 점유
                    output_message(&db_msg.smart_ac, SMART_AC_MSG_ID);

                    if (command_info[index].control_command == TURN_ON) { // 에어컨을 키고,
                        if (db_msg.motor1_window.B.motor1_tick_counter < 95 ||
                                db_msg.motor2_sunroof.B.motor2_tick_counter < 95) // 창문, 선루프 하나라도 95% 이상 열려있을 때,
                        {
                            if (command_info[AIR].counter_1s == 0) // 타이머가 동작중이지 않을 때.
                            {
                                command_info[AIR].counter_1s = ONE_MINUTE; // 1분 타이머 시작
                            }
                        }
                    }
                }
                command_info[index].flag += 1; // 메세지 출력 상태로 전환
            } // 메세지 출력해야 하는 상태 처리

            if (window_and_sunroof_state[0] == 1 && window_and_sunroof_state[1] == 1) // 둘 다 열면,
            {
                boolean enqueue_audio = FALSE;
                // 원인
                if (command_info[WINDOW].priority == TUNNEL && command_info[SUNROOF].priority == TUNNEL)
                {
                    queue_enqueue(audio_queue, EXIT_TUNNEL_AND_OPEN_WINDOW_AND_SUNROOF);
                    enqueue_audio = TRUE;
                }
                // 원인
                else if (command_info[WINDOW].priority == IN_CO2 && command_info[SUNROOF].priority == IN_CO2)
                {
                    queue_enqueue(audio_queue, INDOOR_AIR_BAD_AND_OPEN_WINDOW_AND_SUNROOF);
                    enqueue_audio = TRUE;
                }
                if (enqueue_audio == FALSE)
                {
                    queue_enqueue(audio_queue, OPEN_WINDOW_AND_SUNROOF);
                }
            }

            if (window_and_sunroof_state[0] == 3 && window_and_sunroof_state[1] == 3) // 둘 다 닫으면
            {
                boolean enqueue_audio = FALSE;
                // 원인
                if (command_info[WINDOW].priority == TUNNEL && command_info[SUNROOF].priority == TUNNEL)
                {
                    queue_enqueue(audio_queue, ENTER_TUNNEL_AND_CLOSE_WINDOW_AND_SUNROOF);
                    enqueue_audio = TRUE;
                }
                // 원인
                else if (command_info[WINDOW].priority == DUST && command_info[SUNROOF].priority == DUST)
                {
                    queue_enqueue(audio_queue, HIGH_FINE_DUST_AND_CLOSE_WINDOW_AND_SUNROOF);
                    enqueue_audio = TRUE;
                }

                if (enqueue_audio == FALSE)
                {
                    queue_enqueue(audio_queue, CLOSE_WINDOW_AND_SUNROOF);
                }
            }

            else if (window_and_sunroof_state[0] == 1)
            {
                queue_enqueue(audio_queue, OPEN_WINDOW);
            }
            else if (window_and_sunroof_state[0] == 3)
            {
                queue_enqueue(audio_queue, CLOSE_WINDOW);
            }
            else if (window_and_sunroof_state[1] == 1)
            {
                queue_enqueue(audio_queue, OPEN_SUNROOF);
            }
            else if (window_and_sunroof_state[1] == 3)
            {
                queue_enqueue(audio_queue, CLOSE_SUNROOF);
            }

            window_and_sunroof_state[0] = 0;
            window_and_sunroof_state[1] = 0;

            //오디오 제어 명령 생성
            if (control_state.audio_state == 2) // 오디오 출력 안하고 있을 때,
            {
                while (audio_flag == FALSE && queue_isempty(audio_queue) == FALSE) // 오디오 출력 목록이 있을 경우
                {
                    queue_dequeue(audio_queue, &audio_file_num);

                    if (audio_file_num == SMART_CONTROL_PAUSE)
                    {
                        audio_flag = TRUE;
                    }

                    //창문 스마트 제어가 켜진 상태에서 창문 관련 시그널이 오면 유효한 오디오 출력이라고 판단.
                    else if (command_info[WINDOW].state == ON && (audio_file_num == CLOSE_WINDOW ||
                            audio_file_num == OPEN_WINDOW))
                    {
                        audio_flag = TRUE;
                    }

                    //선루프 스마트 제어가 켜진 상태에서 창문 관련 시그널이 오면 유효한 오디오 출력이라고 판단.
                    else if (command_info[SUNROOF].state == ON && (audio_file_num == CLOSE_SUNROOF ||
                            audio_file_num == OPEN_SUNROOF))
                    {
                        audio_flag = TRUE;
                    }

                    else if ((command_info[WINDOW].state == ON && command_info[SUNROOF].state == ON) &&
                            (audio_file_num == OPEN_WINDOW_AND_SUNROOF || audio_file_num == CLOSE_WINDOW_AND_SUNROOF))
                    {
                        audio_flag = TRUE;
                    }

                    else if (command_info[AIR].state == ON && (audio_file_num == ACTIVATE_AIRCON ||
                            audio_file_num == DEACTIVATE_AIRCON))
                    {
                        audio_flag = TRUE;
                    }

                    else if (command_info[HEATER].state == ON && (audio_file_num == ACTIVATE_HEATER ||
                            audio_file_num == DEACTIVATE_HEATER))
                    {
                        audio_flag = TRUE;
                    }
                }
                if (audio_flag == TRUE) // 유효한 오디오 파일이라고 판단하면,
                {
                    db_msg.smart_audio.B.Audio_file = audio_file_num;
                    command_info[index].counter_action_100ms = TIME_2S; //2초 동안 우선순위 점유
                    output_message(&db_msg.smart_audio, SMART_AUDIO_MSG_ID);
                }
                audio_flag = FALSE;
            }
        }
    }

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

    countdown_action_counter();

    suntouch = get_touch_condition(SUN_TOUCH_PIN);
    wintouch = get_touch_condition(WIN_TOUCH_PIN);

    //내부공기질센서

    //내부 이산화탄소 1000이상
    if (in_gas.CO2 >= 1000)//Co2 나쁜 상태 매크로 상수) // 환기 필요
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

     make_can_message();
}


/**
 * @brief  1s 주기인 온습도 센서 데이터 받고, 동작에 대한 트리거 카운터를 계산하는 함수
 * @note   창문 열린 상태에서 에어컨, 히터 동작, 환기가 필요한 시점 계산, 환기 동작 후 다시 닫아줌
 */
void AppTask1000ms(void)
{
    stTestCnt.u32nuCnt1000ms++;

    //스마트 제어 모드가 켜져있거나, 꺼져있는 모듈에 대한 출력
    db_msg.smart_ctrl_state.B.motor1_smart_control = command_info[WINDOW].state;
    db_msg.smart_ctrl_state.B.motor2_smart_control = command_info[SUNROOF].state;
    db_msg.smart_ctrl_state.B.heater_smart_control = command_info[HEATER].state;
    db_msg.smart_ctrl_state.B.air_smart_control = command_info[AIR].state;
    db_msg.smart_ctrl_state.B.audio_smart_control = command_info[AUDIO].state;

    output_message(&db_msg.smart_ctrl_state, SMART_CONTROL_STATE_MSG_ID);



    //내부 동작 카운터 줄이기
    for (int index = 1; index <= CONTROL_MODULE; index++)
    {
        if (command_info[index].counter_1s > 0) // 트리거 카운터가 있을 때
        {
            command_info[index].counter_1s--;

            //트리거 카운터가 0이 될 때,
            if (command_info[index].counter_1s == 0)
            {
                if (index == WINDOW || index == SUNROOF) // 창문, 선루프에 대한 카운터가 0이 되었을 때,
                {
                    if (command_info[index].control_command == OPEN) // 환기 5분 수행했을 때,
                    {
                        command_function(&command_info[index], CLOSE, FULL_CLOSE, IN_CO2);
                    }
                    else if (command_info[index].control_command == CLOSE) // 닫은 상태로 2시간 이상 주행했을 때,
                    {
                        needs_ventilation = TRUE;
                    }
                }

                if (index == HEATER || index == AIR) // 히터, 에어컨에 대한 카운터가 0이 되었을 때,
                {
                    if (command_info[index].control_command == TURN_ON) // 에어컨/히터 동작 후 1분 지났을 때,
                    {
                        command_function(&command_info[index], CLOSE, FULL_CLOSE, IN_TEMP);
                    }
                }
            }
        }
    }
}

void AppTask5000ms(void)
{
    stTestCnt.u32nuCnt5000ms++;
    in_temp_hum = get_temp_hum();
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
    }
}
/*********************************************************************************************************************/
