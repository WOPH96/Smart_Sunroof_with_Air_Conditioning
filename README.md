# 프로젝트 주제

**팀이름** : FourEver  
**구성원** : 김종욱(조장), 유혜림, 양원필, 여동훈, 이대경, 한상준  
**프로젝트 목표** : 내외부 환경 변화에 따른 선루프/창문과 공조시스템을 제어하여 쾌적한 운전 경험 제공  
**기술 스택** : CAN 통신, 리눅스, 파이썬, 날씨 API, 미세먼지 API, 서버(웹소켓)  
**기능** : 

1. 스마트 선루프/창문  
   1)차량 내부 온습도 또는 빗물감지센서를 고려하여 선루프와 창문 제어  
   2)조도를 고려하여 선루프 제어  
   3)주변 소음을 감지하여 선루프, 창문 제어  
   4)내부 공기 환기를 위한 선루프, 창문 제어  
   5)선루프와 창문 제어시 손가락 등 끼임 고려  
   6)선루프가 닫혀있을 때, 태양광 충전진행, 추후 공조시스템 제어에 전력활용  

2. 공조시스템(에어컨/히터)  
   1)내부의 온도가 높으면 에어컨 동작  
   2)내부의 온도가 낮으면 히터 동작  

3. 배터리 ECU  
   1)태양광 선루프로 배터리 충전  
   2)공조 시스템 활성화 시 배터리 감소  

4. 오디오 ECU  
   1)날씨 정보 알림  
   2)제어 동작 수행에 대한 알림 (예 : 환기를 위해 창문을 엽니다.)  

5. RPI (Cluster, GW)  
   1)사용자 입력에 따른 액추에이터 제어  
   \- CAN버스와 연결하여 사용자의 입력을 액추에이터에 반영함.    

   2)차량 실제 상태 모니터링  
   \- 차량의 상태(창문, 선루프 개방 정도 / 에어컨, 히터 동작 여부 / 스마트 제어 동작 여부 / 배터리 상태 여부)  
   \- 배터리 및 보조 배터리 실시간 모니터링
   
   3)차량 내부와 외부를 연결하는 게이트웨이  
   \- 웹서버와 연결, 날씨 API, 미세먼지 API, DB(json) 연결  
   \- 1시간 마다 업데이트 되는 날씨, 미세먼지 정보를 Cluster에 반영

**서비스흐름도** : 

- 선루프, 창문 제어 ECU / 공조ECU / 배터리 ECU / RPI (CGW)  
- STM32 : 액추에이터, 모터 제어  
- TC275(ECU) : 센싱 및 제어, 제어기 간 CAN 통신,  
- 라즈베리파이 : 사용자 입력 제어, 실제 액추에이터 동작 상태 모니터링, 날씨 API, 미세먼지 API, 서버, DB(json) 연결, 게이트웨이  

**사용 센서** : 공기질 센서, 빗물 센서, 소음감지 센서, 터치 센서, DC 모터, 팬, MP3모듈, 스피커, LCD 16x2, 라즈베리파이용 모니터

**사용기술** : 
- Jira : 프로젝트 일정 관리  
- Confluence : 산출물 관리  
- git (Github) : CAPL 소스코드 형상관리, 협업  
- Discord : 자료 공유, 프로젝트 업무 분장, 대화  



**기대효과** : 

1. **쾌적한 운전 환경 조성:** 외부 환경 변화에 자동 대응하여 탑승자에게 쾌적한 운전 환경 제공
2. **연비 절감:** 태양광 충전을 통한 연비 절감 효과 기대
3. **편리한 제어:** Cluster(HMI)를 통한 직관적이고 편리한 제어 기능 제공
4.  **맞춤형 알림:** 날씨 및 미세먼지 정보를 실시간으로 수신하여 사용자에게 맞춤형 알림 제공



# User Guide

## 1. 시스템 개요
![image](https://github.com/user-attachments/assets/4bc8bd64-cb7a-4bb7-ac43-c45efc27238c)

## 2. 주요 기능

### 2.1 상태 모니터링

상단바에서 다음 정보를 실시간으로 확인할 수 있습니다.

- 현재 날씨  
- 미세먼지 수준  
- 배터리 상태   
- 현재 온도   
- 체감 온도   

![image](https://github.com/user-attachments/assets/7cfd0ddb-8a38-477c-a6a1-8093ab9f8640)


화면의 아래쪽에서는 주배터리와 보조배터리의 그래프를 도식화하여 볼 수 있습니다.
![image](https://github.com/user-attachments/assets/abf764d9-e2b6-41fb-ace5-c082d6d5b65a)



### 2.2 차량 상태 표시

좌측 패널에서 다음 정보를 확인할 수 있습니다:  

![image](https://github.com/user-attachments/assets/0d286220-2a65-4e67-b5bb-96aab0f5aecf)


- 시동 상태 (ON/OFF)  
- 창문 위치 (0-100%)  
- 썬루프 위치 (0-100%)  
- 히터 상태 (ON/OFF)  
- 에어컨 상태 (ON/OFF)  
- 스마트 제어 상태 (ON/OFF)  



### 2.3 제어 기능

<기본 제어 - 수동조작>

![image](https://github.com/user-attachments/assets/a178fa3b-ff80-4f98-8423-9a675c92da15)


클러스터의 버튼조작을 통해 사용자가 실제 액추에이터를 수동조작할 수 있습니다.  


![image](https://github.com/user-attachments/assets/292ca343-dc4f-442b-a729-2d03aa3628a8)



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

**- 차량 외부 상태에 따른 창문 및 선루프 제어**    
![image](https://github.com/user-attachments/assets/fc9fe366-26c7-487b-9496-9f0c226080e7)


조도센서를 통해 터널 진입과 통과를 감지하고, 빗물 센서를 통해 우천을 감지합니다.  
날씨 API를 통해 외부 미세먼지 농도를 파악하고, 소음감지 센서를 통해 외부 소음을 측정해 경고음성을 출력하고, 상황에 따른 창문과 선루프제어를 DC모터를 통해 구현하였습니다.  



**- 운전자 안전을 위한 자동 환기**

![image](https://github.com/user-attachments/assets/48d2f539-7d00-4264-90c1-642c6a3b3785)


운전자의 졸음운전과 안전을 방지하기 위해서 공기질 센서를 통해 내부 공기질을 측정해 너무 높거나, 환기를 한 후 오랜 시간이 지났으면 자동으로 환기하여 운전자의 안전한 운전을 도울 수 있도록 하였습니다.  



**- 운전자 편의를 위한 에어컨/히터 제어** 

![image](https://github.com/user-attachments/assets/aac5230f-89bb-429f-aac5-f65a084789fa)


온습도 센서를 이용해 내부 온도를 측정하고, 온도에 따라 에어컨과 히터를 작동시켰습니다.
이는 DC팬과 LED를 통해 구현하였습니다. 이와 함께 열려있는 선루프를 닫았을 때, 태양광 배터리가 자동 충전되도록 구현하였습니다.



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
