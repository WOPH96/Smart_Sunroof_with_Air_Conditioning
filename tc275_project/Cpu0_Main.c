/**********************************************************************************************************************
 * @file    Cpu0_Main.c
 * @brief   Logic Inner TC275
 * @version 0.1
 * @date    2025-01-21
 *********************************************************************************************************************/


/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
#include <sensors/rain_sensor.h>
#include <sensors/sound_sensor.h>
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
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

#include "sound_sensor.h"
#include "rain_sensor.h"
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
#define AIR_PIN 4
#define LIGHT_PIN 6
#define R_PIN 7


#define CONTROL_MODULE 5

#define ALL 0
#define WINDOW 1
#define SUNROOF 2
#define HEATER 3
#define AIR 4
#define AUDIO 5

#define OFF 0
#define ON 1

#define CLOSE 1
#define OPEN 2

#define NINTY 3

#define TURN_ON 1
#define TURN_OFF 2


#define DRIVER_CONTROL 0
#define SMART_CONTROL 1
#define ENGINE_CONTROL 2

#define TEN_MINUTE 600

#define NO_RAIN 0
#define RAIN 1

#define GOOD 0
#define MODERATE 1
#define UNHEALTHY 2
#define VERY_UNHEALTHY 3

#define SAFETY 1
#define DIRECT_CONTROL 2
#define WEATHER 3
#define TUNNEL 4
#define INNER_CO2 5
#define DUST 6
#define INNER_TEMP 7
#define NOISE 8
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Data Structures---------------------------------------------------*/
typedef struct
{
    boolean state; // On, Off 상태
    uint8 flag; // message out 상태 표시
    //0 : 초기값
    //1 : Open, Turn On 동작 대기, 2 : Open, Turn On 동작 중(메세지 보냄)
    //3 : Close, Turn Off 동작 대기, 4 : Close, Turn Off 동작 중(메세지 보냄)

    uint8 control_command; // Close, Open, Turn_Off, Turn_On
    uint8 option; // 창문 여는 정도,
    uint8 priority; // 명령 우선 순위
    uint16 counter_1s; // 닫힌 상태로 부터 시간, 환기시 시간 측정 용도, 1초에 한 번 동작하도록
    uint8 counter_action; // 동작 유지 카운터. 이 값이 0이 되면 명령 우선순위, 명령 초기화, 명령에 따라 동작 유지 카운터를 달리 설정
} control_command_info;

typedef struct
{
    uint32 u32nuCnt1ms;
    uint32 u32nuCnt10ms;
    uint32 u32nuCnt100ms;
    uint32 u32nuCnt1000ms;

} Taskcnt;
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
Taskcnt stTestCnt;

control_command_info command_info[CONTROL_MODULE + 1]; // ALL 추가

IfxCpu_syncEvent g_cpuSyncEvent = 0;

float dB = 0.0f;
float maxdB = 0.0f;
boolean is_rain = FALSE;
uint32 rain_adc = 0.0f;
Gas cur_gas = {0, 0, 0, 0};
uint32 resist_adc=0;
uint32 light_adc = 0;
IfxPort_State w_touch = 0;
IfxPort_State s_touch = 0;

boolean weather_state = 0;
uint8 dust_state = 0;
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
void smart_control_mode_off(control_command_info* command_info, uint8 option);
// 사용자 조작에 의한 off인지, 시동 off에 의한 off인지, 스마트 제어모드 off에 의한 off인지 구별
// 사용자 조작에 의한 off이면 counter 값 10분으로 설정

void smart_control_mode_on(control_command_info* command_info, uint8 option);

void command_function(control_command_info* command_info, uint8 command, uint8 option, uint8 priority);

void make_can_message(void);

