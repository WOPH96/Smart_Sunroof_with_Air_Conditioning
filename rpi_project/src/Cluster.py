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
import pyqtgraph as pg 

from can_module import CANBus
from weather_module import *
from dust_module import *
from web_server_module import *
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
        
        # 스마트 제어 모드 상태
        self.motor1_smart_control = 1
        self.motor2_smart_control = 1
        self.heater_smart_control = 1
        self.air_smart_control = 1
        self.audio_smart_control = 1
    

class ClusterUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setCursor(Qt.BlankCursor)
        self.vehicle = VehicleStatus() # 차량 변수 생성
        self.initUI() # UI 생성
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
        # web sever json file update
        self.status_thread = VehicleStatusThread(self.vehicle)
        self.control_thread = VehicleControlThread(self)
        self.status_thread.start()
        self.control_thread.start()
        
        

    def initUI(self):
        self.setWindowTitle("Vehicle Controls")
        self.setStyleSheet("background-color: #111827;")
        self.status_labels = {} # 상태정보 레이아웃
    
        # Main widget and layout
        main_widget = QWidget()
        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(12, 12, 12, 12)
        main_layout.setSpacing(12)
        
        
        # Top info bar
        top_bar = QHBoxLayout()
        
        # Environment info
        env_widget = QWidget()
        env_widget.setStyleSheet("background-color: #1F2937; border-radius: 4px; font-size: 25px")
        env_layout = QHBoxLayout()
        env_layout.setContentsMargins(8, 8, 8, 8)
        
        env_info = [
            ("날씨:", "--"),
            ("미세먼지:", "--"),
            ("온도:", "--"),
            ("체감온도:", "--")
        ]
        # Environment info 부분에서
        
        for label, value in env_info:
            info_layout = QHBoxLayout()
            label_widget = QLabel(label)
            label_widget.setStyleSheet("color: #9CA3AF;")
            value_widget = QLabel(value)
            value_widget.setStyleSheet("color: white")
            info_layout.addWidget(label_widget)
            info_layout.addWidget(value_widget)
            env_layout.addLayout(info_layout)
            self.status_labels[label.split(':')[0]] = value_widget  # 레이블 저장
            if value != env_info[-1][1]:
                env_layout.addSpacing(20)
        
        env_widget.setLayout(env_layout)
        
        # Battery info
        battery_widget = QWidget()
        battery_widget.setStyleSheet("background-color: #1F2937; border-radius: 4px; font-size: 25px;")
        battery_layout = QHBoxLayout()
        battery_layout.setContentsMargins(8, 8, 8, 8)
        
        battery_info = [
            ("주 배터리:", "--"),
            ("보조 배터리:", "--")
        ]
        
        for label, value in battery_info:
            info_layout = QHBoxLayout()
            label_widget = QLabel(label)
            label_widget.setStyleSheet("color: #9CA3AF;")
            value_widget = QLabel(value)
            value_widget.setStyleSheet("color: white;")
            info_layout.addWidget(label_widget)
            info_layout.addWidget(value_widget)
            battery_layout.addLayout(info_layout)
            self.status_labels[label.split(':')[0]] = value_widget  # 레이블 저장
            if value != battery_info[-1][1]:
                battery_layout.addSpacing(20)
        
        battery_widget.setLayout(battery_layout)
        
        top_bar.addWidget(env_widget)
        top_bar.addWidget(battery_widget)
        
        # Main content area
        content = QHBoxLayout()
        content.setSpacing(12)
        
        # Status panel
        status_panel = QWidget()
        status_panel.setStyleSheet("background-color: #1F2937; border-radius: 4px;")
        status_layout = QVBoxLayout()
        status_layout.setContentsMargins(12, 12, 12, 12)
        status_layout.setSpacing(8)
        
        status_title = QLabel("차량 상태")
        status_title.setStyleSheet("color: #D1D5DB; font-size: 20px;")
        status_layout.addWidget(status_title)
        
        # Add status widgets
        self.engine_status = StatusWidget("시동", is_engine=True)
        status_layout.addWidget(self.engine_status)
        
        status_items = ["창문 -- %", "썬루프 -- %", "에어컨", "히터", "오디오"]
        self.status_widgets = {}
        for item in status_items:
            widget = StatusWidget(item)
            self.status_widgets[item] = widget
            status_layout.addWidget(widget)
        
        status_layout.addStretch()
        status_panel.setLayout(status_layout)
        
        # Controls panel
        controls_panel = QVBoxLayout()
        
        # Basic controls
        basic_controls = QWidget()
        basic_controls.setStyleSheet("background-color: #1F2937; border-radius: 4px;")
        basic_layout = QVBoxLayout()
        basic_layout.setContentsMargins(12, 12, 12, 12)
        basic_layout.setSpacing(8)
        
        basic_title = QLabel("기본 제어")
        basic_title.setStyleSheet("color: #D1D5DB; font-size: 20px;")
        basic_layout.addWidget(basic_title)
        
        buttons_grid = QGridLayout()
        buttons_grid.setSpacing(8)
        
        self.control_buttons = {}  # 버튼 생성 전에 추가 필요
        control_buttons_info = [
            ('engine', '시동 On/Off/Utility'),
            ('smart', '스마트제어 On/Off'),
            ('ac', '에어컨 On/Off'),
            ('heater', '히터 On/Off')
        ]

        for i, (key, text) in enumerate(control_buttons_info): # 1초간 중복으로 눌리지 않게 처리
            btn = ControlButton(text)
            btn.clicked.connect(lambda checked, k=key: (
                self.handle_button_click(k),
                btn.setEnabled(False),
                QTimer.singleShot(1000, lambda: btn.setEnabled(True))
            ))
            self.control_buttons[key] = btn
            buttons_grid.addWidget(btn, 0, i)
        
        basic_layout.addLayout(buttons_grid)
        basic_controls.setLayout(basic_layout)
        
        # Window/Sunroof controls
        open_controls = QWidget()
        open_controls.setStyleSheet("background-color: #1F2937; border-radius: 4px;")
        
        open_layout = QVBoxLayout()
        open_layout.setContentsMargins(12, 12, 12, 12)
        open_layout.setSpacing(8)
        
        open_title = QLabel("개폐 제어")
        open_title.setStyleSheet("color: #D1D5DB; font-size: 20px;")
        open_layout.addWidget(open_title)
        
        battery_graph_controls = QWidget()
        battery_graph_controls.setStyleSheet("background-color: #1F2937; border-radius: 4px;")

        battery_graph_layout = QVBoxLayout()
        battery_graph_layout.setContentsMargins(12, 12, 12, 12)
        battery_graph_layout.setSpacing(8)

        battery_graph_title = QLabel("배터리 상태")
        battery_graph_title.setStyleSheet("color: #D1D5DB; font-size: 20px;")
        battery_graph_layout.addWidget(battery_graph_title)
        
        # 그래프 위젯 추가
        self.battery_graph = pg.PlotWidget()
        self.battery_graph.setBackground('#111827')  # 그래프 배경 색상 설정
        self.battery_graph.showGrid(x=True, y=True)
        self.battery_graph.setLabel('left', 'Battery (%)', color='#D1D5DB', size='15pt')
        self.battery_graph.setLabel('bottom', 'Time (s)', color='#D1D5DB', size='15pt')
        self.battery_graph.addLegend()
        
        # 정적 데이터 배열로 그래프 초기화
        self.main_battery_data = [0,0,0,0,0,0,0,0,0,0,0]  # 정적 데이터 예제
        self.sub_battery_data = [0,0,0,0,0,0,0,0,0,0,0] 
        self.time_data = list(range(len(self.main_battery_data)))  # 시간 데이터 (초 단위)
        
        # 그래프에 데이터 표시
        self.battery_graph.plot(
            self.time_data, self.main_battery_data,
            pen=pg.mkPen(color='#10B981', width=2),
            name="주 배터리"
        )
        self.battery_graph.plot(
            self.time_data, self.sub_battery_data,
            pen=pg.mkPen(color='#3B82F6', width=2),  # 파란색 선
            name="보조 배터리"
        )
        
        open_buttons = QHBoxLayout()
        open_buttons.setSpacing(16)
        
        # Window controls
        window_layout = QVBoxLayout()
        window_layout.setSpacing(8)
        
        # Sunroof controls
        sunroof_layout = QVBoxLayout()
        sunroof_layout.setSpacing(8)
        
        open_buttons.addLayout(window_layout)
        open_buttons.addLayout(sunroof_layout)
        open_layout.addLayout(open_buttons)
        open_controls.setLayout(open_layout)
        
        controls_panel.addWidget(basic_controls)
        controls_panel.addWidget(open_controls)
        controls_panel.addWidget(battery_graph_controls)
        controls_panel.addStretch()
        
        # battery graph
        battery_graph_layout.addWidget(self.battery_graph)
        battery_graph_controls.setLayout(battery_graph_layout)
        
        # Add panels to content layout
        content.addWidget(status_panel, 1)
        content_right = QWidget()
        content_right.setLayout(controls_panel)
        content.addWidget(content_right, 3)
        
        # Add all layouts to main layout
        main_layout.addLayout(top_bar)
        main_layout.addLayout(content)
        
        main_widget.setLayout(main_layout)
        self.setCentralWidget(main_widget)  
       
        # ESC 키 설정
        self.shortcut = QShortcut(QKeySequence('Esc'), self)
        self.shortcut.activated.connect(QApplication.quit)
        
        # 마우스 Hide
        self.setCursor(Qt.BlankCursor)
        
        # 키 Hide
        window_sunroof_buttons = [
            ('window_close', '창문 Close ▲'),
            ('window_open', '창문 Open ▼'),
            ('sunroof_close', '썬루프 Close ▲'),
            ('sunroof_open', '썬루프 Open ▼')
        ]
        for btn_key, text in window_sunroof_buttons[:2]:  # Window buttons
            btn = ControlButton(text)
            btn.pressed.connect(lambda k=btn_key: self.send_press_message(k))
            btn.released.connect(lambda k=btn_key: self.send_release_message(k))
            self.control_buttons[btn_key] = btn
            window_layout.addWidget(btn)
            
        for btn_key, text in window_sunroof_buttons[2:]:  # Sunroof buttons 
            btn = ControlButton(text)
            btn.pressed.connect(lambda k=btn_key: self.send_press_message(k))
            btn.released.connect(lambda k=btn_key: self.send_release_message(k))
            self.control_buttons[btn_key] = btn
            sunroof_layout.addWidget(btn)


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
            #self.state_labels['window'].setText(f'창문 상태: {self.vehicle.window_now}%')
            self.status_widgets["창문 -- %"].update_text(f"창문 {self.vehicle.window_now}%")
            self.status_widgets["창문 -- %"].set_alive(self.vehicle.window_alive) # 전원상태
            self.status_widgets["창문 -- %"].update_health("정상")
            self.status_widgets["창문 -- %"].update_activate(self.vehicle.window_status)
            
        elif msg.arbitration_id == MessageIDs.MOTOR2_SUNROOF:
            motor2_msg = Motor2SunroofMsg.from_buffer_copy(msg.data)
            recv_msg_print(motor2_msg)
            # 내부 변수 최신화
            self.vehicle.sunroof_alive = motor2_msg.motor2_alive
            self.vehicle.sunroof_status = motor2_msg.motor2_running
            self.vehicle.sunroof_now = motor2_msg.motor2_tick_counter # 실제 상태 (0 ~ 100)
            
            # CLUSTER 상태 최신화
            # 썬루프 상태: 0%
            # self.state_labels['sunroof'].setText(f'썬루프 상태: {self.vehicle.sunroof_now}%')
            self.status_widgets["썬루프 -- %"].update_text(f"썬루프 {self.vehicle.sunroof_now}%")
            self.status_widgets["썬루프 -- %"].set_alive(self.vehicle.sunroof_alive) # 전원상태
            self.status_widgets["썬루프 -- %"].update_health("정상")
            self.status_widgets["썬루프 -- %"].update_activate(self.vehicle.sunroof_status)

        elif msg.arbitration_id == MessageIDs.HEATER:
            Heater_msg = HeaterMsg.from_buffer_copy(msg.data)
            recv_msg_print(Heater_msg)
            # 내부 변수 최신화
            self.vehicle.heater_status = Heater_msg.heater_running # 0 : 미작동, 1: 약한세기, 2: 강한세기
            self.vehicle.heater_alive = Heater_msg.heater_alive
            
            # CLUSTER 상태 최신화
            self.status_widgets["히터"].update_health("정상")
            self.status_widgets["히터"].set_alive(self.vehicle.heater_alive)
            self.status_widgets["히터"].update_activate(self.vehicle.heater_status)
                
        elif msg.arbitration_id == MessageIDs.AIRCONDITIONER:
            Aircon_msg = AirConditionerMsg.from_buffer_copy(msg.data)
            recv_msg_print(Aircon_msg)
            # 내부 변수 최신화
            self.vehicle.ac_status = Aircon_msg.AC_running # 0 : 미작동, 1: 약한세기, 2: 강한세기
            self.vehicle.ac_alive = Aircon_msg.AC_alive            
            
            # CLUSTER 상태 최신화
            self.status_widgets["에어컨"].update_health("정상")
            self.status_widgets["에어컨"].set_alive(self.vehicle.ac_alive)
            self.status_widgets["에어컨"].update_activate(self.vehicle.ac_status)
            
        elif msg.arbitration_id == MessageIDs.AUDIO:
            Audio_msg = AudioMsg.from_buffer_copy(msg.data)
            recv_msg_print(Audio_msg)
            # 내부 변수 최신화
            self.vehicle.audio_runnig = Audio_msg.Audio_running
            self.vehicle.audio_alive = Audio_msg.Audio_alive
            self.status_widgets["오디오"].set_alive(self.vehicle.audio_alive)
            self.status_widgets["오디오"].update_health("정상")
            
        elif msg.arbitration_id == MessageIDs.BATTERY:
            battery_msg = BatteryMsg.from_buffer_copy(msg.data)
            recv_msg_print(battery_msg)
            # 내부 변수 최신화
            self.vehicle.battery_status = battery_msg.Battery_state
            self.vehicle.sub_battery_status = battery_msg.Battery_spare_state
            self.vehicle.battery_alive = battery_msg.Battery_alive
            self.vehicle.battery_using_status = battery_msg.Battery_use
            # CLUSTER 상태 최신화
            self.update_battery_graph(self.vehicle.battery_status,self.vehicle.sub_battery_status)
            self.status_labels['주 배터리'].setText(f'{self.vehicle.battery_status}%')
            self.status_labels['보조 배터리'].setText(f'{self.vehicle.sub_battery_status}%')
            
            # 배터리 고갈
            if(self.vehicle.battery_status == 0 and self.vehicle.sub_battery_status == 0):
                msg = EngineModeMsg()
                msg.mode = 0
                # 내부 변수 초기화
                self.vehicle.engine_status = 0 # 시동 OFF
                self.engine_status.update_engine_status(self.vehicle.engine_status)
                self.can_bus.send_message(MessageIDs.DRIVER_E, bytes(msg)) 
        
        elif msg.arbitration_id == MessageIDs.SMART_CONTROL_STATE:
            smart_ctrl_msg = SmartCtrlStateMsg.from_buffer_copy(msg.data)
            recv_msg_print(smart_ctrl_msg)
            self.vehicle.motor1_smart_control = smart_ctrl_msg.motor1_smart_control
            self.vehicle.motor2_smart_control = smart_ctrl_msg.motor2_smart_control
            self.vehicle.heater_smart_control = smart_ctrl_msg.heater_smart_control
            self.vehicle.air_smart_control = smart_ctrl_msg.air_smart_control
            self.vehicle.audio_smart_control = smart_ctrl_msg.audio_smart_control
            
            # Update smart control status for each widget
            self.status_widgets["창문 -- %"].update_smart_control(self.vehicle.motor1_smart_control)
            self.status_widgets["썬루프 -- %"].update_smart_control(self.vehicle.motor2_smart_control)
            self.status_widgets["히터"].update_smart_control(self.vehicle.heater_smart_control)
            self.status_widgets["에어컨"].update_smart_control(self.vehicle.air_smart_control)
            self.status_widgets["오디오"].update_smart_control(self.vehicle.audio_smart_control)
            
            
    def update_battery_graph(self, main_battery_value, sub_battery_value):
        try:
            # alive가 False면 0으로 설정
            main_value = float(main_battery_value) if self.vehicle.battery_alive else 0
            sub_value = float(sub_battery_value) if self.vehicle.battery_alive else 0
            
            self.main_battery_data = self.main_battery_data[1:] + [main_value]
            self.sub_battery_data = self.sub_battery_data[1:] + [sub_value]
            
            self.battery_graph.clear()
            self.battery_graph.addLegend()
            
            self.battery_graph.plot(
                self.time_data, self.main_battery_data,
                pen=pg.mkPen(color='#10B981', width=2),
                name="주 배터리"
            )
            self.battery_graph.plot(
                self.time_data, self.sub_battery_data,
                pen=pg.mkPen(color='#3B82F6', width=2),
                name="보조 배터리"
            )
        except Exception as e:
            print(f"배터리 그래프 업데이트 중 오류 발생: {e}")
    
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
            # if self.vehicle.engine_status == 2:
            #     self.state_labels['engine'].setText('시동 상태: ON')
            # elif self.vehicle.engine_status == 1:
            #     self.state_labels['engine'].setText('시동 상태: Utility')  
            # else:
            #     self.state_labels['engine'].setText('시동 상태: OFF')  
            
            # 시동 상태 추가 (필요)
            # Update engine status widget
            
            self.engine_status.update_engine_status(self.vehicle.engine_status)
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
                    
            for widget in self.status_widgets.values():
                widget.update_smart_control(self.vehicle.smart_control)
            
            # CLUSTER 상태 최신화(CLUSTER 자체적으로 최신화)
            # if self.vehicle.smart_control == 1:
            #     self.status_labels['smart'].setText('스마트 제어: OFF') 
            # elif self.vehicle.smart_control == 0:
            #     self.status_labels['smart'].setText('스마트 제어: ON') 
            
            # SMART ON/OFF 추가 (필요)
            
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
            self.status_labels['날씨'].setText(weather)
            self.status_labels['온도'].setText(f'{temperature}°C')
            self.status_labels['체감온도'].setText(f'{feels_like}°C')
            
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
            self.status_labels['미세먼지'].setText(f'{pm10}㎍/㎥')
            
            # VehicleStatus 객체 업데이트
            self.vehicle.dust_level = pm10
            
            # can으로 메세지 송신
            msg = DustMsg()
            msg.weather_dust = self.dust_thread.get_dust_status(self.vehicle.dust_level)
            self.can_bus.send_message(MessageIDs.DUST, bytes(msg))
            
        except Exception as e:
            print(f"미세먼지 정보 업데이트 중 오류 발생: {e}")
            
    def closeEvent(self, event):
        # JSON 관련 스레드 정리
        self.status_thread.stop()
        self.control_thread.stop()
        self.status_thread.wait()
        self.control_thread.wait()
        
        # 날씨, 미세먼지 스레드 정리
        self.weather_thread.stop()
        self.dust_thread.stop()
        self.weather_thread.wait()  
        self.dust_thread.wait()

        # CAN 통신 정리
        self.can_bus.stop_receiver()
        
        event.accept()

