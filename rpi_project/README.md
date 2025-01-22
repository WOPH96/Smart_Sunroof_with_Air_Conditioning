# 라즈베리파이 설명

기능 : 클러스터 역할(사용자 입력 수신 및 선루프, 창문 등을 제어 + 차량 상태 데이터 모니터링)

사용 언어 : Python

사용 프레임 워크 : PyQT

사용 API : CAN, Weather API, 미세먼지 API(한국환경공단)

![image-20250122144210591](C:\Users\USER\AppData\Roaming\Typora\typora-user-images\image-20250122144210591.png)

![image-20250122144238463](C:\Users\USER\AppData\Roaming\Typora\typora-user-images\image-20250122144238463.png)



---

## Cluster 구성

![image-20250122153753782](C:\Users\USER\AppData\Roaming\Typora\typora-user-images\image-20250122153753782.png)

---

## Python 파일 구성 

Cluster.py : Cluster UI 관리 및 CAN Message 송수신 시 메인 로직, 제어 관리 (MAIN)

message_structure_c.py : 메세지 구조체 및 형식 관리(C의 구조체 모듈 사용)

config.py : API 호출에 필요한 개인키와 URL , 위치 정보 관리

can_module.py : CAN 통신을 위한 버스 관리

weather_module.py : 날씨 API 호출

dust_module.py : 미세먼지 API 호출



## 코드 구조

![image-20250122151011592](C:\Users\USER\AppData\Roaming\Typora\typora-user-images\image-20250122151011592.png)

- UI : PYQT 사용

- CAN bus 통신 : python CAN Package 사용
- 자료 구조체 : python ctypes Package 사용



- CAN 필터 사용

  message_structure_c.py → ID 기반으로 필터 리스트 생성 함수

![image-20250122151545083](C:\Users\USER\AppData\Roaming\Typora\typora-user-images\image-20250122151545083.png)

Cluster.py → ID Filter 생성

![image-20250122151705477](C:\Users\USER\AppData\Roaming\Typora\typora-user-images\image-20250122151705477.png)

can_module.py → CAN thread에 Filter 설정

![image-20250122151727782](C:\Users\USER\AppData\Roaming\Typora\typora-user-images\image-20250122151727782.png)

