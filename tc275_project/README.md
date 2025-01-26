# 코드설명

## 시그널 정의

```c
typedef struct
{
    unsigned int Temp_Hum_alive:1;              /**< \brief [0:0] 전원 (ON/OFF) */
    unsigned int Temperature:6;                 /**< \brief [6:1] 온도 (0~50) */
    unsigned int Humiditiy:7;                   /**< \brief [13:7] 습도 (0~90) */
    unsigned int Flag:1;                        /**< \brief -> Interrupt Flag*/

}OurCanTHSensorBits;
```

메시지에 속한 시그널에 대하여, 0바이트 0비트부터 순서대로 크기에 맞게 작성한다.

끝에 1비트 Flag를 추가하여, ISR 내에서 동작할 수 있도록 한다.

## 메시지 정의

```c
// CANDB ID 정의
#define TH_SENSOR_ID 0x7FF

typedef union
{
    unsigned long long U;                         /**< \brief Unsigned access */
    signed long long I;                           /**< \brief Signed access */
    OurCanTHSensorBits B;                         /**< \brief Bitfield access */
}OurCanTHSensor;
```

메시지의 아이디를 define으로 정의하고, 시그널 정의 부분을 포함한 struct를 작성한다.

## DB 정의

```c
typedef struct
{
    OurCanTHSensor TH_sensor;
    OurCanACControl AC_control;
}DBMessages;

extern DBMessages db_msg;
```

DB에 순서와 상관없이 메시지들을 추가한다.

## 인터럽트 정의

```c
/*OurCan.c/initCan()*/
//CAN0 인터럽트 활성화
canConfig.nodePointer[TC275_CAN0].priority      = 101;
canConfig.nodePointer[TC275_CAN0].typeOfService = IfxSrc_Tos_cpu0;

canMsgObjConfig.rxInterrupt.enabled = TRUE;
canMsgObjConfig.rxInterrupt.srcId = TC275_CAN0;

/*OurCan.c*/
IFX_INTERRUPT(RX_Int0Handler, 0, 101);

/*OurCan.c/RX_Int0Handler() */
void RX_Int0Handler (void)
{
    /**/
}
```

Can 기능을 수행할 수 있게, initcan 내부에 인터럽트 활성화 부분을 추가한다.

이후, IFX_INTERRUPT를 통해 인터럽트 콜백함수를 정의하고,

Handler를 정의하여 내부에 인터럽트가 걸렸을 때 어떤 작업을 수행할지에 대한 내용을 작성한다.

현재, 모든 메시지에 대하여, Interrupt가 걸리게 설정하였음.

필터를 통해 특정 메시지만 들어오게 설정 가능

```c
/*OurCan.c/initCan()*/
// Rx 메시지 객체 초기화
canMsgObjConfig.msgObjId    = 1;  // 메시지 객체 ID
canMsgObjConfig.messageId   = CAN_RX_MESSAGE_ID;
canMsgObjConfig.acceptanceMask = 0x0; // 비교 안함, 전부 수신
canMsgObjConfig.frame       = IfxMultican_Frame_receive;
canMsgObjConfig.control.extendedFrame = FALSE;
```

messageId → 받아들이고자 하는 ID = 0x123으로 설정

acceptanceMask → 비교하고자 하는 ID = 0x0으로 설정

→ 0x7FF라면, 정확히 0x123 ID만 수신함 (인터럽트가 걸림)

## 사용 예시

```c
/*Cpu0_Main.c/core0_main()*/
initCan();
initCanDB();

Can과 CanDB 초기화 함수를 사용하여, CAN 통신을 활성화하고 DB를 초기화한다.



while(1)
{
  if(db_msg.TH_sensor.B.Flag == 1) // on message TH_sensor 같은 방식
  {
    db_msg.TH_sensor.B.Flag = 0; // 필수 처리!
    //처리 로직
  }
}
```

BUS에 원하는 메시지가 들어왔다면, ISR이 Flag를 1로 바꿔놓을 것.

그걸 감지하기 위해, if문으로 Flag를 수행한다.

내부로 들어왔다면. Flag를 0으로 바꾸어, 기존 플래그가 수행되지 않도록 한다.

## 보완 사항

1. DLC 값에 따라 송 수신 분리 작업 필요 (현재 8바이트 값 송.수신)

2. Header 추가 분리 작업 수행