class StatusWidget(QWidget):
    def __init__(self, name, is_engine=False):
        super().__init__()
        self.is_engine = is_engine
        self.alive = False
        self.health = "비정상" # CAN 메세지 미 수신
        self.smart_control = False
       
        outer_layout = QVBoxLayout()
        outer_layout.setContentsMargins(0, 0, 0, 0)
       
        container = QWidget()
        container.setStyleSheet("background-color: #374151; border-radius: 8px;")
       
        layout = QVBoxLayout(container)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(8)
       
       # Top row
        top_row = QHBoxLayout()
        self.name_label = QLabel(name)
        self.name_label.setStyleSheet("color: white; font-weight: bold; font-size: 30px;")
        self.status_label = QLabel("Inactive")
        self.status_label.setStyleSheet("font-size: 20px; background-color: #4B5563; color: white; padding: 4px 8px; border-radius: 4px;")
        top_row.addWidget(self.name_label)
        top_row.addStretch()
        top_row.addWidget(self.status_label)
        layout.addLayout(top_row)
       
        if not is_engine:
            bottom_row = QHBoxLayout()
            self.health_label = QLabel(self.health)
            self.health_label.setStyleSheet("color: #EF4444; font-size: 20px;")
            self.smart_control_label = QLabel("스마트제어 OFF")
            self.smart_control_label.setStyleSheet("color: #6B7280; font-size: 20px;")
            bottom_row.addWidget(self.health_label)
            bottom_row.addStretch()
            bottom_row.addWidget(self.smart_control_label)
            layout.addLayout(bottom_row)
       
        outer_layout.addWidget(container)
        self.setLayout(outer_layout)

    def set_alive(self, alive):
        self.alive = alive
        self.status_label.setText("active" if alive else "Inactive")

        self.status_label.setStyleSheet(
           f"font-size: 20px; background-color: {'#10B981' if alive else '#4B5563'}; "
           "color: white; padding: 4px 8px; border-radius: 4px;"
        )
        
    def update_activate(self, activate):
        container = self.findChild(QWidget)
        if container:
            container.setStyleSheet(f"background-color: {'#065F46' if activate else '#374151'}; border-radius: 8px;")
       
    def update_engine_status(self, status):
        # status: 0 (OFF), 1 (Utility), 2 (Active)
        status_map = {
            0: "OFF",
            1: "Utility", 
            2: "Active"
        }
        self.status_label.setText(status_map.get(status, "Unknown"))
        self.status_label.setStyleSheet(
            f"font-size: 20px; "
            f"background-color: {'#48D79A' if status == 1 else '#10B981' if status == 2 else '#4B5563'}; "
            "color: white; padding: 4px 8px; border-radius: 4px;"
        )
       
    def update_smart_control(self, is_smart_off):
        self.smart_control = is_smart_off
        self.smart_control_label.setText("스마트제어 OFF" if is_smart_off else "스마트제어 ON")
        self.smart_control_label.setStyleSheet(
                f"color: {'#6B7280' if is_smart_off else '#10B981'}; font-size: 20px;"
            )      
       
    def update_health(self, health_status):
        self.health = health_status
        self.health_label.setStyleSheet(f"color: {'#10B981' if health_status == '정상' else '#EF4444'}; font-size: 20px;")
        self.health_label.setText(health_status)   
       
    def update_text(self, new_text):
        self.name_label.setText(new_text)

class ControlButton(QPushButton):
    def __init__(self, text, parent=None):
        super().__init__(text, parent)
        self.setStyleSheet("""
            QPushButton {
                background-color: #2F4F4F;
                color: white;
                border-radius: 4px;
                padding: 8px;
                font-size: 20px;
                border: none;
            }
            QPushButton:pressed {
                background-color: #4F6F6F;
            }
        """)
        self.setMinimumHeight(40)

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
   
