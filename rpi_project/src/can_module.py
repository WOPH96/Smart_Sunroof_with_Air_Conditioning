#******************************************************************************
# @file can_module.py
# @brief CAN Manage
# @author DongHun
# @version 1.0
# @date 2025-01-19
#******************************************************************************

import can
import threading
from PyQt5.QtCore import QThread, pyqtSignal
import atexit
import signal
import sys

class CANReceiver(QThread):
    message_received = pyqtSignal(object)

    def __init__(self, bus, filters=None):
        super().__init__()
        self._bus = bus
        self._running = True
        self._filters = filters

    def run(self):
        while self._running:
            try:
                msg = self._bus.recv(timeout=0.1)
                if msg and self._should_process_message(msg):
                    self.message_received.emit(msg)
            except Exception as e:
                print(f"메시지 수신 에러: {e}")

    def _should_process_message(self, msg):
        if not self._filters:
            return True
        return msg.arbitration_id in self._filters

    def stop(self):
        self._running = False

class CANBus:
    _instance = None
    _lock = threading.Lock()
    _bus = None
    _initialized = False

    def __new__(cls, *args, **kwargs):
        if not cls._instance:
            with cls._lock:
                if not cls._instance:
                    cls._instance = super(CANBus, cls).__new__(cls)
                    atexit.register(cls._cleanup)
                    signal.signal(signal.SIGTERM, cls._signal_handler)
                    signal.signal(signal.SIGINT, cls._signal_handler)
        return cls._instance

    def __init__(self, channel='can0', bitrate=500000):
        if not self._initialized:
            with self._lock:
                if not self._initialized:
                    try:
                        self._bus = can.interface.Bus(
                            channel=channel,
                            bustype='socketcan',
                            bitrate=bitrate
                        )
                        print("CAN 버스 초기화 성공")
                        self._initialized = True
                        self.receiver = None
                    except Exception as e:
                        print(f"CAN 버스 초기화 실패: {e}")
                        self._bus = None

    def start_receiver(self, message_handler=None, filters=None):
        """
        CAN 메시지 수신 시작
        :param message_handler: 메시지 수신시 호출될 콜백 함수
        :param filters: 수신할 CAN ID 리스트 (예: [0x123, 0x456])
        """
        if not self._bus:
            print("CAN 버스가 초기화되지 않았습니다.")
            return

        if self.receiver is None:
            self.receiver = CANReceiver(self._bus, filters)
            if message_handler:
                self.receiver.message_received.connect(message_handler)
            self.receiver.start()
            print(f"CAN 수신 시작 (필터: {[hex(x) for x in filters] if filters else '없음'})")

    def stop_receiver(self):
        """CAN 메시지 수신 중지"""
        if self.receiver:
            self.receiver.stop()
            self.receiver.wait()
            self.receiver = None
            print("CAN 수신 중지")

    def send_message(self, can_id, data):
        if not self._bus:
            print("CAN 버스가 초기화되지 않았습니다.")
            return False

        try:
            if isinstance(data, list):
                data = bytes(data)
            
            with self._lock:
                message = can.Message(
                    arbitration_id=can_id,
                    data=data,
                    is_extended_id=False
                )
                self._bus.send(message)
            return True

        except Exception as e:
            print(f"메시지 전송 실패: {e}")
            return False

    @classmethod
    def _cleanup(cls):
        if cls._instance and cls._instance.receiver:
            cls._instance.stop_receiver()
        
        if cls._bus:
            with cls._lock:
                try:
                    cls._bus.shutdown()
                    print("CAN 버스 정상 종료")
                except Exception as e:
                    print(f"CAN 버스 종료 중 오류: {e}")
                finally:
                    cls._bus = None
                    cls._initialized = False

    @classmethod
    def _signal_handler(cls, signum, frame):
        print(f"\nSignal {signum} received. 종료합니다.")
        cls._cleanup()
        sys.exit(0)
