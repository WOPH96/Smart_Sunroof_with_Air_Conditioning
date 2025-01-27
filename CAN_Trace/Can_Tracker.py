import threading
import time
from collections import defaultdict
from typing import Dict, List
from CAN_thread import CANBus
from tabulate import tabulate
from Message_define import *

class CANMessageTracker:
    def __init__(self, update_interval=0.5):
        self.message_stats = defaultdict(lambda: {
            'msg_id' : 0,
            'count': 0,
            'sender': "",
            'last_signals': {}
        })
        self.lock = threading.Lock()
        self.running = True
        self.update_interval = update_interval
        self.last_display_time = 0
        threading.Thread(target=self._display_thread, daemon=True).start()

    def track_message(self, msg_id: int, sender: str, signals: Dict[str, any]):
        with self.lock:
            msg_name = self._get_message_name(msg_id)
            self.message_stats[msg_name]['msg_id'] = hex(msg_id)
            self.message_stats[msg_name]['count'] += 1
            self.message_stats[msg_name]['sender'] = sender
            self.message_stats[msg_name]['last_signals'] = signals

    def _get_message_name(self, msg_id: int) -> str:
        # Map message IDs to human-readable names
        id_to_name = {
            MessageIDs.MOTOR1_WINDOW: '모터1 창문 결과',
            MessageIDs.MOTOR2_SUNROOF: '모터2 선루프 결과',
            MessageIDs.HEATER: '히터 결과',
            MessageIDs.AIRCONDITIONER: '에어컨 결과',
            MessageIDs.AUDIO: '오디오 결과',
            MessageIDs.BATTERY: '배터리 결과',
            MessageIDs.DRIVER_W_MSG: '운전자 창문 제어',
            MessageIDs.DRIVER_S_MSG: '운전자 선루프 제어',
            MessageIDs.DRIVER_H_MSG: '운전자 히터 제어',
            MessageIDs.DRIVER_A_MSG: '운전자 에어컨 제어',
            MessageIDs.DRIVER_E_MSG: '엔진 모드',
            MessageIDs.DRIVER_C_MSG: '스마트 제어 모드',
            MessageIDs.WEATHER_MSG: '날씨',
            MessageIDs.DUST_MSG: '미세먼지',
            MessageIDs.LIGHT_MSG: '조도',
            MessageIDs.RAIN_MSG: '강수 상태',
            MessageIDs.DB_MSG: '소음',
            MessageIDs.OUT_AIR_QUALITY_MSG: '외부 공기 품질',
            MessageIDs.IN_AIRQUALITY_MSG: '실내 공기 품질',
            MessageIDs.TH_SENSOR_MSG: '온습도 센서',
            MessageIDs.SMART_CONTROL_WINDOW_MSG: '창문 스마트 제어',
            MessageIDs.SMART_CONTROL_SUNROOF_MSG: '선루프 스마트 제어',
            MessageIDs.SMART_CONTROL_HEAT_MSG: '히터 스마트 제어',
            MessageIDs.SMART_CONTROL_AC_MSG: '에어컨 스마트 제어',
            MessageIDs.SMART_CONTROL_AUDIO_MSG: '오디오 스마트 제어',
            MessageIDs.SAFETY_CONTROL_WINDOW_MSG: '창문 안전 제어',
            MessageIDs.SAFETY_CONTROL_SUNROOF_MSG: '선루프 안전 제어',
            MessageIDs.SMART_CONTROL_STATE_MSG: '스마트 제어 상태'
        }
        return id_to_name.get(msg_id, f'{"Unknown ID ",hex(msg_id)}')

    def _display_thread(self):
        while self.running:
            current_time = time.time()
            with self.lock:
                if current_time - self.last_display_time >= self.update_interval:
                    self._display_stats()
                    self.last_display_time = current_time
            time.sleep(self.update_interval)

    def _display_stats(self):
        table_data = []
        max_signals = max((len(stats['last_signals']) for stats in self.message_stats.values()), default=0)
    
        headers = ['Message', 'ID', 'Receive Count','Sender Node'] + [f'Signal {i+1}' for i in range(max_signals)]
        
        for msg_name, stats in self.message_stats.items():
            row = [msg_name,stats['msg_id'], stats['count'], stats['sender']]
            for signal, value in stats['last_signals'].items():
                row.append(f"{signal}: {value}")
            
            # 빈 시그널 칸 채우기
            while len(row) < len(headers):
                row.append('-')
            
            table_data.append(row)
        
        print("\n"*30 + "=" * 50)
        # print("CAN 메시지 실시간 추적")
        # print("=" * 50)
        print(tabulate(table_data, headers=headers, tablefmt='grid'))

    def stop(self):
        self.running = False

