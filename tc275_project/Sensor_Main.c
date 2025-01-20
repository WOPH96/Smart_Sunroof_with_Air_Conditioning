/*******************************************************************************
 * @file    Sensor_main.c
 * @brief   Get Sensor value body
 * @version 1.0
 * @date    2025-01-20
 ******************************************************************************/
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "Driver_Adc.h"
#include "Driver_Easy.h"
#include "Driver_Touch.h"
#include "Driver_Air.h"

#include "sound_sensor.h"
#include "rain_sensor.h"

#define AIR_PIN 4
#define LIGHT_PIN 6
#define R_PIN 7
IfxCpu_syncEvent g_cpuSyncEvent = 0;


float dB = 0.0f;
float maxdB = 0.0f;
boolean is_rain = FALSE;
uint32 rain_adc = 0.0f;
Gas gas_adc = {0.0f,0.0f,0.0f,0.0f};
uint32 resist_adc=0;
uint32 light_adc = 0;
IfxPort_State touch = 0;
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
    init_led();
    init_gpio();
    init_sensor_driver(AIR_PIN); //Air
    init_sensor_driver(LIGHT_PIN); //Light
    init_sensor_driver(R_PIN); //Resistance


    init_sound_sensor();
    init_rain_sensor();

    while(1)
    {
        gas_adc = get_air_condition();
        light_adc = get_light_condition();
        resist_adc= get_resist_condition();
        touch = get_touch_condition();

        dB = get_decibel();
        if (maxdB < dB) {
            maxdB = dB;
        }

        is_rain = is_raining();
        rain_adc = get_rain_sensor_analog_value();
    }
    return;
}
