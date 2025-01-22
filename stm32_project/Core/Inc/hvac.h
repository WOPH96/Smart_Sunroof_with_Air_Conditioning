/*
 * hvac.h
 *
 *  Created on: Jan 22, 2025
 *      Author: USER
 */

#ifndef INC_HVAC_H_
#define INC_HVAC_H_

extern uint8_t heater_led_state;
extern uint8_t ac_led_state;
extern uint8_t pan_state;


void driver_ac(uint8_t state);
void driver_heater(uint8_t state);
void smart_ac(uint8_t state);
void smart_heater(uint8_t state);
void on_heater(void);
void off_heater(void);
void on_ac(void);
void off_ac(void);

#endif /* INC_HVAC_H_ */
