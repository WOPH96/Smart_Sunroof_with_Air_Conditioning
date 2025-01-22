/**********************************************************************************************************************
 * @file    Cpu0_Main.c
 * @brief   Logic Inner TC275
 * @version 0.5
 * @date    2025-01-21
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
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
#define AIR_PIN 4
#define LIGHT_PIN 6
#define R_PIN 7
#define SUN_TOUCH_PIN 0
#define WIN_TOUCH_PIN 1

#define CONTROL_MODULE 5

#define ONE_MINUTE 60
#define FIVE_MINUTE 300
#define TEN_MINUTE 600
#define TWO_HOUR 7200

// 100ms 기준
#define TIME_500MS 5
#define TIME_1MIN 600
#define TIME_5MIN 3000
//

#define OFF 0
#define ON 1

#define CLOSE 1
#define OPEN 2
#define NINTY 3

#define TURN_OFF 1
#define TURN_ON 2
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
    IN_CO2,
    DUST,
    IN_TEMP,
    NOISE,
    NONE
}PriorityType;

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
} ControlCommandInfo;
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
Taskcnt stTestCnt;
ControlCommandInfo command_info[CONTROL_MODULE + 1]; // ALL 추가

IfxCpu_syncEvent g_cpuSyncEvent = 0;

Gas in_gas = {0, 0, 0, 0};
IfxPort_State suntouch = 0;
IfxPort_State wintouch = 0;
Temp_Hum in_temp_hum = { 0, 0, 0, 0, 0 };

RainState weather_state = NO_RAIN;
DustType dust_state = GOOD;

boolean needs_ventilation = FALSE;

uint32 light_adc = 0;
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

    smart_control_mode_on(&command_info[ALL], ENGINE_CONTROL); // 엔진 시작했다고 가정

    while (1)
    {
        runShellInterface();
        AppScheduling();

        ////////////////////////////////////////////////////////////////////

#if 1
        //사용자 안전
        if (db_msg.motor1_window.B.Flag == 1) // 창문 메세지를 받을 때,
        {
            db_msg.motor1_window.B.Flag = 0;
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
                //시동 끔, 엔진 종료 시스템 초기화
                smart_control_mode_off(&command_info[ALL], ENGINE_CONTROL);
                //_system_off();
            }
            else if (db_msg.driver_engine.B.engine_mode == 1) // Utility 모드
            {
                //저전력 모드로 수행, 예비 배터리 사용 코드 삽입
                smart_control_mode_on(&command_info[ALL], ENGINE_CONTROL);
                //
            }
            else if (db_msg.driver_engine.B.engine_mode == 2) // 시동 On
            {
                //엔진 On 시스템 초기화, 배터리 교체? 코드
                smart_control_mode_on(&command_info[ALL], ENGINE_CONTROL);
                //_system_on();
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
        // 터널
        /*
        if (터널 진입 인식 됨)
        {
            window_state = 현재 창문 모터 틱;
            sunroof_state = 현재 선루프 모터 틱;

            창문과 선루프를 닫는 명령
            command_function(&command_info[WINDOW], CLOSE, CLOSE, TUNNEL);
            command_function(&command_info[SUNROOF], CLOSE, CLOSE, TUNNEL);
        }
        else if (터널 탈출 인식 됨)
        {
            창문과 선루프를 여는 명령
            command_function(&command_info[WINDOW], OPEN, window_state, TUNNEL);
            command_function(&command_info[SUNROOF], OPEN, sunroof_state, TUNNEL);
        }
        */
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
        /*
        if (겨울에 내부 온도가 실내 적정 온도보다 낮으면)
        {
            //command_function(&command_info[HEATER], TURN_ON, TURN_ON, IN_TEMP);
            //밑에서 히터가 동작하면 타이머 시작, 1분 후에 창문/선루프 제어레벨 (7) 이하면 닫는다.
        }
        else if (겨울에 내부 온도가 실내 적정 온도에 도달하면)
        {
            //command_function(&command_info[HEATER], TURN_OFF, TURN_OFF, IN_TEMP);
        }

        if (여름에 내부 온도가 실내 적정 온도보다 높으면)
        {
            //command_function(&command_info[AIR], TURN_ON, TURN_ON, IN_TEMP);
            //밑에서 에어컨이 동작하면 타이머 시작, 1분 후에 창문/선루프 제어레벨 (7) 이하면 닫는다.
        }
        else if (여름에 내부 온도가 실내 적정 온도에 도달하면)
        {
            //command_function(&command_info[AIR], TURN_OFF, TURN_OFF, IN_TEMP);
        }
        */
        ////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////
        //소음
        if (db_msg.db.B.Flag == 1)
        {
            db_msg.db.B.Flag = 0;

            if (db_msg.db.B.db_outside >= 80) // 80dB 이상일 때 창문, 선루프 닫음
            {
                command_function(&command_info[WINDOW], CLOSE, CLOSE, NOISE);
                command_function(&command_info[SUNROOF], CLOSE, CLOSE, NOISE);
            }
        }
        ////////////////////////////////////////////////////////////////////