void AppScheduling(void);
void AppTask1ms(void);
void AppTask10ms(void);
void AppTask100ms(void);
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
    init_gpio();
    init_sensor_driver(AIR_PIN); //Air
    init_sensor_driver(LIGHT_PIN); //Light
    init_sensor_driver(R_PIN); //Resistance

    init_sound_sensor();
    init_rain_sensor();

    // init CAN
    initCan();
    initCanDB();


    // 어떤 명령을 수행하는지..
    //

    while(1)
    {
        AppScheduling();

        ////////////////////////////////////////////////////////////////////
        //사용자 안전
        if (db_msg.motor1_window.B.Flag == 1) // 창문이 동작중일 때,
        {
            db_msg.motor1_window.B.Flag = 0;
            if (db_msg.motor1_window.B.motor1_running == 2) // 닫히고 있을 때,
            {
                if (db_msg.motor1_window.B.motor1_tick_counter >= 80) // 80% 정도 닫힌 상황일 때,
                {
                    if (w_touch == 1) // 터치 센서 감지, 끼임 발생하면 안전제어 명령 수행
                    {
                        //어떤 동작을 수행해야하는지, 우선순위 정보를 가져서 스케줄링으로 명령 수행?
                        command_function(&command_info[WINDOW], OPEN, OPEN, SAFETY);
                    }
                }
            }
        }

        if (db_msg.motor2_sunroof.B.Flag == 1) // 선루프가 동작중일 때,
        {
            db_msg.motor2_sunroof.B.Flag = 0;
            if (db_msg.motor2_sunroof.B.motor2_running == 2) // 닫히고 있을 때,
            {
                if (db_msg.motor2_sunroof.B.motor2_tick_counter >= 80) // 80% 정도 닫힌 상황일 때,
                {
                    if (w_touch == 1) // 터치 센서 감지, 끼임 발생하면 안전제어 명령 수행
                    {
                        //어떤 동작을 수행해야하는지, 우선순위 정보를 가져서 스케줄링으로 명령 수행?
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
            db_msg.driver_window.B.Flag = 0;  // 필수 처리!
            // 처리 로직 예시
            if (db_msg.driver_window.B.driver_window == 1) // 사용자 제어 중
            {
                //창문에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[WINDOW], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_sunroof.B.Flag == 1) // 사용자 선루프 제어일 때
        {
            db_msg.driver_sunroof.B.Flag = 0;  // 필수 처리!
            // 처리 로직 예시
            if (db_msg.driver_sunroof.B.driver_sunroof == 1) // 사용자 제어 중
            {
                //선루프에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[SUNROOF], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_heater.B.Flag == 1) // 사용자 히터 제어일 때
        {
            db_msg.driver_heater.B.Flag = 0;  // 필수 처리!
            // 처리 로직 예시
            if (db_msg.driver_heater.B.driver_heater == 1) // 사용자 제어 중
            {
                //히터에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[HEATER], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_air.B.Flag == 1) // 사용자 에어컨 제어일 때
        {
            db_msg.driver_air.B.Flag = 0;  // 필수 처리!
            // 처리 로직 예시
            if (db_msg.driver_air.B.driver_air == 1) // 사용자 제어 중
            {
                //에어컨에 대한 스마트 제어모드 10분간 종료, 재진입 시간 초기화
                smart_control_mode_off(&command_info[AIR], DRIVER_CONTROL);
            }
        }

        if (db_msg.driver_engine.B.Flag == 1) // 사용자 엔진 조작일 때
        {
            db_msg.driver_engine.B.Flag = 0;  // 필수 처리!
            // 처리 로직 예시
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
            db_msg.driver_control.B.Flag = 0;  // 필수 처리!
            // 처리 로직 예시
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
            //command_function(&command_info[HEATER], TURN_ON, TURN_ON, INNER_TEMP);
            //밑에서 히터가 동작하면 타이머 시작, 1분 후에 창문/선루프 제어레벨 (7) 이하면 닫는다.
        }
        else if (겨울에 내부 온도가 실내 적정 온도에 도달하면)
        {
            //command_function(&command_info[HEATER], TURN_OFF, TURN_OFF, INNER_TEMP);
        }

        if (여름에 내부 온도가 실내 적정 온도보다 높으면)
        {
            //command_function(&command_info[AIR], TURN_ON, TURN_ON, INNER_TEMP);
            //밑에서 에어컨이 동작하면 타이머 시작, 1분 후에 창문/선루프 제어레벨 (7) 이하면 닫는다.
        }
        else if (여름에 내부 온도가 실내 적정 온도에 도달하면)
        {
            //command_function(&command_info[AIR], TURN_OFF, TURN_OFF, INNER_TEMP);
        }
        */
        ////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////
        //소음
        if (db_msg.db.B.Flag == 1)
        {
            db_msg.db.B.Flag = 0;

            if (db_msg.db.B.db_outside >= 80)
            {
                command_function(&command_info[WINDOW], CLOSE, CLOSE, NOISE);
                command_function(&command_info[SUNROOF], CLOSE, CLOSE, NOISE);
            }
        }
        ////////////////////////////////////////////////////////////////////

    } //while(1)

    return;
}

void smart_control_mode_off(control_command_info* command_info, uint8 option)
{
    if (option == ENGINE_CONTROL || option == SMART_CONTROL)
    {
        for (int i = 1; i <= CONTROL_MODULE; i++)
        {
            (command_info + i)->state = OFF;
            (command_info + i)->flag = 0;
            (command_info + i)->control_command = 0;
            (command_info + i)->option = 0;
            (command_info + i)->priority = 0;
            (command_info + i)->counter_1s = 0;
            (command_info + i)->counter_action = 0;
        }
    }

    else if (option == DRIVER_CONTROL)
    {
        command_info->state = OFF;
        (command_info)->flag = 0;
        command_info->counter_1s = TEN_MINUTE;
        command_info->counter_action = 0;
    }
}

void smart_control_mode_on(control_command_info* command_info, uint8 option)
{
    if (option == ENGINE_CONTROL || option == SMART_CONTROL)
    {
        for (int i = 1; i <= CONTROL_MODULE; i++)
        {
            (command_info + i)->state = ON;
            (command_info + i)->flag = 0;
            (command_info + i)->control_command = 0;
            (command_info + i)->option = 0;
            (command_info + i)->priority = 0;
            (command_info + i)->counter_1s = 0;
            (command_info + i)->counter_action = 0;
        }
    }

    else if (option == DRIVER_CONTROL)
    {
        command_info->state = ON;
        command_info->flag = 0;
        command_info->counter_1s = 0;
        command_info->counter_action = 0;
    }
}

void command_function(control_command_info* command_info, uint8 command, uint8 option, uint8 priority)
{
    if (command_info->state == ON)
    {
        if (command_info->priority <= priority)
        {
            command_info->priority = priority;
            command_info->control_command = command;
            command_info->option = option;
        }
    }
}

void make_can_message()
{
    for (int i = 1; i <= CONTROL_MODULE; i++)
    {
        if (command_info[i].state == ON) // 스마트 제어가 켜져있을 때, 이벤트 출력이므로 현재 동작과 반대되는 동작이 대기 상태일 때,
        {
            ///////기존 동작과 반대되는 동작 발생
            if (command_info[i].control_command == CLOSE || command_info[i].control_command == TURN_OFF) //같은 값이긴 하다.
            {
                if (command_info[i].flag == 0 || command_info[i].flag == 1 || command_info[i].flag == 2) //초기값, 열어야 하는 상태, 열린 상태일 때,
                {
                    command_info[i].flag = 3; // 닫거나 꺼야하는 상태
                }

            }
            else if (command_info[i].control_command == OPEN || command_info[i].control_command == TURN_ON) //같은 값이긴 하다.
            {
                if (command_info[i].flag == 0 || command_info[i].flag == 3 || command_info[i].flag == 4) //닫은 상태, 닫아야 하는 상태면
                {
                    command_info[i].flag = 1; // 열고 켜야하는 상태
                }
            }
            /////////이벤트 처리 하기 위한 flag on
            if (command_info[i].flag == 1 || command_info[i].flag == 3) // 메세지 출력해야 하는 상태
            {
                if (i == WINDOW)
                {
                    if (command_info[i].priority == SAFETY)
                    {
                        db_msg.safety_window.B.motor1_smart_state = command_info[i].control_command;
                        output_message(&db_msg.safety_window, SAFETY_WINDOW_MSG_ID);
                    }
                    else
                    {
                        db_msg.smart_window.B.motor1_smart_state = command_info[i].control_command;
                        db_msg.smart_window.B.motor1_state = command_info[i].option;
                        output_message(&db_msg.smart_window, SMART_WINDOW_MSG_ID);
                    }
                }

                else if (i == SUNROOF)
                {
                    if (command_info[i].priority == SAFETY)
                    {
                        db_msg.safety_sunroof.B.motor2_smart_state = command_info[i].control_command;
                        output_message(&db_msg.safety_sunroof, SAFETY_SUNROOF_MSG_ID);
                    }
                    else
                    {
                        db_msg.smart_sunroof.B.motor2_smart_state = command_info[i].control_command;
                        output_message(&db_msg.smart_sunroof, SMART_SUNROOF_MSG_ID);
                    }
                }
                else if (i == HEATER)
                {
                    db_msg.smart_heater.B.Heater_state = command_info[i].control_command;
                    output_message(&db_msg.smart_heater, SMART_HEAT_MSG_ID);
                }
                else if (i == AIR)
                {
                    db_msg.smart_ac.B.Air_state = command_info[i].control_command;
                    output_message(&db_msg.smart_ac, SMART_AC_MSG_ID);
                }
                else if (i == AUDIO)
                {
                    db_msg.smart_audio.B.Audio_file = command_info[i].option;
                    output_message(&db_msg.smart_audio, SMART_AUDIO_MSG_ID);
                }

                command_info[i].flag += 1; // 메세지 출력 상태로 전환
            }
        }
    }

    //내부 공기질 센서 데이터 output
    //db_msg.in_air_quality.B.air_CO2 = in_air_quality.CO2;
    //db_msg.in_air_quality.B.air_CO = in_air_quality.CO;
    //db_msg.in_air_quality.B.air_NH4 = in_air_quality.NH4;
    //db_msg.in_air_quality.B.air_alch = in_air_quality.alch;

    output_message(&db_msg.in_air_quality, IN_AIR_QUAILITY_MSG_ID);

    //내부 온습도 센서 데이터 output
    //db_msg.TH_sensor.B.Temperature = TH.temp;
    //db_msg.TH_sensor.B.Humidity = TH.hum;

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

    //w_touch = get_touch_condition(); // 창문 쪽 터치센서
    //s_touch = get_touch_condition(); // 선루프 쪽 터치센서

    //내부공기질센서

    if (weather_state == RAIN)
    {
        command_function(&command_info[SUNROOF], CLOSE, CLOSE, 3);
        if(command_info[WINDOW].control_command == OPEN)
        {
            command_function(&command_info[WINDOW], OPEN, NINTY, 3);
        }
    }
    else if (weather_state == NO_RAIN)
    {
          //if (차량 속도 xkm/h 이상이면)
          //{
          //    if(command_info[WINDOW].control_command == OPEN || command_info[SUNROOF].contorl_command == OPEN)
          //    {
          //        command_function(&command_info[SUNROOF], OPEN, OPENs, 3);
          //        command_function(&command_info[WINDOW], CLOSE, CLOSE, 3);
          //    }
          //}
    }

    /*
    내부 이산화탄소 1000이상
    if (in_air_quality.CO2 >= 1000)//Co2 나쁜 상태 매크로 상수) // 환기 필요
    {
        command_function(&command_info[WINDOW], OPEN, OPEN, INNER_CO2);
        command_function(&command_info[SUNROOF], OPEN, OPEN, INNER_CO2);
    }
    */
    /*
    if (창문, 선루프 안열은지 2시간 됨)
    {
            command_function(&command_info[WINDWO], OPEN, OPEN, INNER_CO2);
            command_function(&command_info[SUNROOF], OPEN, OPEN, INNER_CO2);

            //창문, 선루프 시간 체크 시작
    }
    */

     if (dust_state >= UNHEALTHY)
     {
         command_function(&command_info[SUNROOF], CLOSE, CLOSE, DUST);
         command_function(&command_info[WINDOW], CLOSE, CLOSE, DUST);
     }

     make_can_message();
}

void AppTask1000ms(void)
{
    stTestCnt.u32nuCnt1000ms++;

    //내부온습도센서 받아오기

    //내부 동작 카운터 줄이기
    for (int i = 1; i <= CONTROL_MODULE; i++)
    {
        if (command_info[i].counter_1s > 0)
        {
            command_info[i].counter_1s--;

            //내부 카운터가 0이 될 때,
            if (command_info[i].counter_1s == 0)
            {

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
