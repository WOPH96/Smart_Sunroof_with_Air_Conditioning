/**********************************************************************************************************************
 * @file        sound_sensor.c
 * @brief       Driver for sound sensor
 * @version     1.0
 * @date        2025-01-19
 *********************************************************************************************************************/

/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
#include <sensors/sound_sensor.h>
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
SoundSensorVadcAutoScan g_sound_sensor_scan;
IfxVadc_Adc_ChannelConfig adc_channel_config_sound_sensor[8];
IfxVadc_Adc_Channel   adc_channel_sound_sensor[8];
uint32 adc_result_sound_sensor[8] = {0u,};

//디버깅을 위한 변수
uint32 g_sound_sensor_adc_result = 0;
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/


/*
 * @brief  소리감지 센서(A8, SAR0.3)초기화 함수
 */
void init_sound_sensor(void)
{
    uint32    chnIx = 3;

    /* VADC Configuration */

    /* create configuration */
    IfxVadc_Adc_Config adcConfig;
    IfxVadc_Adc_initModuleConfig(&adcConfig, &MODULE_VADC);

    /* initialize module */
    IfxVadc_Adc_initModule(&g_sound_sensor_scan.vadc, &adcConfig);

    /* create group config */
    IfxVadc_Adc_GroupConfig adcGroupConfig;
    IfxVadc_Adc_initGroupConfig(&adcGroupConfig, &g_sound_sensor_scan.vadc);

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
    IfxVadc_Adc_initGroup(&g_sound_sensor_scan.adcGroup, &adcGroupConfig);

    /*channel init*/
    IfxVadc_Adc_initChannelConfig(&adc_channel_config_sound_sensor[chnIx],
                                            &g_sound_sensor_scan.adcGroup);

    adc_channel_config_sound_sensor[chnIx].channelId      =
            (IfxVadc_ChannelId)(chnIx);
    adc_channel_config_sound_sensor[chnIx].resultRegister =
            (IfxVadc_ChannelResult)(chnIx); /* use dedicated result register */

    /* initialize the channel */
    IfxVadc_Adc_initChannel(&adc_channel_sound_sensor[chnIx],
                    &adc_channel_config_sound_sensor[chnIx]);

    /* add to scan */
    unsigned channels = (1 << adc_channel_config_sound_sensor[chnIx].channelId);
    unsigned mask     = channels;
    IfxVadc_Adc_setScan(&g_sound_sensor_scan.adcGroup, channels, mask);

}


/*
 * @brief  adc 변환을 시작하는 함수
 */
void start_sound_sensor_conversion(void)
{
    /* start autoscan */
    IfxVadc_Adc_startScan(&g_sound_sensor_scan.adcGroup);
}


/*
 * @brief  센서 데이터를 읽어오는 함수
 * @return uint32: adc 결과값
 */
uint32 obtain_sound_sensor_data(void)
{
    uint32    chnIx =3;
    Ifx_VADC_RES conversionResult; /* wait for valid result */

    /* check results */
    do
    {
        conversionResult = IfxVadc_Adc_getResult(&adc_channel_sound_sensor[chnIx]);
    } while (!conversionResult.B.VF);

    adc_result_sound_sensor[chnIx] = conversionResult.B.RESULT;
    return adc_result_sound_sensor[chnIx];
}


/*
 * @brief  디버깅을 위해 adc 글로벌 결과값 출력하는 함수
 * @return uint32: adc 글로벌 결과값
 * @note   get_decibel 함수 동작 이후에 수행해야함
 */
uint32 get_sound_sensor_analog_value(void) {
    return g_sound_sensor_adc_result;
}


/*
 * @brief  아날로그 결과값을 데시벨로 변환하는 함수
 * @param  analogValue: obtain으로 얻은 adc 결과값
 * @return float: 데시별 결과값
 * @note   정확한 데시벨 연산 불가: EK033센서에 대한 데이터 시트가 없음, 가변저항으로 데이터 증폭
 */
float convert_to_decibel(uint32 analogValue) {
    float voltage = analogValue * (5.0 / 4095.0);

    // 조용한 상황(약 40dB)에서 analog 값 850
    // 2 = log10(1.037V / REFERENCE_VOLTAGE)
    if (voltage < BASE_VOLTAGE) {
        voltage = 0;
    }
    else {
        voltage = voltage - BASE_VOLTAGE;
    }

    // 높은 dB를 얻기위한 임의의 변환식, 70dB에서 analog 값 1117
    float decibel = 20 * log10f(voltage/REFERENCE_VOLTAGE) + DB_OFFSET; // + offset 이 연산에서 최대값 93dB


    // 전압에 따른 dB 변환식
    // float decibel = 20 * log10f(voltage/referenceVoltage); // 이 연산에서 최대값 54dB

    if (decibel < MIN_DB) {
        decibel = MIN_DB;
    }
    if (decibel > MAX_DB) {
        decibel = MAX_DB;
    }

    return decibel;
}


/*
 * @brief  데시벨을 받아오는 함수
 * @return float: 데시별 결과값
 * @note   2ms 이상의 주기에서 수행해야함
 */
float get_decibel(void) {
    start_sound_sensor_conversion();
    waitTime(1); // adc 변환 시간 확보
    uint32 adc_result = obtain_sound_sensor_data();

    //디버깅을 위한 부분
    g_sound_sensor_adc_result = adc_result;
    start_sound_sensor_conversion();

    return convert_to_decibel(adc_result);
}



/*********************************************************************************************************************/
