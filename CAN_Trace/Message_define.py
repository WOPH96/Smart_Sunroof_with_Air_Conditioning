from ctypes import *

# CAN Message ID Definitions
class MessageIDs:
    # Send Messages
    MOTOR1_WINDOW = 0x752
    MOTOR2_SUNROOF = 0x751
    HEATER = 0x754
    AIRCONDITIONER = 0x755
    AUDIO = 0x753
    BATTERY = 0x76F
    DRIVER_W_MSG = 0x722
    DRIVER_S_MSG = 0x721
    DRIVER_H_MSG = 0x724
    DRIVER_A_MSG = 0x725
    DRIVER_E_MSG = 0x726
    DRIVER_C_MSG = 0x720
    WEATHER_MSG = 0x74D
    DUST_MSG = 0x74B
    LIGHT_MSG = 0x748
    RAIN_MSG = 0x747
    DB_MSG = 0x74E
    OUT_AIR_QUALITY_MSG = 0x74A
    IN_AIRQUALITY_MSG = 0x749
    TH_SENSOR_MSG = 0x74C
    SMART_CONTROL_WINDOW_MSG = 0x732
    SMART_CONTROL_SUNROOF_MSG = 0x731
    SMART_CONTROL_HEAT_MSG = 0x734
    SMART_CONTROL_AC_MSG = 0x735
    SMART_CONTROL_AUDIO_MSG = 0x733
    SAFETY_CONTROL_WINDOW_MSG = 0x712
    SAFETY_CONTROL_SUNROOF_MSG = 0x711
    SMART_CONTROL_STATE_MSG = 0x770

    @classmethod
    def get_filters(cls):
        return [getattr(cls, attr) for attr in dir(cls) 
                if not attr.startswith('__') and 
                isinstance(getattr(cls, attr), int)]

class BaseMsg(Structure):
    _pack_ = 1

    def __repr__(self):
        return "\n".join([f"{field[0]}: {getattr(self, field[0])}" for field in self._fields_])

# 모터1 창문 메시지
class Motor1WindowMsg(BaseMsg):
    _fields_ = [
        ("motor1_running", c_uint64, 2),      # byte 0, bit 0-1
        ("motor1_alive", c_uint64, 1),        # byte 0, bit 2
        ("motor1_tick_counter", c_uint64, 16), # byte 0, bit 3-18
        ("reserved", c_uint64, 13)            # remaining bits
    ]

# 모터2 선루프 메시지
class Motor2SunroofMsg(BaseMsg):
    _fields_ = [
        ("motor2_running", c_uint64, 2),      # byte 0, bit 0-1
        ("motor2_alive", c_uint64, 1),        # byte 0, bit 2
        ("motor2_tick_counter", c_uint64, 16), # byte 0, bit 3-18
        ("reserved", c_uint64, 13)            # remaining bits
    ]

# 히터 메시지
class HeaterMsg(BaseMsg):
    _fields_ = [
        ("Heater_running", c_uint64, 2),    # byte 0, bit 0-1
        ("Heater_alive", c_uint64, 1),      # byte 0, bit 2
        ("reserved", c_uint64, 29)          # remaining bits
    ]

# 에어컨 메시지
class AirconditionerMsg(BaseMsg):
    _fields_ = [
        ("AC_running", c_uint64, 2),    # byte 0, bit 0-1
        ("AC_alive", c_uint64, 1),      # byte 0, bit 2
        ("reserved", c_uint64, 29)      # remaining bits
    ]

# 오디오 메시지
class AudioMsg(BaseMsg):
    _fields_ = [
        ("Audio_running", c_uint64, 2),    # byte 0, bit 0-1
        ("Audio_alive", c_uint64, 1),      # byte 0, bit 2
        ("reserved", c_uint64, 29)         # remaining bits
    ]

# 배터리 메시지
class BatteryMsg(BaseMsg):
    _fields_ = [
        ("Battery_state", c_uint64, 7),        # byte 0, bit 0-6
        ("Battery_spare_state", c_uint64, 7),  # byte 0, bit 7-13
        ("Battery_use", c_uint64, 2),          # byte 1, bit 6-7
        ("Battery_alive", c_uint64, 1),        # byte 2, bit 0
        ("reserved", c_uint64, 15)             # remaining bits
    ]

# 드라이버 제어 메시지
class DriverWSEMsg(BaseMsg):
    _fields_ = [
        ("control", c_uint64, 2),    # 2비트 (0-2)
        ("reserved", c_uint64, 30)   # 나머지 31비트
    ]

class DriverHACMsg(BaseMsg):
    _fields_ = [
        ("control", c_uint64, 1),    # 1비트 (0-1)
        ("reserved", c_uint64, 31)   # 나머지 31비트
    ]

