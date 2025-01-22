/*
 * LCD_def.h
 *
 *  Created on: Jan 22, 2025
 *      Author: USER
 */

#ifndef INC_LCD_DEF_H_
#define INC_LCD_DEF_H_

typedef enum
{
	WEAK=1,
	STRONG
}WindIntensity;

#define MOTOR_CONSUM			(3)

#define AC_WEAK_CONSUM 			(8)
#define HEATER_WEAK_CONSUM 		(5)
#define WINDOW_CONSUM 			(2)
#define SUNROOF_CONSUM 			(3)
#define AUDIO_CONSUM 			(4)

#define ECOBAT_CHARGING			(10)

#endif /* INC_LCD_DEF_H_ */