#endif
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
    }

    else if (control == DRIVER_CONTROL) // 사용자 조작에 의한 일시적 비활성화일 때,
    {
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
                    command_info[index].flag = 3; // 닫거나 꺼야하는 상태
                }

            }
            ///////열거나 켜야하는 동작을 수행해야 할 때,
            else if (command_info[index].control_command == OPEN || command_info[index].control_command == TURN_ON) //같은 값이긴 하다.
            {
                if (command_info[index].flag == 0 || command_info[index].flag == 3 || command_info[index].flag == 4) //초기화된 상태거나 현재 동작과 반대되는 상태였다면,
                {
                    command_info[index].flag = 1; // 열거나 켜야하는 상태
                }
            }
            /////////이벤트 처리 하기 위한 flag on
            if (command_info[index].flag == 1 || command_info[index].flag == 3) // 메세지 출력해야 하는 상태
            {
                //창문 제어 명령 생성
                if (index == WINDOW)
                {
                    if (command_info[index].priority == SAFETY) //안전 명령어라면,
                    {
                        //오디오 번호 입력
                        //command_info[AUDIO].sub_command = 안전 창문 제어;

                        db_msg.safety_window.B.motor1_smart_state = command_info[index].control_command;
                        command_info[index].counter_action_100ms = TIME_500MS; //500ms 동안 우선순위 점유
                        output_message(&db_msg.safety_window, SAFETY_WINDOW_MSG_ID);
                    }
                    else
                    {
                        //오디오 번호 입력
                        //command_info[AUDIO].sub_command = 스마트 창문 여는 제어, 스마트 창문 닫는 제어;
                        
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
                    if (command_info[index].priority == SAFETY) //안전 명령어라면,
                    {
                        //오디오 번호 입력
                        //command_info[AUDIO].sub_command = 안전 선루프 제어;

                        db_msg.safety_sunroof.B.motor2_smart_state = command_info[index].control_command;
                        command_info[index].counter_action_100ms = TIME_500MS; //500ms 동안 우선순위 점유
                        output_message(&db_msg.safety_sunroof, SAFETY_SUNROOF_MSG_ID);
                    }
                    else
                    {
                        //오디오 번호 입력
                        //command_info[AUDIO].sub_command = 스마트 선루프 여는 제어, 스마트 선루프 닫는 제어;

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
                    //command_info[AUDIO].sub_command = 스마트 히터 켜는 제어, 스마트 히터 끄는 제어;

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
                    //command_info[AUDIO].sub_command = 스마트 에어컨 켜는 제어, 스마트 에어컨 끄는 제어;

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
            }
            //오디오 제어 명령 생성
            //오디오 로직 수정 필요
            //다른 모듈에서 동시 작동이 수행되면 하나만 출력,
            //새로운 작동이 1분 내에 동작하면 출력하지 않음,

            if (command_info[AUDIO].counter_action_100ms == 0)
            {
                db_msg.smart_audio.B.Audio_file = command_info[AUDIO].sub_command;
                command_info[index].counter_action_100ms = TIME_1MIN; //1min 동안 우선순위 점유
                output_message(&db_msg.smart_audio, SMART_AUDIO_MSG_ID);
            }
        }
    }

    //내부 공기질 센서 데이터 output
    db_msg.in_air_quality.B.air_CO2 = in_gas.CO2;
    db_msg.in_air_quality.B.air_CO = in_gas.CO;
    db_msg.in_air_quality.B.air_NH4 = in_gas.NH4;
    db_msg.in_air_quality.B.air_alch = in_gas.Alcohol;

    //test
    in_gas.CO2 = 1100;
    //

    output_message(&db_msg.in_air_quality, IN_AIR_QUAILITY_MSG_ID);

    //내부 온습도 센서 데이터 output
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
          //        command_function(&command_info[SUNROOF], OPEN, OPENs, WEATHER); // 안전을 위해 선루프를 연다. 안전이지만 사용자 직접 조작보다는 우선순위가 떨어짐
          //        command_function(&command_info[WINDOW], CLOSE, CLOSE, WEATHER); // 안전을 위해 창문을 닫는다. 안전이지만 사용자 직접 조작보다는 우선순위가 떨어짐
          //    }
          //}
    }


    //내부 이산화탄소 1000이상
    if (in_gas.CO2 >= 1000)//Co2 나쁜 상태 매크로 상수) // 환기 필요
    {
        command_function(&command_info[WINDOW], OPEN, OPEN, IN_CO2);
        command_function(&command_info[SUNROOF], OPEN, OPEN, IN_CO2);
    }


     if (needs_ventilation == TRUE)
     {
         command_function(&command_info[SUNROOF], OPEN, OPEN, IN_CO2);
         command_function(&command_info[WINDOW], OPEN, OPEN, IN_CO2);
     }

     if (dust_state >= UNHEALTHY)
     {
         command_function(&command_info[SUNROOF], CLOSE, CLOSE, DUST);
         command_function(&command_info[WINDOW], CLOSE, CLOSE, DUST);
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

    in_temp_hum = get_temp_hum();

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
                        command_function(&command_info[index], CLOSE, CLOSE, IN_CO2);
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
                        command_function(&command_info[index], CLOSE, CLOSE, IN_TEMP);
                    }
                }
            }
        }
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
    }
}
/*********************************************************************************************************************/
