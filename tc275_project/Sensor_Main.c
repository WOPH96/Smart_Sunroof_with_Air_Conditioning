
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "Driver_Adc.h"
#include "Driver_Easy.h"
#include "Driver_Touch.h"
#include "Driver_Air.h"

#include "sound_sensor.h"
#include "rain_sensor.h"

#define Air_Pin 4
#define Light_Pin 6
#define R_Pin 7
IfxCpu_syncEvent g_cpuSyncEvent = 0;


float dB = 0.0f;
float maxdB = 0.0f;
boolean is_rain = FALSE;
uint32 rain_adc = 0.0f;
Gas gas = {0.0f,0.0f,0.0f,0.0f};
uint32 resist=0;
uint32 light = 0;
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
    initLED();
    Driver_Adc_Init(Air_Pin); //Air
    Driver_Adc_Init(Light_Pin); //Light
    Driver_Adc_Init(R_Pin); //Resistance
    initGPIO();

    init_sound_sensor();
    init_rain_sensor();

    while(1)
    {
        gas = Air();
        light = Light();
        resist= Resist();
        touch = Touch();

        dB = get_decibel();
        if (maxdB < dB) {
            maxdB = dB;
        }

        is_rain = is_raining();
        rain_adc = get_rain_sensor_analog_value();
    }
    return;
}
