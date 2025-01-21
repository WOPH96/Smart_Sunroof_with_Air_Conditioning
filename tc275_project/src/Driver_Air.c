/*******************************************************************************
 * @file    Driver_Air.c
 * @brief   Diver air sensor
 * @version 1.0
 * @date    2025-01-20
 ******************************************************************************/

/***********************************************************************/
/*Include*/ 
/***********************************************************************/
#include "Driver_Air.h"

/***********************************************************************/
/*Define*/ 
/***********************************************************************/

/***********************************************************************/
/*Typedef*/ 
/***********************************************************************/


/***********************************************************************/
/*Static Function Prototype*/ 
/***********************************************************************/
//static void Driver_Adc0_Init(uint32 chnIx);

float CO_ppm = 0;
float Alcohol_ppm = 0;
float CO2_ppm = 0;
float NH4_ppm = 0;
float Rs = 0;
/***********************************************************************/
/*Variable*/ 
/***********************************************************************/


/***********************************************************************/
/*Function*/ 
/***********************************************************************/
/**
 * @brief  calculate ppm
 */
float calculate_ppm(float a, float b, float ratio, float setup) {
    return a * pow(ratio, b) / setup;
}

/**
 * @brief  air condition 계산 함수
 * @return Gas: Gas condition
 *
 */
Gas get_air_condition(void)
{
    Gas nowppm;
    nowppm.adc=0;
    nowppm.CO=0;
    nowppm.Alcohol=0;
    nowppm.CO2=0;
    nowppm.NH4=0;
    float CO=0;
    float Alcohol=0;
    float CO2=0;
    float NH4=0;
    uint32 airadcResult = 0;
    float AiradcResult = 0;

    airadcResult = Driver_Adc0_DataObtain(AIR_PIN);
    Driver_Adc0_ConvStart();
    nowppm.adc=airadcResult;
    AiradcResult = (float)airadcResult / 4095.0f * 5.0f;
    Rs = (5.0f / AiradcResult - 1.0f) * Rl;
    CO =  calculate_ppm(COa, COb, Rs / R0, 594);
    Alcohol =  calculate_ppm(Alcohola, Alcoholb, Rs / R0, 2);
    CO2 =  calculate_ppm(CO2a, CO2b, Rs / R0, 0.25);
    NH4 =  calculate_ppm(NH4a, NH4b, Rs / R0, 100);
    if(CO>1023)CO=1023;
    if(CO<0)CO=0;
    if(Alcohol>1023)Alcohol=1023;
    if(Alcohol<0)Alcohol=0;
    if(CO2>4095)CO2=4095;
    if(CO2<0)CO2=0;
    if(NH4>1023)NH4=1023;
    if(NH4<0)NH4=0;

    nowppm.CO = (int)CO;
    nowppm.Alcohol = (int)Alcohol;
    nowppm.CO2 = (int)CO2;
    nowppm.NH4 = (int)NH4;
    return nowppm;

    }
