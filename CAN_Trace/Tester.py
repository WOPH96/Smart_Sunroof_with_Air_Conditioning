from ctypes import *
from CAN_thread import CANBus
from Can_Tracker import *
from Message_define import * # 위의 메시지 클래스들이 정의된 파일을 messages.py로 가정

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
            # 메인 루프
            cmd = input("Enter command (w: window, s: sunroof, h: heater, a: ac, e: engine, q: quit):\n ")
            
            if cmd == 'q':
                break
            elif cmd == 'w':
                msg = DriverMsg()
                msg.control = 1
                can_bus.send_message(MessageIDs.DRIVER_W_MSG, bytes(msg))
            elif cmd == 's':
                msg = DriverMsg()
                msg.control = 1
                can_bus.send_message(MessageIDs.DRIVER_S_MSG, bytes(msg))
            elif cmd == 'h':
                msg = DriverMsg()
                msg.control = 1
                can_bus.send_message(MessageIDs.DRIVER_H_MSG, bytes(msg))
            elif cmd == 'a':
                msg = DriverMsg()
                msg.control = 1
                can_bus.send_message(MessageIDs.DRIVER_A_MSG, bytes(msg))
            elif cmd == 'e':
                msg = EngineModeMsg()
                msg.mode = 1
                can_bus.send_message(MessageIDs.DRIVER_E_MSG, bytes(msg))
            
    except KeyboardInterrupt:
        print("\nProgram terminated by user")
    finally:
        can_bus.stop_receiver()

if __name__ == "__main__":
    main()