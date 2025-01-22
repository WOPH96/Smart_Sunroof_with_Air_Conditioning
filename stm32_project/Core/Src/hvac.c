/*
 * hvac.c
 *
 *  Created on: Jan 22, 2025
 *      Author: USER
 */
#include "gpio.h"
#include "hvac.h"


uint8_t heater_led_state = 0;
uint8_t ac_led_state = 0;
uint8_t pan_state = 0;
uint8_t heater_power = 0;
uint8_t ac_power = 0;

void driver_ac(uint8_t state)
{
	if (state == 1)
	{
		on_ac();
	}
	else
	{
		off_ac();
	}

}

void driver_heater(uint8_t state)
{
	if (state == 1)
	{
		on_heater();

	}
	else
	{
		off_heater();
	}
}

void smart_ac(uint8_t state)
{
	if (state == 1)
	{
		off_ac();
	}
	else if(state == 2)
	{
		on_ac();
	}
}

void smart_heater(uint8_t state)
{
	if (state == 1)
	{
		off_heater();
	}
	else if(state == 2)
	{
		on_heater();
	}
}


void on_heater(void)
{
	if (heater_led_state == 0)
	{
		// 에어컨이 켜져있으면 끈다
		if (ac_led_state == 1)
		{
			ac_led_state = 0;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
		}
		// 히터킨다
		heater_led_state = 1;
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
		pan_state = 1;

	}
}
void off_heater(void)
{
	if (heater_led_state == 1)
	{
		heater_led_state = 0;
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
		pan_state = 0;
	}
}
void on_ac(void)
{
	// 에어컨이 꺼져있을때만
	if (ac_led_state == 0)
	{
		// 히터가 켜져있으면 끈다
		if (heater_led_state == 1)
		{
			heater_led_state = 0;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
		}
		// 에어컨 킨다
		ac_led_state = 1;
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
		pan_state = 1;

	}
}
void off_ac(void)
{
	// 에어컨 켜저있을때만
	if (ac_led_state == 1)
	{
		ac_led_state = 1;
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
		pan_state = 0;
	}
}


