#ifndef DRIVER_ADC
#define DRIVER_ADC

/***********************************************************************/
/*Include*/ 
/***********************************************************************/
#include "Ifx_Types.h"
#include "IfxVadc.h"
#include "IfxVadc_Adc.h"

/***********************************************************************/
/*Typedef*/ 
/***********************************************************************/
typedef struct
{
    IfxVadc_Adc vadc; /* VADC handle */
    IfxVadc_Adc_Group adcGroup;
} App_VadcAutoScan;


/***********************************************************************/
/*Define*/ 
/***********************************************************************/

/***********************************************************************/
/*External Variable*/ 
/***********************************************************************/
IFX_EXTERN App_VadcAutoScan g_VadcAutoScan;


/***********************************************************************/
/*Global Function Prototype*/ 
/***********************************************************************/
extern void init_sensor_driver(uint32 chnIx);
extern void Driver_Adc0_ConvStart(void);
extern uint32 Driver_Adc0_DataObtain(uint32 chnIx);




#endif /* DRIVER_STM */
