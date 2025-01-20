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
float Acetona_ppm = 0;
float Rs = 0;
/***********************************************************************/
/*Variable*/ 
/***********************************************************************/


/***********************************************************************/
/*Function*/ 
/***********************************************************************/

float calculatePPM(float a, float b, float ratio, float setup) {
    return a * pow(ratio, b) / setup;  // PPM �� ���
}


Gas Air(void)
{
    Gas nowppm;
    nowppm.CO=0;
    nowppm.Alcohol=0;
    nowppm.CO2=0;
    nowppm.NH4=0;
    uint32 airadcResult = 0;
    airadcResult = Driver_Adc0_DataObtain(Air_Pin);
    Driver_Adc0_ConvStart();
    float AiradcResult = (float)airadcResult / 4095.0f * 5.0f;
    Rs = (5.0f / AiradcResult - 1.0f) * Rl;
    nowppm.CO =  calculatePPM(COa, COb, Rs / R0, 594);
    nowppm.Alcohol =  calculatePPM(Alcohola, Alcoholb, Rs / R0, 2);
    nowppm.CO2 =  calculatePPM(CO2a, CO2b, Rs / R0, 0.25);
    nowppm.NH4 =  calculatePPM(NH4a, NH4b, Rs / R0, 100);
    return nowppm;
}