# 날씨 메시지
class WeatherMsg(BaseMsg):
    _fields_ = [
        ("temp", c_uint64, 10),          # 10비트 (-30~60)
        ("real_temp", c_uint64, 10),     # 10비트 (-30~60)
        ("reserved", c_uint64, 12)       # 나머지 12비트
    ]

# 미세먼지 메시지
class DustMsg(BaseMsg):
    _fields_ = [
        ("weather_dust", c_uint64, 3),    # 3비트 (0-4)
        ("reserved", c_uint64, 29)        # 나머지 29비트
    ]

# 엔진 모드 메시지
# class EngineModeMsg(BaseMsg):
#     _fields_ = [
#         ("mode", c_uint64, 2),       # 2비트 (0-2)
#         ("reserved", c_uint64, 30)   # 나머지 30비트
#     ]

# 창문, 썬루프 메세지
# class MotorModeMsg(BaseMsg):
#     _fields_ = [
#         ("mode", c_uint64, 2),       # 2비트 (0-2)
#         ("reserved", c_uint64, 30)   # 나머지 30비트
#     ]
    




# Light Message Structure
class LightMsg(BaseMsg):
    _fields_ = [
        ("Light_pct", c_uint64, 7),
        ("Light_alive", c_uint64, 1),
        ("reserved", c_uint64, 24)  # Padding to complete the byte
    ]

# Rain Message Structure
class RainMsg(BaseMsg):
    _fields_ = [
        ("raining_status", c_uint64, 1),
        ("raining_alive", c_uint64, 1),
        ("reserved", c_uint64, 30)
    ]

# Decibel Message Structure
class DbMsg(BaseMsg):
    _fields_ = [
        ("db_outside", c_uint64, 8),
        ("db_alive", c_uint64, 1),
        ("reserved", c_uint64, 23)
    ]

# Outdoor Air Quality Message Structure
class OutAirQualityMsg(BaseMsg):
    _fields_ = [
        ("air_CO2", c_uint64, 12),
        ("air_CO", c_uint64, 10),
        ("air_NH4", c_uint64, 10),
        ("air_alch", c_uint64, 10),
        ("AQ_alive", c_uint64, 1),
        ("reserved", c_uint64, 21)
    ]

# Indoor Air Quality Message Structure
class InAirQualityMsg(BaseMsg):
    _fields_ = [
        ("AQ_alive", c_uint64, 1),
        ("air_CO2", c_uint64, 12),
        ("air_CO", c_uint64, 10),
        ("air_NH4", c_uint64, 10),
        ("air_alch", c_uint64, 10),
        ("reserved", c_uint64, 21)
    ]

# Temperature Humidity Sensor Message Structure
class THSensorMsg(BaseMsg):
    _fields_ = [
        ("Temp_Hum_alive", c_uint64, 1),
        ("Temperature", c_uint64, 6),
        ("Humidity", c_uint64, 7),
        ("reserved", c_uint64, 18)
    ]

# Smart Control Window Message Structure
class SmartControlWindowMsg(BaseMsg):
    _fields_ = [
        ("motor1_smart_state", c_uint64, 2),
        ("motor1_state", c_uint64, 7),
        ("reserved", c_uint64, 23)
    ]

# Smart Control Sunroof Message Structure
class SmartControlSunroofMsg(BaseMsg):
    _fields_ = [
        ("motor2_smart_state", c_uint64, 2),
        ("reserved", c_uint64, 30)
    ]

# Smart Control Heat Message Structure
class SmartControlHeatMsg(BaseMsg):
    _fields_ = [
        ("Heater_state", c_uint64, 2),
        ("Heater_fan_speed", c_uint64, 2),
        ("reserved", c_uint64, 28)
    ]

# Smart Control AC Message Structure
class SmartControlACMsg(BaseMsg):
    _fields_ = [
        ("Air_state", c_uint64, 2),
        ("Air_fan_speed", c_uint64, 2),
        ("reserved", c_uint64, 28)
    ]

# Smart Control Audio Message Structure
class SmartControlAudioMsg(BaseMsg):
    _fields_ = [
        ("Audio_file", c_uint64, 6),
        ("reserved", c_uint64, 26)
    ]

# Safety Control Window Message Structure
class SafetyControlWindowMsg(BaseMsg):
    _fields_ = [
        ("motor1_smart_state", c_uint64, 2),
        ("reserved", c_uint64, 30)
    ]

# Safety Control Sunroof Message Structure
class SafetyControlSunroofMsg(BaseMsg):
    _fields_ = [
        ("motor2_smart_state", c_uint64, 2),
        ("reserved", c_uint64, 30)
    ]

# Smart Control State Message Structure
class SmartControlStateMsg(BaseMsg):
    _fields_ = [
        ("motor1_smart_control", c_uint64, 1),
        ("motor2_smart_control", c_uint64, 1),
        ("heater_smart_control", c_uint64, 1),
        ("air_smart_control", c_uint64, 1),
        ("audio_smart_control", c_uint64, 1),
        ("reserved", c_uint64, 27)
    ]