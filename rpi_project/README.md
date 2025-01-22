# 라즈베리파이 설명

기능 : 클러스터 역할(사용자 입력 수신 및 선루프, 창문 등을 제어 + 차량 상태 데이터 모니터링)

사용 언어 : Python

사용 프레임 워크 : PyQT

사용 API : CAN, Weather API, 미세먼지 API(한국환경공단)

![image](https://github.com/user-attachments/assets/7212e9b4-225e-4a9c-88fa-f6034d2c885a)


![image](https://github.com/user-attachments/assets/35e905c6-07c1-4460-a2c4-00d338c50e5b)

---

## Cluster 구성

![image](https://github.com/user-attachments/assets/4937dbad-7d7f-4e2f-add2-b9f490068c5d)



---

## Python 파일 구성 

Cluster.py : Cluster UI 관리 및 CAN Message 송수신 시 메인 로직, 제어 관리 (MAIN)

message_structure_c.py : 메세지 구조체 및 형식 관리(C의 구조체 모듈 사용)

config.py : API 호출에 필요한 개인키와 URL , 위치 정보 관리

can_module.py : CAN 통신을 위한 버스 관리

weather_module.py : 날씨 API 호출

dust_module.py : 미세먼지 API 호출



## 코드 구조

![image](https://github.com/user-attachments/assets/b46798b5-71c0-4543-87ac-eab412b466ee)



- UI : PYQT 사용

- CAN bus 통신 : python CAN Package 사용
- 자료 구조체 : python ctypes Package 사용



- CAN 필터 사용

  message_structure_c.py → ID 기반으로 필터 리스트 생성 함수

![image](https://github.com/user-attachments/assets/dc3bfc61-2240-41fc-9246-ff316a56218b)


Cluster.py → ID Filter 생성

![image](https://github.com/user-attachments/assets/a39ba09a-16ec-4027-b9f7-19295b5a09ea)


can_module.py → CAN thread에 Filter 설정

![image](https://github.com/user-attachments/assets/a557c6ca-461e-48ae-81b8-5806202ccdff)


