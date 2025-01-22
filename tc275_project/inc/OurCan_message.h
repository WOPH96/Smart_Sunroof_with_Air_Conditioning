/**********************************************************************************************************************
 * \file OurCan_message.h
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

#ifndef OURCAN_MESSAGE_H_
#define OURCAN_MESSAGE_H_

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
#include "OurCan_signal.h"

/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
// CANDB ID 정의
#define MOTOR1_WINDOW_MSG_ID 0x752
#define MOTOR2_SUNROOF_MSG_ID 0x751
#define HEATER_MSG_ID 0x754
#define AIRCONDITIONER_MSG_ID 0x755
#define AUDIO_MSG_ID 0x753
#define BATTERY_MSG_ID 0x76F
#define DRIVER_WINDOW_MSG_ID 0x722
#define DRIVER_SUNROOF_MSG_ID 0x721
#define DRIVER_HEATER_MSG_ID 0x724
#define DRIVER_AIR_MSG_ID 0x725
#define DRIVER_ENGINE_MSG_ID 0x726
#define DRIVER_CTRL_MSG_ID 0x720
#define WEATHER_MSG_ID 0x74D
#define DUST_MSG_ID 0x74B
#define LIGHT_MSG_ID 0x748
#define RAIN_MSG_ID 0x747
#define DB_MSG_ID 0x74E
#define OUT_AIR_QUALITY_MSG_ID 0x74A
#define IN_AIR_QUAILITY_MSG_ID 0x749
#define TH_SENSOR_MSG_ID 0x74C
#define SMART_WINDOW_MSG_ID 0x732
#define SMART_SUNROOF_MSG_ID 0x731
#define SMART_HEAT_MSG_ID 0x734
#define SMART_AC_MSG_ID 0x735
#define SMART_AUDIO_MSG_ID 0x733
#define SAFETY_WINDOW_MSG_ID 0x712
#define SAFETY_SUNROOF_MSG_ID 0x711
//추가본
#define SMART_CONTROL_STATE_MSG_ID 0x770

#define DECLARE_CAN_MESSAGE_UNION(TypeName, BitsName)        \
    typedef union                                            \
    {                                                        \
        unsigned long long U; /**< \brief Unsigned access */ \
        long long I;          /**< \brief Signed access */   \
        BitsName B;           /**< \brief Bitfield access */ \
    } TypeName;
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Data Structures---------------------------------------------------*/
DECLARE_CAN_MESSAGE_UNION(OurCanMotor1Window, OurCanMotor1WindowBits)
DECLARE_CAN_MESSAGE_UNION(OurCanMotor2Sunroof, OurCanMotor2SunroofBits)
DECLARE_CAN_MESSAGE_UNION(OurCanHeater, OurCanHeaterBits)
DECLARE_CAN_MESSAGE_UNION(OurCanAC, OurCanACBits)
DECLARE_CAN_MESSAGE_UNION(OurCanAudio, OurCanAudioBits)
DECLARE_CAN_MESSAGE_UNION(OurCanBattery, OurCanBatteryBits)
DECLARE_CAN_MESSAGE_UNION(OurCanDriverWindow, OurCanDriverWindowBits)
DECLARE_CAN_MESSAGE_UNION(OurCanDriverSunroof, OurCanDriverSunroofBits)
DECLARE_CAN_MESSAGE_UNION(OurCanDriverHeater, OurCanDriverHeaterBits)
DECLARE_CAN_MESSAGE_UNION(OurCanDriverAir, OurCanDriverAirBits)
DECLARE_CAN_MESSAGE_UNION(OurCanDriverEngine, OurCanDriverEngineBits)
DECLARE_CAN_MESSAGE_UNION(OurCanDriverControl, OurCanDriverControlBits)
DECLARE_CAN_MESSAGE_UNION(OurCanWeather, OurCanWeatherBits)
DECLARE_CAN_MESSAGE_UNION(OurCanDust, OurCanDustBits)
DECLARE_CAN_MESSAGE_UNION(OurCanLight, OurCanLightBits)
DECLARE_CAN_MESSAGE_UNION(OurCanRain, OurCanRainBits)
DECLARE_CAN_MESSAGE_UNION(OurCanDB, OurCanDBBits)
DECLARE_CAN_MESSAGE_UNION(OurCanTHSensor, OurCanTHSensorBits)
DECLARE_CAN_MESSAGE_UNION(OurCanOutAirQuality, OurCanOutAirQualityBits)
DECLARE_CAN_MESSAGE_UNION(OurCanInAirQuality, OurCanInAirQualityBits)
DECLARE_CAN_MESSAGE_UNION(OurCanSmartWindow, OurCanSmartWindowBits)
DECLARE_CAN_MESSAGE_UNION(OurCanSmartSunroof, OurCanSmartSunroofBits)
DECLARE_CAN_MESSAGE_UNION(OurCanSmartHeater, OurCanSmartHeaterBits)
DECLARE_CAN_MESSAGE_UNION(OurCanSmartAC, OurCanSmartACBits)
DECLARE_CAN_MESSAGE_UNION(OurCanSmartAudio, OurCanSmartAudioBits)
DECLARE_CAN_MESSAGE_UNION(OurCanSafetyWindow, OurCanSafetyWindowBits)
DECLARE_CAN_MESSAGE_UNION(OurCanSafetySunroof, OurCanSafetySunroofBits)
DECLARE_CAN_MESSAGE_UNION(OurCanSmartState, OurCanSmartStateBits)

/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/

#endif /* OURCAN_MESSAGE_H_ */
