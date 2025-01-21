/*******************************************************************************
 * @file    Driver_DHT11.h
 * @brief   temperature, humidity
 * @version 1.0
 * @date    2025-01-21
 ******************************************************************************/

#ifndef INC_DHT11_DRIVER_H_
#define INC_DHT11_DRIVER_H_

#include "IfxPort.h"
#include "IfxPort_PinMap.h"
#include "IfxStm.h"


typedef struct {
    uint8 flag;
    uint8 huminityhigh;
    uint8 huminitylow;
    uint8 temperaturehigh;
    uint8 temperaturelow;
} Temp_Hum;


#define DHT IfxPort_P10_4

void DHT11_Start (void);
uint8 DHT11_Read (void);
uint8 Check_Response (void);
void delay(uint32 us);
Temp_Hum get_temp_hum(void);

#endif /* INC_DHT11_DRIVER_H_ */