def handle_can_message(tracker, msg_id: int, data: bytes):
    signals = {}
    
    # Motor and Actuator Messages
    if msg_id == MessageIDs.MOTOR1_WINDOW:
        msg = Motor1WindowMsg.from_buffer_copy(data)
        sender = "STM32"
        signals = {
            'running': msg.motor1_running,
            'alive': msg.motor1_alive,
            'tick_counter': msg.motor1_tick_counter
        }
    
    elif msg_id == MessageIDs.MOTOR2_SUNROOF:
        msg = Motor2SunroofMsg.from_buffer_copy(data)
        sender = "STM32"
        signals = {
            'running': msg.motor2_running,
            'alive': msg.motor2_alive,
            'tick_counter': msg.motor2_tick_counter
        }
    
    elif msg_id == MessageIDs.HEATER:
        msg = HeaterMsg.from_buffer_copy(data)
        sender = "STM32"
        signals = {
            'running': msg.Heater_running,
            'alive': msg.Heater_alive
        }
    
    elif msg_id == MessageIDs.AIRCONDITIONER:
        msg = AirconditionerMsg.from_buffer_copy(data)
        sender = "STM32"
        signals = {
            'running': msg.AC_running,
            'alive': msg.AC_alive
        }
    
    elif msg_id == MessageIDs.AUDIO:
        msg = AudioMsg.from_buffer_copy(data)
        sender = "STM32"
        signals = {
            'running': msg.Audio_running,
            'alive': msg.Audio_alive
        }
    
    elif msg_id == MessageIDs.BATTERY:
        msg = BatteryMsg.from_buffer_copy(data)
        sender = "STM32"
        signals = {
            'state': msg.Battery_state,
            'spare_state': msg.Battery_spare_state,
            'use': msg.Battery_use,
            'alive': msg.Battery_alive
        }
    
    # Driver and Engine Control Messages
    
    elif msg_id in [MessageIDs.DRIVER_W_MSG, MessageIDs.DRIVER_S_MSG,  
                    MessageIDs.DRIVER_E_MSG]:
        msg = DriverWSEMsg.from_buffer_copy(data)
        sender = "RPI"
        signals = {
            'control': msg.control
        }
    elif msg_id in [MessageIDs.DRIVER_H_MSG, MessageIDs.DRIVER_A_MSG, 
                    MessageIDs.DRIVER_C_MSG]:
        msg = DriverHACMsg.from_buffer_copy(data)
        sender = "RPI"
        signals = {
            'control': msg.control
        }
    
    # Sensor and Environmental Messages
    elif msg_id == MessageIDs.WEATHER_MSG:
        msg = WeatherMsg.from_buffer_copy(data)
        sender = "RPI"
        signals = {
            'temp': msg.temp,
            'real_temp': msg.real_temp
        }
    
    elif msg_id == MessageIDs.DUST_MSG:
        msg = DustMsg.from_buffer_copy(data)
        sender = "RPI"
        signals = {
            'dust_level': msg.weather_dust
        }
    
    elif msg_id == MessageIDs.LIGHT_MSG:
        msg = LightMsg.from_buffer_copy(data)
        sender = "OUT_TC275"
        signals = {
            'percentage': msg.Light_pct,
            'alive': msg.Light_alive
        }
    
    elif msg_id == MessageIDs.RAIN_MSG:
        msg = RainMsg.from_buffer_copy(data)
        sender = "OUT_TC275"
        signals = {
            'raining': msg.raining_status,
            'alive': msg.raining_alive
        }
    
    elif msg_id == MessageIDs.DB_MSG:
        msg = DbMsg.from_buffer_copy(data)
        sender = "OUT_TC275"
        signals = {
            'decibel_outside': msg.db_outside,
            'alive': msg.db_alive
        }
    
    elif msg_id == MessageIDs.OUT_AIR_QUALITY_MSG:
        msg = OutAirQualityMsg.from_buffer_copy(data)
        sender = "OUT_TC275"
        signals = {
            'CO2': msg.air_CO2,
            'CO': msg.air_CO,
            'NH4': msg.air_NH4,
            'alcohol': msg.air_alch,
            'alive': msg.AQ_alive
        }
    
    elif msg_id == MessageIDs.IN_AIRQUALITY_MSG:
        msg = InAirQualityMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'CO2': msg.air_CO2,
            'CO': msg.air_CO,
            'NH4': msg.air_NH4,
            'alcohol': msg.air_alch,
            'alive': msg.AQ_alive
        }
    
    elif msg_id == MessageIDs.TH_SENSOR_MSG:
        msg = THSensorMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'temperature': msg.Temperature,
            'humidity': msg.Humidity,
            'alive': msg.Temp_Hum_alive
        }
    
    # Smart Control Messages
    elif msg_id == MessageIDs.SMART_CONTROL_WINDOW_MSG:
        msg = SmartControlWindowMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'smart_state': msg.motor1_smart_state,
            'motor_state': msg.motor1_state
        }
    
    elif msg_id == MessageIDs.SMART_CONTROL_SUNROOF_MSG:
        msg = SmartControlSunroofMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'smart_state': msg.motor2_smart_state
        }
    
    elif msg_id == MessageIDs.SMART_CONTROL_HEAT_MSG:
        msg = SmartControlHeatMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'state': msg.Heater_state,
            'fan_speed': msg.Heater_fan_speed
        }
    
    elif msg_id == MessageIDs.SMART_CONTROL_AC_MSG:
        msg = SmartControlACMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'state': msg.Air_state,
            'fan_speed': msg.Air_fan_speed
        }
    
    elif msg_id == MessageIDs.SMART_CONTROL_AUDIO_MSG:
        msg = SmartControlAudioMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'audio_file': msg.Audio_file
        }
    
    # Safety Control Messages
    elif msg_id == MessageIDs.SAFETY_CONTROL_WINDOW_MSG:
        msg = SafetyControlWindowMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'smart_state': msg.motor1_smart_state
        }
    
    elif msg_id == MessageIDs.SAFETY_CONTROL_SUNROOF_MSG:
        msg = SafetyControlSunroofMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'smart_state': msg.motor2_smart_state
        }
    
    elif msg_id == MessageIDs.SMART_CONTROL_STATE_MSG:
        msg = SmartControlStateMsg.from_buffer_copy(data)
        sender = "IN_TC275"
        signals = {
            'window_smart_control': msg.motor1_smart_control,
            'sunroof_smart_control': msg.motor2_smart_control,
            'heater_smart_control': msg.heater_smart_control,
            'ac_smart_control': msg.air_smart_control,
            'audio_smart_control': msg.audio_smart_control
        }
    
    else:
        signals = {'unknown_message_id': msg_id}
    
    tracker.track_message(msg_id, sender, signals)

def main():
    try:
        # CAN 통신 초기화
        can_bus = CANBus()
        tracker = CANMessageTracker()
        
        # 메시지 수신 핸들러
        can_bus.start_receiver(
            message_handler=lambda msg_id, data: handle_can_message(tracker, msg_id, data),
            filters=MessageIDs.get_filters()  # 받는 메시지 ID 정의
        )
        
        while True:
            pass

    except KeyboardInterrupt:
        print("\nProgram terminated by user")
    finally:
        can_bus.stop_receiver()
    


if __name__ == "__main__":
    main()