/*******************************************************************************
 * @file    Driver_DTC11.c
 * @brief   temperature, humidity
 * @version 1.0
 * @date    2025-01-21
 ******************************************************************************/
#include "Driver_DTC11.h"

Temp_Hum get_temp_hum(void)
{
    Temp_Hum now_status;
    DHT11_Start();
    uint8 parity=0;
    now_status.flag=0;
    now_status.huminityhigh=0;
    now_status.huminitylow = 0;
    now_status.temperaturehigh = 0;
    now_status.temperaturelow = 0;

    if(Check_Response()==1)
    {
        now_status.huminityhigh = DHT11_Read();
        now_status.huminitylow = DHT11_Read();
        now_status.temperaturehigh = DHT11_Read();
        now_status.temperaturelow = DHT11_Read();
        parity = DHT11_Read();
        if(parity==(now_status.huminityhigh+now_status.huminitylow+
                now_status.temperaturehigh+now_status.temperaturelow))
            now_status.flag=1;
        else
            now_status.flag=0;
    }
    return now_status;
}

void DHT11_Start (void)
{
	IfxPort_setPinModeOutput(DHT.port, DHT.pinIndex, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
	IfxPort_setPinLow(DHT.port, DHT.pinIndex);
	delay (18000);   // wait for 18ms
	IfxPort_setPinModeInput(DHT.port, DHT.pinIndex, IfxPort_InputMode_noPullDevice);
}

uint8 Check_Response (void)
{
	uint8 Response = 0;
	uint32 startTime = MODULE_STM0.TIM0.U;
	while((IfxPort_getPinState(DHT.port, DHT.pinIndex)))
	{
	    if(MODULE_STM0.TIM0.U-startTime > 100*100)return Response;
	}//0이면 다음 단계
	startTime = MODULE_STM0.TIM0.U;
	while(!(IfxPort_getPinState(DHT.port, DHT.pinIndex)))
	{
	    if(MODULE_STM0.TIM0.U-startTime >100*100)return Response;
	}//1이면 다음 단계
	startTime = MODULE_STM0.TIM0.U;
	while((IfxPort_getPinState(DHT.port, DHT.pinIndex)))
	{
	    if(MODULE_STM0.TIM0.U-startTime > 100*100)return Response;
	}//0이면 다음 단계
	Response = 1;
	return Response;
}

uint8 DHT11_Read (void)
{
	uint8 i=0;
	uint8 j;
	for (j=0;j<8;j++)
	{
		while (!(IfxPort_getPinState(DHT.port, DHT.pinIndex)));   // wait for the pin to go high
		delay (40);   // wait for 40 us
		if (!(IfxPort_getPinState(DHT.port, DHT.pinIndex)))   // if the pin is low
		{
			i&= ~(1<<(7-j));   // write 0
		}
		else i|= (1<<(7-j));  // if the pin is high, write 1
		while ((IfxPort_getPinState(DHT.port, DHT.pinIndex)));  // wait for the pin to go low
	}
	return i;
}

void delay(uint32 us){
    uint32 startTime = MODULE_STM0.TIM0.U;
    while(MODULE_STM0.TIM0.U-startTime < us*100) ;
}


