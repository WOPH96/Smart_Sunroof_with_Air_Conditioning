#ifndef DRIVER_STM
#define DRIVER_STM

/***********************************************************************/
/*Include*/ 
/***********************************************************************/
#include "Ifx_Types.h"
#include "IfxStm.h"
#include "IfxCpu_Irq.h"


/***********************************************************************/
/*Typedef*/ 
/***********************************************************************/
typedef struct
{
    uint8 u8nuScheduling1msFlag;
    uint8 u8nuScheduling10msFlag;
    uint8 u8nuScheduling100msFlag;
    uint8 u8nuScheduling1000msFlag;
    uint8 u8nuScheduling5000msFlag;
<<<<<<< HEAD
    uint8 u8nuScheduling5000ms_2Flag;
=======
>>>>>>> 32c56d5c152b3c2d7a6c445aaf0e13a9c2559f38
}SchedulingFlag;

/***********************************************************************/
/*Define*/ 
/***********************************************************************/

/***********************************************************************/
/*External Variable*/ 
/***********************************************************************/
extern SchedulingFlag stSchedulingInfo;
extern uint32 u32nuCounter1ms_2;

/***********************************************************************/
/*Global Function Prototype*/ 
/***********************************************************************/
extern void Driver_Stm_Init(void);


#endif /* DRIVER_STM */
