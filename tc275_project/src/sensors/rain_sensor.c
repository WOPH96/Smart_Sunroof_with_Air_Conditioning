/**********************************************************************************************************************
 * @file        rain_sensor.c
 * @brief       Driver for rain sensor
 * @version     1.0
 * @date        2025-01-19
 *********************************************************************************************************************/

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
#include <sensors/rain_sensor.h>
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
//adc를 위한 변수
RainSensorVadcAutoScan g_rain_sensor_scan;
IfxVadc_Adc_ChannelConfig adc_channel_config_rain_sensor[8];
IfxVadc_Adc_Channel   adc_channel_rain_sensor[8];
uint32 adc_result_rain_sensor[8] = {0u,};

//디버깅을 위한 변수
uint32 g_rain_sensor_adc_result = 0;
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/


/*
 * @brief  빗물감지 센서(A9, SAR0.2)초기화 함수
 */
void init_rain_sensor(void)
{
    uint32    chnIx = 2;

    /* VADC Configuration */

    /* create configuration */
    IfxVadc_Adc_Config adcConfig;
    IfxVadc_Adc_initModuleConfig(&adcConfig, &MODULE_VADC);

    /* initialize module */
    IfxVadc_Adc_initModule(&g_rain_sensor_scan.vadc, &adcConfig);

    /* create group config */
    IfxVadc_Adc_GroupConfig adcGroupConfig;
    IfxVadc_Adc_initGroupConfig(&adcGroupConfig, &g_rain_sensor_scan.vadc);

    /* with group 0 */
    adcGroupConfig.groupId = IfxVadc_GroupId_0;
    adcGroupConfig.master  = adcGroupConfig.groupId;

    /* enable scan source */
    adcGroupConfig.arbiter.requestSlotScanEnabled = TRUE;

    /* enable auto scan */
    adcGroupConfig.scanRequest.autoscanEnabled = TRUE;

    /* enable all gates in "always" mode (no edge detection) */
    adcGroupConfig.scanRequest.triggerConfig.gatingMode =
                                    IfxVadc_GatingMode_always;

    /* initialize the group */
    /*IfxVadc_Adc_Group adcGroup;*/    //declared globally
    IfxVadc_Adc_initGroup(&g_rain_sensor_scan.adcGroup, &adcGroupConfig);

    /*channel init*/
    IfxVadc_Adc_initChannelConfig(&adc_channel_config_rain_sensor[chnIx],
                                            &g_rain_sensor_scan.adcGroup);

    adc_channel_config_rain_sensor[chnIx].channelId      =
            (IfxVadc_ChannelId)(chnIx);
    adc_channel_config_rain_sensor[chnIx].resultRegister =
            (IfxVadc_ChannelResult)(chnIx);  /* use dedicated result register */

    /* initialize the channel */
    IfxVadc_Adc_initChannel(&adc_channel_rain_sensor[chnIx],
                    &adc_channel_config_rain_sensor[chnIx]);

    /* add to scan */
    unsigned channels = (1 << adc_channel_config_rain_sensor[chnIx].channelId);
    unsigned mask     = channels;
    IfxVadc_Adc_setScan(&g_rain_sensor_scan.adcGroup, channels, mask);

}


/*
 * @brief  adc 변환을 시작하는 함수
 */
void start_rain_sensor_conversion(void)
{
    /* start autoscan */
    IfxVadc_Adc_startScan(&g_rain_sensor_scan.adcGroup);
}


/*
 * @brief  센서 데이터를 읽어오는 함수
 * @return uint32: adc 결과값
 */
uint32 obtain_rain_sensor_data(void)
{
    uint32    chnIx = 2;
    Ifx_VADC_RES conversionResult; /* wait for valid result */

    /* check results */
    do
    {
        conversionResult = IfxVadc_Adc_getResult(&adc_channel_rain_sensor[chnIx]);
    } while (!conversionResult.B.VF);

    adc_result_rain_sensor[chnIx] = conversionResult.B.RESULT;
    return adc_result_rain_sensor[chnIx];
}


/*
 * @brief  디버깅을 위해 adc 글로벌 결과값 출력하는 함수
 * @return uint32: adc 글로벌 결과값
 * @note   is_raining 함수 동작 이후에 수행해야함
 */
uint32 get_rain_sensor_analog_value(void) {
    return g_rain_sensor_adc_result;
}


/*
 * @brief  현재 비가 오는지 판단하는 함수
 * @return boolean: 0 : 비 안옴, 1 : 비 옴
 * @note   2ms 이상의 주기에서 수행해야함
 */
boolean is_raining(void) {
    start_rain_sensor_conversion();
    waitTime(1); // adc 변환 시간 확보
    uint32 adcResult = obtain_rain_sensor_data();

    //디버깅을 위한 부분
    g_rain_sensor_adc_result = adcResult;

    if (adcResult > RAIN_THRESHOLD) {
        return NO_RAIN_DETECTED;
    }

    else {
        return RAIN_DETECTED;
    }
}
