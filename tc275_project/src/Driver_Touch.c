/*******************************************************************************
 * @file    Driver_Touch.c
 * @brief   Diver Touch sensor
 * @version 1.0
 * @date    2025-01-20
 ******************************************************************************/

/***********************************************************************/
/*Include*/ 
/***********************************************************************/
#include "Driver_Touch.h"

/***********************************************************************/
/*Define*/ 
/***********************************************************************/

/***********************************************************************/
/*Typedef*/ 
/***********************************************************************/


/***********************************************************************/
/*Static Function Prototype*/ 
/***********************************************************************/



/***********************************************************************/
/*Variable*/ 
/***********************************************************************/


/***********************************************************************/
/*Function*/ 
/***********************************************************************/
/**
 * @brief  led init
 *
 */
void init_led(void)
{
    P10_IOCR0.U &= ~(0x1F << PCn_2_IDX);
    P10_IOCR0.U |= (0x10 << PCn_2_IDX);
}
/**
 * @brief  touch sensor gpio init
 *
 */
void init_gpio_touch(uint8 num)
{
    IfxPort_setPinMode(&MODULE_P14, num, IfxPort_Mode_inputPullUp);//input
}
/**
 * @brief  touch 센서 데이터 읽어오는 함수
 * @return touch on/off
 *
 */
IfxPort_State get_touch_condition(uint8 num)
{
    IfxPort_State TouchState=0;
    TouchState = IfxPort_getPinState(&MODULE_P14, num);
    if (TouchState == 1)
        IfxPort_setPinState(&MODULE_P10, 2, IfxPort_State_high);
    else
        IfxPort_setPinState(&MODULE_P10, 2, IfxPort_State_low);   // LED
    return TouchState;
}
