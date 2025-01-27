from PyQt5.QtCore import QThread
from message_structure_c import *
import json
import time
import os

class VehicleStatusThread(QThread):
    def __init__(self, vehicle):
        super().__init__()
        self.vehicle = vehicle
        self.running = True

    def run(self):
        while self.running:
            try:
                status = {
                    'window_alive': self.vehicle.window_alive,
                    'sunroof_alive': self.vehicle.sunroof_alive,
                    'heater_alive': self.vehicle.heater_alive,
                    'ac_alive': self.vehicle.ac_alive,
                    'audio_alive': self.vehicle.audio_alive,
                    'battery_alive': self.vehicle.battery_alive,
                    'engine_status': self.vehicle.engine_status,
                    'smart_control': self.vehicle.smart_control,
                    'window_status': self.vehicle.window_status,
                    'window_now': self.vehicle.window_now,
                    'sunroof_status': self.vehicle.sunroof_status,
                    'sunroof_now': self.vehicle.sunroof_now,
                    'heater_status': self.vehicle.heater_status,
                    'ac_status': self.vehicle.ac_status,
                    'audio_runnig': self.vehicle.audio_runnig,
                    'battery_status': self.vehicle.battery_status,
                    'sub_battery_status': self.vehicle.sub_battery_status,
                    'battery_using_status': self.vehicle.battery_using_status,
                    'temperature': self.vehicle.temperature,
                    'feels_like': self.vehicle.feels_like,
                    'humidity': self.vehicle.humidity,
                    'dust_level': self.vehicle.dust_level,
                    'weather': self.vehicle.weather,
                    'motor1_smart_control': self.vehicle.motor1_smart_control,
                    'motor2_smart_control': self.vehicle.motor2_smart_control,
                    'heater_smart_control': self.vehicle.heater_smart_control, 
                    'air_smart_control': self.vehicle.air_smart_control,
                    'audio_smart_control': self.vehicle.audio_smart_control
                }
                with open('datatoout.json', 'w') as f:
                    json.dump(status, f, indent=4)
            except Exception as e:
                print(f"Status save error: {e}")
            time.sleep(1)

    def stop(self):
        self.running = False

class VehicleControlThread(QThread):

    def __init__(self, ui_cluster):
        super().__init__()
        self.ui_cluster = ui_cluster
        self.running = True
        self.prev_control = {}
        self.init_control_file()
        
    def init_control_file(self):
        initial_control = {
            'engine_status': 10,
            'smart_control': 10,
            'window_control': 10,
            'sunroof_control': 10,
            'ac_status': 10,
            'heater_status': 10
        }
        with open('datatoin.json', 'w') as f:
            json.dump(initial_control, f, indent=4)    
        
    def run(self):
        while self.running:
            try:
                if os.path.exists('datatoin.json'):
                    self.process_control_file()
            except Exception as e:
                print(f"Control file error: {e}")
            time.sleep(0.1)

    def process_control_file(self):
        try:
            with open('datatoin.json', 'r') as f:
                control = json.load(f)

            for key, value in control.items():
                if key not in self.prev_control or value != self.prev_control[key]:
                    if key == 'engine_status':
                        print(value)
                        if value != 10:
                            msg = EngineModeMsg()
                            msg.mode = value
                            
                            # 내부 변수 최신화 및 캔 송신
                            print(value)
                            self.ui_cluster.vehicle.engine_status = value
                            self.ui_cluster.engine_status.update_engine_status(value)
                            self.ui_cluster.can_bus.send_message(MessageIDs.DRIVER_E, bytes(msg))
                    
                    if key == 'ac_status':
                        if value != 10:
                            msg = DriverMsg()
                            msg.control = value
                            self.ui_cluster.can_bus.send_message(MessageIDs.DRIVER_A, bytes(msg))
                        
                    if key == 'heater_status':
                        if value != 10:
                            msg = DriverMsg()
                            msg.control = value
                            self.ui_cluster.can_bus.send_message(MessageIDs.DRIVER_H, bytes(msg))

                    if key == 'smart_control':
                        if value != 10:
                            msg = DriverMsg()
                            msg.control = value
                            self.ui_cluster.vehicle.smart_control = value
                            for widget in self.ui_cluster.status_widgets.values():
                                widget.update_smart_control(value)
                            self.ui_cluster.can_bus.send_message(MessageIDs.DRIVER_C, bytes(msg))

                    if key == 'window_control':
                        if value != 10:
                            msg = MotorModeMsg()
                            msg.mode = value 
                            self.ui_cluster.can_bus.send_message(MessageIDs.DRIVER_W, bytes(msg))            
                        
                    if key == 'sunroof_control':
                        if value != 10:
                            msg = MotorModeMsg() 
                            msg.mode = value
                            self.ui_cluster.can_bus.send_message(MessageIDs.DRIVER_S, bytes(msg))
                    
            self.prev_control = control.copy()

        except Exception as e:
            print(f"Control file processing error: {e}")
            
    def stop(self):
        self.running = False