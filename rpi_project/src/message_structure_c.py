#******************************************************************************
# @file message_structure_c.py
# @brief message structure and format management
# @author DongHun
# @version 1.0
# @date 2025-01-19
#******************************************************************************

from ctypes import *

# CAN 메시지 ID 정의
class MessageIDs:
    # SEND MSG ID
    DRIVER_W = 0x722  # 창문 제어
    DRIVER_S = 0x721  # 선루프 제어
    DRIVER_H = 0x724  # 히터 제어
    DRIVER_A = 0x725  # 에어컨 제어
    DRIVER_E = 0x726  # 엔진 모드
    DRIVER_C = 0x720  # 스마트 제어
    WEATHER = 0x74D   # 날씨 정보
    DUST = 0x74B      # 미세먼지
    
    # RECEIVE MSG
    MOTOR1_WINDOW = 0x752
    MOTOR2_SUNROOF = 0x751
    HEATER = 0x754
    AIRCONDITIONER = 0x755
    AUDIO = 0x753
    BATTERY = 0x76F

    @classmethod
    def get_filters(cls):
        return [getattr(cls, attr) for attr in dir(cls) 
                if not attr.startswith('__') and 
                isinstance(getattr(cls, attr), int)]

# 공통 메시지 기본 클래스
class BaseMsg(Structure):
    _pack_ = 1

    def __repr__(self):
        # 디버깅을 위한 출력 추가
        return "\n".join([f"{field[0]}: {getattr(self, field[0])}" for field in self._fields_])

# SEND MSG
# 드라이버 제어 메시지
class DriverMsg(BaseMsg):
    _fields_ = [
        ("control", c_uint64, 1),    # 1비트 (0-1)
        ("reserved", c_uint64, 31)   # 나머지 31비트
    ]

# 엔진 모드 메시지
class EngineModeMsg(BaseMsg):
    _fields_ = [
        ("mode", c_uint64, 2),       # 2비트 (0-2)
        ("reserved", c_uint64, 30)   # 나머지 30비트
    ]

# 창문, 썬루프 메세지
class MotorModeMsg(BaseMsg):
    _fields_ = [
        ("mode", c_uint64, 2),       # 2비트 (0-2)
        ("reserved", c_uint64, 30)   # 나머지 30비트
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

# RECV MSG
# 모터1 창문 메시지
class Motor1WindowMsg(BaseMsg):
    _fields_ = [
        ("motor1_running", c_uint64, 2),      # 하위 2비트
        ("motor1_alive", c_uint64, 1),        # 다음 1비트
        ("motor1_tick_counter", c_uint64, 16) # 16비트
    ]

# 모터2 선루프 제어 메시지
class Motor2SunroofMsg(BaseMsg):
    _fields_ = [
        ("motor2_running", c_uint64, 2),     # 2비트 (0-2)
        ("motor2_alive", c_uint64, 1),       # 1비트 (0-1)
        ("motor2_tick_counter", c_uint64, 16) # 16비트
    ]

# 히터 제어 메시지
class HeaterMsg(BaseMsg):
    _fields_ = [
        ("heater_running", c_uint64, 2),    # 2비트 (0-2)
        ("heater_alive", c_uint64, 1),      # 1비트 (0-1)
        ("reserved", c_uint64, 29)          # 나머지 29비트
    ]

# 에어컨 제어 메시지
class AirConditionerMsg(BaseMsg):
    _fields_ = [
        ("AC_running", c_uint64, 2),    # 2비트 (0-2)
        ("AC_alive", c_uint64, 1),      # 1비트 (0-1)
        ("reserved", c_uint64, 29)      # 나머지 29비트
    ]

# 오디오 제어 메시지
class AudioMsg(BaseMsg):
    _fields_ = [
        ("Audio_running", c_uint64, 2),    # 2비트 (0-2)
        ("Audio_alive", c_uint64, 1),      # 1비트 (0-1)
        ("reserved", c_uint64, 29)         # 나머지 29비트
    ]

# 배터리 상태 메시지
class BatteryMsg(BaseMsg):
    _fields_ = [
        ("Battery_state", c_uint64, 7),        # 7비트 LCD1
        ("Battery_spare_state", c_uint64, 7),  # 7비트 LCD2
        ("Battery_use", c_uint64, 2),          # 2비트 (0-2)
        ("Battery_alive", c_uint64, 1),        # 1비트 (0-1)
        ("reserved", c_uint64, 15)             # 나머지 15비트
    ]
    
    
# 비트 처리를 위한 도우미 함수들
def encode_temp(temp: float) -> int:
    """온도값(-30~60)을 10비트 값으로 변환"""
    return int((temp + 30) * 10) & 0x3FF  # 10비트 마스킹

def decode_temp(raw_temp: int) -> float:
    """10비트 값을 실제 온도값으로 변환"""
    return (raw_temp / 10) - 30

def debug_print(*args, **kwargs):
    if debug_print.enabled:
        print(*args, **kwargs)

def send_msg_print(*args, **kwargs):
    if send_msg_print.enabled:
        print(*args, **kwargs)
        
def recv_msg_print(*args, **kwargs):
    if recv_msg_print.enabled:
        print(*args, **kwargs)

# 전역에서 설정
debug_print.enabled = True  
send_msg_print.enabled = True
recv_msg_print.enabled = True