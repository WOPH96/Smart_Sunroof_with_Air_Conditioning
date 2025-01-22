#******************************************************************************
# @file Cluster.py
# @brief Cluster UI and Main Logic and CAN Message Control
# @author DongHun
# @version 1.0
# @date 2025-01-19
#******************************************************************************

import sys
import os
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from can_module import CANBus
from weather_module import *
from dust_module import *
from config import *
from message_structure_c import *

os.environ["QT_QPA_PLATFORM"] = "eglfs"
os.environ["QT_QPA_EGLFS_PHYSICAL_WIDTH"] = "176"
os.environ["QT_QPA_EGLFS_PHYSICAL_HEIGHT"] = "110"
os.environ["QT_QPA_EGLFS_WIDTH"] = "1280"
os.environ["QT_QPA_EGLFS_HEIGHT"] = "800"

class VehicleStatus:
    def __init__(self):
        # 상태
        # Alive     0 : 전원 OFF, 1 : 전원 ON
        self.window_alive = 0 
        self.sunroof_alive = 0
        self.heater_alive = 0
        self.ac_alive = 0
        self.audio_alive = 0
        self.battery_alive = 0
    
        # 시동, 스마트
        self.engine_status = 0  # 0 : 기본, 1 : Utility(저전력), 2 : 시동 On
        self.smart_control = 0  # 0 : 기본(스마트 제어 모드 ON), 1 : 스마트 제어모드 OFF
        self.battery_using_status = 0 # 0 초기화, 1 메인배터리, 2 spare 배터리
        
        # status 관련
        self.window_status = 0  # 0 초기화, 시계방향 1, 반시계방향 2
        self.window_now = 0     # 0 ~ 100 까지 상태 저장
        self.sunroof_status = 0 # 0 초기화, 시계방향 1, 반시계방향 2
        self.sunroof_now = 0        # 0 ~ 100 까지 상태 저장
        self.heater_status = 0      # 0 : 미작동, 1: 약한세기, 2: 강한세기
        self.ac_status = 0          # 0 : 미작동, 1: 약한세기, 2: 강한세기
        self.audio_runnig = 0       # 0 : 미작동, 1: 음성메세지 출력중,  2: 출력 안하고 있음
        self.battery_status = 0     # 0 ~ 100 주배터리 잔량
        self.sub_battery_status = 0 # 0 ~ 100 태양광 충전한 배터리 잔량
        
        
        # 날씨 및 먼지 정보 API로 수집
        self.temperature = 0
        self.feels_like = 0
        self.humidity = 0
        self.dust_level = 0 # 값으로 저장
        self.weather = ""
    

class ClusterUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setCursor(Qt.BlankCursor)
        self.vehicle = VehicleStatus()
        self.initUI()
        self.can_bus = CANBus()
        self.can_bus.start_receiver(
            message_handler=self.handle_can_message,
            filters=MessageIDs.get_filters() # 받는 메세지 ID 정의
        )
        self.weather_thread = WeatherThread(api_key=WEATHER_API_KEY)
        self.weather_thread.weather_updated.connect(self.update_weather_ui)
        self.weather_thread.start()
        self.dust_thread = DustThread(api_key=DUST_API_KEY, location=LOCATION_CODE)
        self.dust_thread.dust_updated.connect(self.update_dust_ui)
        self.dust_thread.start()
        

    def initUI(self):
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(20, 20, 20, 20)  # 여백 설정
        main_widget.setLayout(main_layout)

        # 상단 레이아웃
        top_layout = QHBoxLayout()
       
        # 좌측 상단에 종료 버튼
        exit_btn = QPushButton('×')
        exit_btn.setFixedSize(40, 40)
        exit_btn.setStyleSheet('''
            QPushButton {
                background-color: #8B0000;
                color: white;
                border-radius: 20px;
                font-size: 20px;
                margin: 10px;
            }
            QPushButton:hover {
                background-color: #A00000;
            }
        ''')
        exit_btn.clicked.connect(QApplication.quit)

        # 상태 정보 레이아웃
        status_layout = QHBoxLayout()
        self.status_labels = {}
        status_texts = ['날씨: --', '미세먼지: --', '배터리: --', '온도: --', '체감온도: --']
       
        for text in status_texts:
            label = QLabel(text)
            label.setStyleSheet('color: white; font-size: 14px; margin-right: 20px;')
            status_layout.addWidget(label)
            self.status_labels[text.split(':')[0]] = label

        # 상단 레이아웃 구성
        top_layout.addWidget(exit_btn, alignment=Qt.AlignLeft | Qt.AlignTop)
        top_layout.addStretch()
        top_layout.addLayout(status_layout)

        # 중앙 상태 표시 영역
        center_status = QGroupBox("차량 상태")
        center_status.setStyleSheet('''
            QGroupBox {
                color: white;
                font-size: 18px;
                border: 2px solid #4F4F4F;
                border-radius: 10px;
                padding: 15px;
                margin: 20px;
            }
        ''')
       
        center_layout = QGridLayout()
        center_status.setLayout(center_layout)
       
        # 상태 라벨 ADD
        self.state_labels = {
            'engine': QLabel('시동 상태: OFF'),
            'window': QLabel('창문 상태: 0%'),
            'sunroof': QLabel('썬루프 상태: 0%'),
            'heater': QLabel('히터: OFF'),
            'ac': QLabel('에어컨: OFF'),
            'smart': QLabel('스마트 제어: ON')
        }
       
        for idx, (key, label) in enumerate(self.state_labels.items()):
            label.setStyleSheet('color: white; font-size: 16px; padding: 5px;')
            center_layout.addWidget(label, idx//3, idx%3)

        # 하단 제어 버튼 영역
        bottom_layout = QGridLayout()
        buttons_info = [
            ('engine', '시동 On/Off/Utility', 0, 0),
            ('smart', '스마트제어 On/Off', 0, 1),
            ('ac', '에어컨 On/Off', 0, 2),
            ('heater', '히터 On/Off', 0, 3),
            ('window_close', '창문 Close ▲', 1, 0),
            ('window_open', '창문 Open ▼', 1, 1),
            ('sunroof_close', '썬루프 Close ▲', 1, 2),
            ('sunroof_open', '썬루프 Open ▼', 1, 3)
        ]
       
        self.control_buttons = {}
        for key, text, row, col in buttons_info:
            btn = QPushButton(text)
            btn.setMinimumSize(150, 80)
            btn.setStyleSheet('''
                QPushButton {
                    background-color: #2F4F4F;
                    color: white;
                    border-radius: 10px;
                    font-size: 16px;
                    padding: 10px;
                }
                QPushButton:pressed {
                    background-color: #4F6F6F;
                }
            ''')
           
            if 'window' in key or 'sunroof' in key:
                btn.pressed.connect(lambda checked=False, k=key: self.send_press_message(k))
                btn.released.connect(lambda checked=False, k=key: self.send_release_message(k))
            else:
                btn.clicked.connect(lambda checked, k=key: (
                    self.handle_button_click(k),
                    btn.setEnabled(False),  # 버튼 비활성화
                    QTimer.singleShot(1000, lambda: btn.setEnabled(True))  # 1초 후 다시 활성화
                ))
            self.control_buttons[key] = btn
            bottom_layout.addWidget(btn, row, col)

        # 메인 레이아웃에 ADD
        main_layout.addLayout(top_layout)
        main_layout.addWidget(center_status)
        main_layout.addLayout(bottom_layout)

        # 윈도우 설정
        self.setWindowTitle('차량 제어 시스템')
        self.setGeometry(0, 0, 1280, 800)
        self.setWindowFlags(Qt.FramelessWindowHint)
       
        # ESC 키 설정
        self.shortcut = QShortcut(QKeySequence('Esc'), self)
        self.shortcut.activated.connect(QApplication.quit)

    def update_vehicle_status(self, can_data):
        """CAN 데이터로부터 차량 상태 업데이트"""
        if can_data.get('type') == 'engine':
            pass
            # self.vehicle.engine_status = can_data['value']
            # self.state_labels['engine'].setText(f"시동 상태: {'ON' if self.vehicle.engine_status else 'OFF'}")
            # self.control_buttons['engine'].setText(f"시동 {'끄기' if self.vehicle.engine_status else '켜기'}")

    def handle_can_message(self,msg):
        # hex_data = ' '.join([f'{b:02X}' for b in msg.data])
        # recv_msg_print(f"수신: ID = {hex(msg.arbitration_id)}, Data={hex_data}")
        # CAN 메세지 수신 처리 코드 추가
        if msg.arbitration_id == MessageIDs.MOTOR1_WINDOW:
            motor1_msg = Motor1WindowMsg.from_buffer_copy(msg.data)
            recv_msg_print(motor1_msg)
            # 내부 변수 최신화
            self.vehicle.window_alive = motor1_msg.motor1_alive
            self.vehicle.window_status = motor1_msg.motor1_running
            self.vehicle.window_now = motor1_msg.motor1_tick_counter # 실제 상태 (0 ~ 100)
            
            # CLUSTER 상태 최신화
            # 창문 상태: 0%
            self.state_labels['window'].setText(f'창문 상태: {self.vehicle.window_now}%')
            
        elif msg.arbitration_id == MessageIDs.MOTOR2_SUNROOF:
            motor2_msg = Motor2SunroofMsg.from_buffer_copy(msg.data)
            recv_msg_print(motor2_msg)
            # 내부 변수 최신화
            self.vehicle.sunroof_alive = motor2_msg.motor2_alive
            self.vehicle.sunroof_status = motor2_msg.motor2_running
            self.vehicle.sunroof_now = motor2_msg.motor2_tick_counter # 실제 상태 (0 ~ 100)
            
            # CLUSTER 상태 최신화
            # 썬루프 상태: 0%
            self.state_labels['sunroof'].setText(f'썬루프 상태: {self.vehicle.sunroof_now}%')

        elif msg.arbitration_id == MessageIDs.HEATER:
            Heater_msg = HeaterMsg.from_buffer_copy(msg.data)
            recv_msg_print(Heater_msg)
            # 내부 변수 최신화
            self.vehicle.heater_status = Heater_msg.heater_running # 0 : 미작동, 1: 약한세기, 2: 강한세기
            self.vehicle.heater_alive = Heater_msg.heater_alive
            
            # CLUSTER 상태 최신화
            # 히터: OFF
            if self.vehicle.heater_status != 0:
                self.state_labels['heater'].setText('히터: ON')
            else:
                self.state_labels['heater'].setText('히터: OFF')
                
        elif msg.arbitration_id == MessageIDs.AIRCONDITIONER:
            Aircon_msg = AirConditionerMsg.from_buffer_copy(msg.data)
            recv_msg_print(Aircon_msg)
            # 내부 변수 최신화
            self.vehicle.ac_status = Aircon_msg.AC_running # 0 : 미작동, 1: 약한세기, 2: 강한세기
            self.vehicle.ac_alive = Aircon_msg.AC_alive            
            
            # CLUSTER 상태 최신화
            # 에어컨: OFF
            if self.vehicle.ac_status != 0:
                self.state_labels['ac'].setText('에어컨: ON')
            else:
                self.state_labels['ac'].setText('에어컨: OFF')
            
        elif msg.arbitration_id == MessageIDs.AUDIO:
            Audio_msg = AudioMsg.from_buffer_copy(msg.data)
            recv_msg_print(Audio_msg)
            # 내부 변수 최신화
            self.vehicle.audio_runnig = Audio_msg.Audio_running
            self.vehicle.audio_alive = Audio_msg.Audio_alive
            
        elif msg.arbitration_id == MessageIDs.BATTERY:
            battery_msg = BatteryMsg.from_buffer_copy(msg.data)
            recv_msg_print(battery_msg)
            # 내부 변수 최신화
            self.vehicle.battery_status = battery_msg.Battery_state
            self.vehicle.sub_battery_status = battery_msg.Battery_spare_state
            self.vehicle.audio_alive = battery_msg.Battery_alive
            self.vehicle.battery_using_status = battery_msg.Battery_use
            # CLUSTER 상태 최신화
            # 배터리: --
            self.status_labels['배터리'].setText(f'배터리: {self.vehicle.battery_status}')
    
    def handle_button_click(self, button_key):
        """일반 버튼 클릭 처리"""
        debug_print(f"Button clicked: {button_key}")
        # CAN 메시지 전송 코드 추가
        # ID 파싱 필요
        
        if "engine" == button_key:
            msg = EngineModeMsg()
            if self.vehicle.engine_status == 0: # 시동 OFF
                msg.mode = 2 # 시동 ON 
                # 내부 변수 최신화
                self.vehicle.engine_status = 2
                
            elif self.vehicle.engine_status == 1: # Utility
                msg.mode = 0 # 시동 OFF
                # 내부 변수 최신화
                self.vehicle.engine_status = 0
                
            elif self.vehicle.engine_status == 2: # 시동 ON
                msg.mode = 1 # Utility 전환
                # 내부 변수 최신화
                self.vehicle.engine_status = 1
                
            # CLUSTER 상태 최신화(CLUSTER 자체적으로 최신화)  
            if self.vehicle.engine_status == 2:
                self.state_labels['engine'].setText('시동 상태: ON')
            elif self.vehicle.engine_status == 1:
                self.state_labels['engine'].setText('시동 상태: Utility')  
            else:
                self.state_labels['engine'].setText('시동 상태: OFF')  
            self.can_bus.send_message(MessageIDs.DRIVER_E, bytes(msg)) 
                
        elif "smart" == button_key:
            msg = DriverMsg()
            if self.vehicle.smart_control == 0: # SMART ON 상태
                msg.control = 1
                # 내부 변수 최신화
                self.vehicle.smart_control= 1
                
            elif self.vehicle.smart_control == 1: # SMART OFF 상태
                msg.control = 0
                # 내부 변수 최신화
                self.vehicle.smart_control= 0
            
            # CLUSTER 상태 최신화(CLUSTER 자체적으로 최신화)
            if self.vehicle.smart_control == 1:
                self.state_labels['smart'].setText('스마트 제어: OFF') 
            elif self.vehicle.smart_control == 0:
                self.state_labels['smart'].setText('스마트 제어: ON') 
            self.can_bus.send_message(MessageIDs.DRIVER_C, bytes(msg))
            
        elif "ac" == button_key:
            msg = DriverMsg()
            if self.vehicle.ac_status == False: # AC OFF 상태
                msg.control = 1
            elif self.vehicle.ac_status == True: # AC ON 상태
                msg.control = 0
            self.can_bus.send_message(MessageIDs.DRIVER_A, bytes(msg))
                
        elif "heater" == button_key:
            msg = DriverMsg()
            if self.vehicle.heater_status == False: # Heater OFF 상태
                msg.control = 1
            elif self.vehicle.heater_status == True: # Heater ON 상태 
                msg.control = 0   
            self.can_bus.send_message(MessageIDs.DRIVER_H, bytes(msg))

    def send_press_message(self, button_key):
        """버튼을 눌렀을 때 메시지 전송"""
        if 'window' in button_key:
            if 'open' in button_key:
                debug_print("Window Open Start")
                # CAN 메시지 전송 코드 추가
                msg = MotorModeMsg()
                msg.mode = 0 # 창문 열기 시작
                send_msg_print(msg)
                self.can_bus.send_message(MessageIDs.DRIVER_W, bytes(msg))
            else:
                debug_print("Window Close Start")
                # CAN 메시지 전송 코드 추가
                msg = MotorModeMsg()
                msg.mode = 2 # 창문 닫기 시작
                send_msg_print(msg)
                self.can_bus.send_message(MessageIDs.DRIVER_W, bytes(msg))
        elif 'sunroof' in button_key:
            if 'open' in button_key:
                debug_print("Sunroof Open Start")
                # CAN 메시지 전송 코드 추가
                msg = MotorModeMsg()
                msg.mode = 0 # 선루프 열기 시작
                send_msg_print(msg)
                self.can_bus.send_message(MessageIDs.DRIVER_S, bytes(msg))
            else:
                debug_print("Sunroof Close Start")
                # CAN 메시지 전송 코드 추가
                msg = MotorModeMsg()
                msg.mode = 2 # 선루프 닫기 시작
                send_msg_print(msg)
                self.can_bus.send_message(MessageIDs.DRIVER_S, bytes(msg))

    def send_release_message(self, button_key):
        """버튼을 뗐을 때 메시지 전송"""
        if 'window' in button_key:
            if 'open' in button_key:
                debug_print("Window Open Stop")
                # CAN 메시지 전송 코드 추가
                msg = MotorModeMsg()
                msg.mode = 1
                send_msg_print(msg)
                self.can_bus.send_message(MessageIDs.DRIVER_W, bytes(msg))
            else:
                debug_print("Window Close Stop")
                # CAN 메시지 전송 코드 추가
                msg = MotorModeMsg()
                msg.mode = 3
                send_msg_print(msg)
                self.can_bus.send_message(MessageIDs.DRIVER_W, bytes(msg))
        elif 'sunroof' in button_key:
            if 'open' in button_key:
                debug_print("Sunroof Open Stop")
                # CAN 메시지 전송 코드 추가
                msg = MotorModeMsg()
                msg.mode = 1
                send_msg_print(msg)
                self.can_bus.send_message(MessageIDs.DRIVER_S, bytes(msg))
            else:
                debug_print("Sunroof Close Stop")
                # CAN 메시지 전송 코드 추가
                msg = MotorModeMsg()
                msg.mode = 3
                send_msg_print(msg)
                self.can_bus.send_message(MessageIDs.DRIVER_S, bytes(msg))
                
    def update_weather_ui(self, weather_data):
        """날씨 정보를 UI에 업데이트"""
        try:
            # 날씨 데이터 추출
            temperature = weather_data['main']['temp']
            humidity = weather_data['main']['humidity']
            weather = weather_data['weather'][0]['main']
            feels_like = weather_data['main']['feels_like']
            
            # 상태 레이블 업데이트
            self.status_labels['날씨'].setText(f'날씨: {weather}')
            self.status_labels['온도'].setText(f'온도: {temperature}°C')
            self.status_labels['체감온도'].setText(f'체감온도: {feels_like}°C')
            
            # VehicleStatus 객체도 업데이트
            self.vehicle.temperature = temperature
            self.vehicle.humidity = humidity
            self.vehicle.weather = weather
            self.vehicle.feels_like = feels_like
            
            # can으로 메세지 송신
            msg = WeatherMsg()
            msg.temp = encode_temp(temperature) # + 30 ) * 10 을 해줌
            msg.real_temp = encode_temp(feels_like)
            self.can_bus.send_message(MessageIDs.WEATHER, bytes(msg))
            
        except Exception as e:
            print(f"날씨 정보 업데이트 중 오류 발생: {e}")

    def update_dust_ui(self, dust_data):
        """미세먼지 정보를 UI에 업데이트"""
        try:
            pm10 = dust_data['pm10']
            # pm25 = dust_data['pm25'] # 초미세먼지
            
            # 상태 레이블 업데이트
            self.status_labels['미세먼지'].setText(f'미세먼지: {pm10}㎍/㎥')
            
            # VehicleStatus 객체 업데이트
            self.vehicle.dust_level = pm10
            
            # can으로 메세지 송신
            msg = DustMsg()
            msg.weather_dust = self.dust_thread.get_dust_status(self.vehicle.dust_level)
            self.can_bus.send_message(MessageIDs.DUST, bytes(msg))
            
        except Exception as e:
            print(f"미세먼지 정보 업데이트 중 오류 발생: {e}")

if __name__ == '__main__':
    app = QApplication(sys.argv)
   
    # 다크 테마 설정
    palette = QPalette()
    palette.setColor(QPalette.Window, QColor(53, 53, 53))
    palette.setColor(QPalette.WindowText, Qt.white)
    palette.setColor(QPalette.Base, QColor(25, 25, 25))
    palette.setColor(QPalette.AlternateBase, QColor(53, 53, 53))
    palette.setColor(QPalette.ToolTipBase, Qt.white)
    palette.setColor(QPalette.ToolTipText, Qt.white)
    palette.setColor(QPalette.Text, Qt.white)
    palette.setColor(QPalette.Button, QColor(53, 53, 53))
    palette.setColor(QPalette.ButtonText, Qt.white)
    palette.setColor(QPalette.BrightText, Qt.red)
    palette.setColor(QPalette.Link, QColor(42, 130, 218))
    palette.setColor(QPalette.Highlight, QColor(42, 130, 218))
    palette.setColor(QPalette.HighlightedText, Qt.black)
    app.setPalette(palette)

    ex = ClusterUI()
    ex.showFullScreen()
    sys.exit(app.exec_())
   
