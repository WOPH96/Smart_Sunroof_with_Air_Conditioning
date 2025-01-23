/**********************************************************************************************************************
 * \file OurCan.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all
 * derivative works of the Software, unless such copies or derivative works are solely in the form of
 * machine-executable object code generated by a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *********************************************************************************************************************/

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
#include "OurCan.h"
#include "Platform_Types.h"
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/

/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
IfxMultican_Can can;
IfxMultican_Can_Node canNode;
IfxMultican_Can_MsgObj canMsgObjTx;
IfxMultican_Can_MsgObj canMsgObjRx;

DBMessages db_msg;

/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
IFX_INTERRUPT(RX_Int0Handler, 0, 101);
// void RX_Int0Handler (void){}
void RX_Int0Handler(void)
{
    IfxCpu_enableInterrupts();

    IfxMultican_Message readmsg;
    //    while (!IfxMultican_Can_MsgObj_isRxPending(&canMsgObjRx)){}// 수신 대기
    if (IfxMultican_Can_MsgObj_readMessage(&canMsgObjRx, &readmsg) == IfxMultican_Status_newData)
    {
        switch (readmsg.id)
        {
        case MOTOR1_WINDOW_MSG_ID:
        {
            db_msg.motor1_window.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.motor1_window.B.Flag = 1;
            //            db_msg.motor1_window = *(OurCanMotor1Window*)readmsg.data;
            //            db_msg.motor1_window.B.Flag = 1;
            break;
        }
        case MOTOR2_SUNROOF_MSG_ID:
        {
            db_msg.motor2_sunroof.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.motor2_sunroof.B.Flag = 1;
            break;
        }
        case HEATER_MSG_ID:
        {
            db_msg.heater.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.heater.B.Flag = 1;
            break;
        }
        case AIRCONDITIONER_MSG_ID:
        {
            db_msg.ac.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.ac.B.Flag = 1;
            break;
        }
        case AUDIO_MSG_ID:
        {
            db_msg.audio.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.audio.B.Flag = 1;
            break;
        }
        case BATTERY_MSG_ID:
        {
            db_msg.battery.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.battery.B.Flag = 1;
            break;
        }
        case DRIVER_WINDOW_MSG_ID:
        {
            db_msg.driver_window.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.driver_window.B.Flag = 1;
            break;
        }
        case DRIVER_SUNROOF_MSG_ID:
        {
            db_msg.driver_sunroof.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.driver_sunroof.B.Flag = 1;
            break;
        }
        case DRIVER_HEATER_MSG_ID:
        {
            db_msg.driver_heater.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.driver_heater.B.Flag = 1;
            break;
        }
        case DRIVER_AIR_MSG_ID:
        {
            db_msg.driver_air.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.driver_air.B.Flag = 1;
            break;
        }
        case DRIVER_ENGINE_MSG_ID:
        {
            db_msg.driver_engine.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.driver_engine.B.Flag = 1;
            break;
        }
        case DRIVER_CTRL_MSG_ID:
        {
            db_msg.driver_control.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.driver_control.B.Flag = 1;
            break;
        }
        case WEATHER_MSG_ID:
        {
            db_msg.weather.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.weather.B.Flag = 1;
            break;
        }
        case DUST_MSG_ID:
        {
            db_msg.dust.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.dust.B.Flag = 1;
            break;
        }
        case LIGHT_MSG_ID:
        {
            db_msg.light.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.light.B.Flag = 1;
            break;
        }
        case RAIN_MSG_ID:
        {
            db_msg.rain.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.rain.B.Flag = 1;
            break;
        }
        case DB_MSG_ID:
        {
            db_msg.db.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.db.B.Flag = 1;
            break;
        }
        case TH_SENSOR_MSG_ID:
        {
            db_msg.TH_sensor.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.TH_sensor.B.Flag = 1;
            break;
        }
        case OUT_AIR_QUALITY_MSG_ID:
        {
            db_msg.out_air_quality.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.out_air_quality.B.Flag = 1;
            break;
        }
        case IN_AIR_QUAILITY_MSG_ID:
        {
            db_msg.in_air_quality.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.in_air_quality.B.Flag = 1;
            break;
        }
        case SMART_WINDOW_MSG_ID:
        {
            db_msg.smart_window.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.smart_window.B.Flag = 1;
            break;
        }
        case SMART_SUNROOF_MSG_ID:
        {
            db_msg.smart_sunroof.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.smart_sunroof.B.Flag = 1;
            break;
        }
        case SMART_HEAT_MSG_ID:
        {
            db_msg.smart_heater.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.smart_heater.B.Flag = 1;
            break;
        }
        case SMART_AC_MSG_ID:
        {
            db_msg.smart_ac.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.smart_ac.B.Flag = 1;
            break;
        }
        case SMART_AUDIO_MSG_ID:
        {
            db_msg.smart_audio.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.smart_audio.B.Flag = 1;
            break;
        }
        case SAFETY_WINDOW_MSG_ID:
        {
            db_msg.safety_window.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.safety_window.B.Flag = 1;
            break;
        }
        case SAFETY_SUNROOF_MSG_ID:
        {
            db_msg.safety_sunroof.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.safety_sunroof.B.Flag = 1;
            break;
        }
        case SMART_CONTROL_STATE_MSG_ID:
        {
            db_msg.smart_ctrl_state.U = ((uint64)readmsg.data[1] << 32) | ((uint64)readmsg.data[0]);
            db_msg.safety_sunroof.B.Flag = 1;
            break;
        }
        default:
            break;
        }
    }
}
void initCanDB(void)
{
    // Basic Control Messages
    db_msg.motor1_window.U = 0;
    db_msg.motor2_sunroof.U = 0;
    db_msg.heater.U = 0;
    db_msg.ac.U = 0;
    db_msg.audio.U = 0;

    // Battery Message
    db_msg.battery.U = 0;

    // Driver Control Messages
    db_msg.driver_window.U = 0;
    db_msg.driver_sunroof.U = 0;
    db_msg.driver_heater.U = 0;
    db_msg.driver_air.U = 0;
    db_msg.driver_engine.U = 0;
    db_msg.driver_control.U = 0;

    // Environment Sensor Messages
    db_msg.weather.U = 0;
    db_msg.dust.U = 0;
    db_msg.light.U = 0;
    db_msg.rain.U = 0;
    db_msg.db.U = 0;
    db_msg.TH_sensor.U = 0; // 모든 값 (플래그 포함) 초기화

    // Air Quality Messages
    db_msg.out_air_quality.U = 0;
    db_msg.in_air_quality.U = 0;

    // Smart Control Messages
    db_msg.smart_window.U = 0;
    db_msg.smart_sunroof.U = 0;
    db_msg.smart_heater.U = 0;
    db_msg.smart_ac.U = 0;
    db_msg.smart_audio.U = 0;

    db_msg.smart_ctrl_state.U = 0;

    // Safety Control Messages
    db_msg.safety_window.U = 0;
    db_msg.safety_sunroof.U = 0;

    //for loop로 개선 가능할듯. long long 사이즈만큼 포인터 값이니까..
}

