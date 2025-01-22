/*
 * OurCan_signal.h
 *
 *  Created on: Jan 21, 2025
 *      Author: USER
 */

#ifndef INC_OURCAN_SIGNAL_H_
#define INC_OURCAN_SIGNAL_H_

// 모든 메시지가 4바이트 이하라고 가정해야 가능, 8바이트 메시지 쓸거면 unsigned long으로 바꿔야
// Message Name : Function

typedef struct
{
    unsigned int motor1_running : 2;       /**< \brief [1:0] Motor1 running state */
    unsigned int motor1_alive : 1;         /**< \brief [2:2] Motor1 alive signal */
    unsigned int motor1_tick_counter : 16; /**< \brief [18:3] Motor1 tick counter */
    unsigned int Flag : 1;
} OurCanMotor1WindowBits;

typedef struct
{
    unsigned int motor2_running : 2;       /**< \brief [1:0] Motor2 running state */
    unsigned int motor2_alive : 1;         /**< \brief [2:2] Motor2 alive signal */
    unsigned int motor2_tick_counter : 16; /**< \brief [18:3] Motor2 tick counter */
    unsigned int Flag : 1;
} OurCanMotor2SunroofBits;

typedef struct
{
    unsigned int Heater_running : 2; /**< \brief [1:0] Heater running state */
    unsigned int Heater_alive : 1;   /**< \brief [2:2] Heater alive signal */
    unsigned int Flag : 1;
} OurCanHeaterBits;

typedef struct
{
    unsigned int AC_running : 2; /**< \brief [1:0] AC running state */
    unsigned int AC_alive : 1;   /**< \brief [2:2] AC alive signal */
    unsigned int Flag : 1;
} OurCanACBits;

typedef struct
{
    unsigned int Audio_running : 2; /**< \brief [1:0] Audio running state */
    unsigned int Audio_alive : 1;   /**< \brief [2:2] Audio alive signal */
    unsigned int Flag : 1;
} OurCanAudioBits;

// Battery Signal Definition
typedef struct
{
    unsigned int Battery_state : 7;       /**< \brief [6:0] Battery state */
    unsigned int Battery_spare_state : 7; /**< \brief [13:7] Battery spare state */
    unsigned int Battery_use : 2;         /**< \brief [15:14] Battery use */
    unsigned int Battery_alive : 1;       /**< \brief [16:16] Battery alive signal */
    unsigned int Flag : 1;
} OurCanBatteryBits;

// Driver Control Signal Definitions
typedef struct
{
    unsigned int driver_window : 2; /**< \brief [0:0] Driver window control */
    unsigned int Flag : 1;
} OurCanDriverWindowBits;

typedef struct
{
    unsigned int driver_sunroof : 2; /**< \brief [0:0] Driver sunroof control */
    unsigned int Flag : 1;
} OurCanDriverSunroofBits;

typedef struct
{
    unsigned int driver_heater : 1; /**< \brief [0:0] Driver heater control */
    unsigned int Flag : 1;
} OurCanDriverHeaterBits;

typedef struct
{
    unsigned int driver_air : 1; /**< \brief [0:0] Driver AC control */
    unsigned int Flag : 1;
} OurCanDriverAirBits;

typedef struct
{
    unsigned int engine_mode : 2; /**< \brief [1:0] Engine mode */
    unsigned int Flag : 1;
} OurCanDriverEngineBits;

typedef struct
{
    unsigned int mode_smart : 1; /**< \brief [0:0] Smart mode control */
    unsigned int Flag : 1;
} OurCanDriverControlBits;

// Environment Sensor Signal Definitions
typedef struct
{
    unsigned int weather_temp : 7;      /**< \brief [6:0] Weather temperature */
    unsigned int weather_real_temp : 7; /**< \brief [13:7] Real temperature */
    unsigned int Flag : 1;
} OurCanWeatherBits;

typedef struct
{
    unsigned int weather_dust : 3; /**< \brief [2:0] Dust level */
    unsigned int Flag : 1;
} OurCanDustBits;

