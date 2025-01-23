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

typedef enum{
	OPEN
}SunroofState;

typedef enum{
	DIE,
	ALIVE
}AliveState;

#define MOTOR_CONSUM			(3)

#define AC_WEAK_CONSUM 			(4) // (8)
#define HEATER_WEAK_CONSUM 		(2) // (5)
#define WINDOW_CONSUM 			(2)
#define SUNROOF_CONSUM 			(3)
#define AUDIO_CONSUM 			(4)

#define ECOBAT_CHARGING			(13)

#endif /* INC_LCD_DEF_H_ */
