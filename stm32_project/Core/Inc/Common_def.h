/*
 * Common_def.h
 *
 *  Created on: Jan 22, 2025
 *      Author: USER
 */

#ifndef INC_COMMON_DEF_H_
#define INC_COMMON_DEF_H_

typedef enum
{
	WEAK=1,
	STRONG
}WindIntensity;

typedef enum
{
	OPEN
}SunroofOpenState;

typedef enum{
	DIE,
	ALIVE
}AliveState;

typedef enum{
	ENGINE_OFF,
	UTILITY,
	DRIVING
}EngineState;

#define MOTOR_CONSUM			(3)

#define AC_WEAK_CONSUM 			(4) // (8)
#define HEATER_WEAK_CONSUM 		(2) // (5)
#define WINDOW_CONSUM 			(2)
#define SUNROOF_CONSUM 			(3)
#define AUDIO_CONSUM 			(4)

#define ECOBAT_CHARGING			(13)

#endif /* INC_COMMON_DEF_H_ */