void output_message(void *msg, uint32 msgID)
{
    IfxMultican_Message tx_msg;
    uint32 send_data[2] = {0};

    switch (msgID)
    {
    case MOTOR1_WINDOW_MSG_ID:
    {
        OurCanMotor1Window *motor_msg = (OurCanMotor1Window *)msg;
        send_data[0] = (motor_msg->U) & FOURBYTE_F;
        send_data[1] = (motor_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, MOTOR1_WINDOW_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);

        break;
    }

    case MOTOR2_SUNROOF_MSG_ID:
    {
        OurCanMotor2Sunroof *motor_msg = (OurCanMotor2Sunroof *)msg;
        send_data[0] = (motor_msg->U) & FOURBYTE_F;
        send_data[1] = (motor_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, MOTOR2_SUNROOF_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }

    case HEATER_MSG_ID:
    {
        OurCanHeater *heat_msg = (OurCanHeater *)msg;
        send_data[0] = (heat_msg->U) & FOURBYTE_F;
        send_data[1] = (heat_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, HEATER_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }

    case AIRCONDITIONER_MSG_ID:
    {
        OurCanAC *ac_msg = (OurCanAC *)msg;
        send_data[0] = (ac_msg->U) & FOURBYTE_F;
        send_data[1] = (ac_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, AIRCONDITIONER_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case AUDIO_MSG_ID:
    {
        OurCanAudio *audio_msg = (OurCanAudio *)msg;
        send_data[0] = (audio_msg->U) & FOURBYTE_F;
        send_data[1] = (audio_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, AUDIO_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case BATTERY_MSG_ID:
    {
        OurCanBattery *battery_msg = (OurCanBattery *)msg;
        send_data[0] = (battery_msg->U) & FOURBYTE_F;
        send_data[1] = (battery_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, BATTERY_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case DRIVER_WINDOW_MSG_ID:
    {
        OurCanDriverWindow *dr_win_msg = (OurCanDriverWindow *)msg;
        send_data[0] = (dr_win_msg->U) & FOURBYTE_F;
        send_data[1] = (dr_win_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, DRIVER_WINDOW_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case DRIVER_SUNROOF_MSG_ID:
    {
        OurCanDriverSunroof *dr_sr_msg = (OurCanDriverSunroof *)msg;
        send_data[0] = (dr_sr_msg->U) & FOURBYTE_F;
        send_data[1] = (dr_sr_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, DRIVER_SUNROOF_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case DRIVER_HEATER_MSG_ID:
    {
        OurCanDriverHeater *dr_heat_msg = (OurCanDriverHeater *)msg;
        send_data[0] = (dr_heat_msg->U) & FOURBYTE_F;
        send_data[1] = (dr_heat_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, DRIVER_HEATER_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case DRIVER_AIR_MSG_ID:
    {
        OurCanDriverAir *dr_air_msg = (OurCanDriverAir *)msg;
        send_data[0] = (dr_air_msg->U) & FOURBYTE_F;
        send_data[1] = (dr_air_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, DRIVER_AIR_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case DRIVER_ENGINE_MSG_ID:
    {
        OurCanDriverEngine *dr_engine_msg = (OurCanDriverEngine *)msg;
        send_data[0] = (dr_engine_msg->U) & FOURBYTE_F;
        send_data[1] = (dr_engine_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, DRIVER_ENGINE_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }

    case DRIVER_CTRL_MSG_ID:
    {
        OurCanDriverControl *dr_ctrl_msg = (OurCanDriverControl *)msg;
        send_data[0] = (dr_ctrl_msg->U) & FOURBYTE_F;
        send_data[1] = (dr_ctrl_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, DRIVER_CTRL_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case WEATHER_MSG_ID:
    {
        OurCanWeather *weather_msg = (OurCanWeather *)msg;
        send_data[0] = (weather_msg->U) & FOURBYTE_F;
        send_data[1] = (weather_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, WEATHER_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case DUST_MSG_ID:
    {
        OurCanDust *dust_msg = (OurCanDust *)msg;
        send_data[0] = (dust_msg->U) & FOURBYTE_F;
        send_data[1] = (dust_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, DUST_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case LIGHT_MSG_ID:
    {
        OurCanLight *light_msg = (OurCanLight *)msg;
        send_data[0] = (light_msg->U) & FOURBYTE_F;
        send_data[1] = (light_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, LIGHT_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case RAIN_MSG_ID:
    {
        OurCanRain *rain_msg = (OurCanRain *)msg;
        send_data[0] = (rain_msg->U) & FOURBYTE_F;
        send_data[1] = (rain_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, RAIN_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }

    case DB_MSG_ID:
    {
        OurCanDB *db_msg = (OurCanDB *)msg;
        send_data[0] = (db_msg->U) & FOURBYTE_F;
        send_data[1] = (db_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, DB_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }

    case TH_SENSOR_MSG_ID:
    {
        OurCanTHSensor *th_msg = (OurCanTHSensor *)msg;
        send_data[0] = (th_msg->U) & FOURBYTE_F;
        send_data[1] = (th_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, TH_SENSOR_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case OUT_AIR_QUALITY_MSG_ID:
    {
        OurCanOutAirQuality *our_air_msg = (OurCanOutAirQuality *)msg;
        send_data[0] = (our_air_msg->U) & FOURBYTE_F;
        send_data[1] = (our_air_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, OUT_AIR_QUALITY_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case IN_AIR_QUAILITY_MSG_ID:
    {
        OurCanInAirQuality *in_air_msg = (OurCanInAirQuality *)msg;
        send_data[0] = (in_air_msg->U) & FOURBYTE_F;
        send_data[1] = (in_air_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, IN_AIR_QUAILITY_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case SMART_WINDOW_MSG_ID:
    {
        OurCanSmartWindow *sm_win_msg = (OurCanSmartWindow *)msg;
        send_data[0] = (sm_win_msg->U) & FOURBYTE_F;
        send_data[1] = (sm_win_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, SMART_WINDOW_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case SMART_SUNROOF_MSG_ID:
    {
        OurCanSmartSunroof *sm_sr_msg = (OurCanSmartSunroof *)msg;
        send_data[0] = (sm_sr_msg->U) & FOURBYTE_F;
        send_data[1] = (sm_sr_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, SMART_SUNROOF_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case SMART_HEAT_MSG_ID:
    {
        OurCanSmartHeater *sm_heat = (OurCanSmartHeater *)msg;
        send_data[0] = (sm_heat->U) & FOURBYTE_F;
        send_data[1] = (sm_heat->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, SMART_HEAT_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }

    case SMART_AC_MSG_ID:
    {
        OurCanSmartAC *sm_ac_msg = (OurCanSmartAC *)msg;
        send_data[0] = (sm_ac_msg->U) & FOURBYTE_F;
        send_data[1] = (sm_ac_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, SMART_AC_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case SMART_AUDIO_MSG_ID:
    {
        OurCanSmartAudio *sm_audio_msg = (OurCanSmartAudio *)msg;
        send_data[0] = (sm_audio_msg->U) & FOURBYTE_F;
        send_data[1] = (sm_audio_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, SMART_AUDIO_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case SAFETY_WINDOW_MSG_ID:
    {
        OurCanSafetyWindow *safe_win_msg = (OurCanSafetyWindow *)msg;
        send_data[0] = (safe_win_msg->U) & FOURBYTE_F;
        send_data[1] = (safe_win_msg->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, SAFETY_WINDOW_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case SAFETY_SUNROOF_MSG_ID:
    {
        OurCanSafetySunroof *safe_sr = (OurCanSafetySunroof *)msg;
        send_data[0] = (safe_sr->U) & FOURBYTE_F;
        send_data[1] = (safe_sr->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, SAFETY_SUNROOF_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }
    case SMART_CONTROL_STATE_MSG_ID:
    {
        OurCanSmartState *smart_state = (OurCanSmartState *)msg;
        send_data[0] = (smart_state->U) & FOURBYTE_F;
        send_data[1] = (smart_state->U >> 32) & FOURBYTE_F;
        IfxMultican_Message_init(&tx_msg, SMART_CONTROL_STATE_MSG_ID, send_data[0], send_data[1], IfxMultican_DataLengthCode_8);
        break;
    }

    default:
        break;
    }

    while (IfxMultican_Can_MsgObj_sendMessage(&canMsgObjTx, &tx_msg) == IfxMultican_Status_notSentBusy)
    {
    }
}

void initCan(void)
{
    // CAN 모듈 초기화
    IfxMultican_Can_Config canConfig;
    IfxMultican_Can_initModuleConfig(&canConfig, &MODULE_CAN);

    //     CAN0 인터럽트 활성화
    canConfig.nodePointer[TC275_CAN0].priority = 101;
    canConfig.nodePointer[TC275_CAN0].typeOfService = IfxSrc_Tos_cpu0;

    IfxMultican_Can_initModule(&can, &canConfig);

    // CAN 노드 초기화
    IfxMultican_Can_NodeConfig canNodeConfig;
    IfxMultican_Can_Node_initConfig(&canNodeConfig, &can);
    canNodeConfig.nodeId = IfxMultican_NodeId_0;
    canNodeConfig.rxPin = &CAN0_RX;
    canNodeConfig.rxPinMode = IfxPort_InputMode_pullUp;
    canNodeConfig.txPin = &CAN0_TX;
    canNodeConfig.txPinMode = IfxPort_OutputMode_pushPull;
    IfxMultican_Can_Node_init(&canNode, &canNodeConfig);

    // Tx 메시지 객체 초기화
    IfxMultican_Can_MsgObjConfig canMsgObjConfig;
    IfxMultican_Can_MsgObj_initConfig(&canMsgObjConfig, &canNode);
    canMsgObjConfig.msgObjId = 0; // 메시지 객체 ID
    canMsgObjConfig.messageId = CAN_TX_MESSAGE_ID;
    canMsgObjConfig.frame = IfxMultican_Frame_transmit;
    canMsgObjConfig.control.extendedFrame = FALSE;
    IfxMultican_Can_MsgObj_init(&canMsgObjTx, &canMsgObjConfig);

    // Rx 메시지 객체 초기화
    canMsgObjConfig.msgObjId = 1; // 메시지 객체 ID
    canMsgObjConfig.messageId = CAN_RX_MESSAGE_ID;
    canMsgObjConfig.acceptanceMask = 0x0; // 비교 안함, 전부 수신
    canMsgObjConfig.frame = IfxMultican_Frame_receive;
    canMsgObjConfig.control.extendedFrame = FALSE;

    // 인터럽트 활성화
    canMsgObjConfig.rxInterrupt.enabled = TRUE;
    canMsgObjConfig.rxInterrupt.srcId = TC275_CAN0;

    IfxMultican_Can_MsgObj_init(&canMsgObjRx, &canMsgObjConfig);
}

/*********************************************************************************************************************/