typedef struct
{
    unsigned int Light_pct : 7;   /**< \brief [6:0] Light percentage */
    unsigned int Light_alive : 1; /**< \brief [7:7] Light alive signal */
    unsigned int Flag : 1;
} OurCanLightBits;

typedef struct
{
    unsigned int raining_status : 1; /**< \brief [0:0] Rain status */
    unsigned int raining_alive : 1;  /**< \brief [1:1] Rain sensor alive */
    unsigned int Flag : 1;
} OurCanRainBits;

typedef struct
{
    unsigned int db_outside : 8; /**< \brief [7:0] Outside noise level */
    unsigned int db_alive : 1;   /**< \brief [8:8] DB sensor alive */
    unsigned int Flag : 1;
} OurCanDBBits;

// Air Quality Signal Definitions
typedef struct
{
    unsigned int air_CO2 : 12;  /**< \brief [11:0] CO2 level */
    unsigned int air_CO : 10;   /**< \brief [21:12] CO level */
    unsigned int air_NH4 : 10;  /**< \brief [31:22] NH4 level */
    unsigned int air_alch : 10; /**< \brief [41:32] Alcohol level */
    unsigned int AQ_alive : 1;  /**< \brief [42:42] Air quality alive */
    unsigned int Flag : 1;
} OurCanOutAirQualityBits;

typedef struct
{
    unsigned int AQ_alive : 1;  /**< \brief [0:0] Indoor AQ alive */
    unsigned int air_CO2 : 12;  /**< \brief [12:1] Indoor CO2 level */
    unsigned int air_CO : 10;   /**< \brief [22:13] Indoor CO level */
    unsigned int air_NH4 : 10;  /**< \brief [32:23] Indoor NH4 level */
    unsigned int air_alch : 10; /**< \brief [42:33] Indoor alcohol level */
    unsigned int Flag : 1;
} OurCanInAirQualityBits;

typedef struct
{
    unsigned int Temp_Hum_alive : 1; /**< \brief [0:0] 전원 (ON/OFF) */
    unsigned int Temperature : 6;    /**< \brief [6:1] 온도 (0~50) */
    unsigned int Humiditiy : 7;      /**< \brief [13:7] 습도 (0~90) */
    unsigned int Flag : 1;           /**< \brief [14:14]-> Interrupt Flag*/
} OurCanTHSensorBits;

// Smart Control Signal Definitions
typedef struct
{
    unsigned int motor1_smart_state : 2; /**< \brief [1:0] Smart window state */
    unsigned int motor1_state : 7;       /**< \brief [8:2] Reserved */
    unsigned int Flag : 1;
} OurCanSmartWindowBits;

typedef struct
{
    unsigned int motor2_smart_state : 2; /**< \brief [1:0] Smart sunroof state */
    unsigned int Flag : 1;
} OurCanSmartSunroofBits;

typedef struct
{
    unsigned int Heater_state : 2;     /**< \brief [1:0] Smart heater state */
    unsigned int Heater_fan_speed : 2; /**< \brief [3:2] Heater fan speed */
    unsigned int Flag : 1;
} OurCanSmartHeaterBits;

typedef struct
{
    unsigned int Air_state : 2;     /**< \brief [1:0] Smart AC state */
    unsigned int Air_fan_speed : 2; /**< \brief [3:2] AC fan speed */
    unsigned int Flag : 1;
} OurCanSmartACBits;

typedef struct
{
    unsigned int Audio_file : 6; /**< \brief [3:0] Audio file selection */
    unsigned int Flag : 1;
} OurCanSmartAudioBits;

// Safety Control Signal Definitions
typedef struct
{
    unsigned int motor1_smart_state : 2; /**< \brief [1:0] Safety window state */
    unsigned int Flag : 1;
} OurCanSafetyWindowBits;

typedef struct
{
    unsigned int motor2_smart_state : 2; /**< \brief [1:0] Safety sunroof state */
    unsigned int Flag : 1;
} OurCanSafetySunroofBits;

#endif /* INC_OURCAN_SIGNAL_H_ */
