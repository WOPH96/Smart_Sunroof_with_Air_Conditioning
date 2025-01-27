# 프로젝트 주제


# User Guide
## 1. 시스템 개요
이 시스템은 차량의 다양한 기능을 제어하고 모니터링하는 인터페이스를 제공합니다.

## 2. 주요 기능
### 2.1 상태 모니터링

상단바에서 다음 정보를 실시간으로 확인할 수 있습니다.

현재 날씨
미세먼지 수준  
배터리 상태   
현재 온도   
체감 온도   

화면의 아래쪽에서는 주배터리와 보조배터리의 그래프를 도식화하여 볼 수 있습니다.  


### 2.2 차량 상태 표시
중앙 패널에서 다음 정보를 확인할 수 있습니다:  

- 시동 상태 (ON/OFF)  
- 창문 위치 (0-100%)  
- 썬루프 위치 (0-100%)  
- 히터 상태 (ON/OFF)  
- 에어컨 상태 (ON/OFF)  
- 스마트 제어 상태 (ON/OFF)  

### 2.3 제어 기능
<기본 제어>

시동 On/Off: 차량 시동을 켜거나 끔  
스마트제어 On/Off: 자동 제어 모드 활성화/비활성화  
에어컨: 에어컨 켜기/끄기  
히터: 히터 켜기/끄기  

창문 제어
- 창문 Close ▲: 버튼을 누르고 있는 동안 창문이 닫힘   
- 창문 Open ▼: 버튼을 누르고 있는 동안 창문이 열림   

썬루프 제어
- 썬루프 Close ▲: 버튼을 누르고 있는 동안 썬루프가 닫힘  
- 썬루프 Open ▼: 버튼을 누르고 있는 동안 썬루프가 열림  

<스마트 제어>
...

## 3. 정기적 업데이트

날씨 정보: 1시간 마다 자동 업데이트   
미세먼지 정보: 1시간 마다 자동 업데이트   

## 4. 문제 해결
시스템에 문제가 발생할 경우  
ESC 키를 눌러 프로그램을 종료   
시스템 재시작   

## 5. 시스템 요구사항
라즈베리파이 운영체제   
Python 3.12.4  
PyQt5  
pyqtgraph   
인터넷 연결 (날씨, 미세먼지 정보 수신용)  
CAN 통신 인터페이스(PCAN Driver : https://www.peak-system.com/PCAN-USB-FD.365.0.html?&L=1) 리눅스용 드라이버 다운  
