# 프로젝트 주제


# 산출물 사이트 관리

JIRA : https://hlygo23.atlassian.net/jira/software/projects/FOUR/boards/7
Confluence : https://hlygo23.atlassian.net/wiki/spaces/FourEver/embed/3702785?atl_f=PAGETREE

# 코딩 컨벤션 명명 규칙 (Naming Conventions)

## 1.1 일반 규칙

모든 식별자는 영문 알파벳, 숫자, 언더스코어(_)만 사용  
모든 식별자는 의미있고 명확한 이름 사용  
약어 사용을 최소화하고, 사용 시 팀 내 공통 약어 목록 관리  

## 1.2 상세 규칙

- 변수명: snake_case 사용

  ```
  uint32_t sensor_value;
  bool is_initialized;
  ```

- 상수 및 매크로: 대문자와 언더스코어 사용

  ```
  #define MAX_BUFFER_SIZE 256
  const uint8_t UART_TIMEOUT = 100;
  ```

- 함수명: 동사_목적어 형태로 작성

  ```
  void init_hardware(void);
  uint8_t read_sensor_data(void);
  ```

- 구조체/열거형: PascalCase 사용

  ```
  typedef struct {
      uint8_t data;
      uint16_t timestamp;
  } SensorData;
  ```

# 2. 코드 포맷팅

## 2.1 들여쓰기 및 중괄호

들여쓰기는 4칸 공백 사용 (탭 문자 사용 금지)  
중괄호는 아래 스타일 준수

```
if (condition)
{
    // code
}
else
{
    // code
}
```

## 2.2 줄 길이 및 줄바꿈

한 줄은 최대 80자로 제한  
긴 문장의 경우 적절한 위치에서 줄 바꿈

```
long_function_name(parameter1,
                  parameter2,
                  parameter3);
```

# 3. 안전성 및 신뢰성 규칙

## 3.1 변수 선언 및 초기화

모든 변수는 선언과 동시에 초기화  
포인터는 NULL로 초기화

```
uint8_t counter = 0;
uint8_t* ptr = NULL;
```

## 3.2 메모리 관리

동적 메모리 할당 사용 금지(사용 시 알려줄 것)  
버퍼 크기 상수는 반드시 정의하여 사용

```
#define BUFFER_SIZE 64
static uint8_t buffer[BUFFER_SIZE];
```



## 3.3 인터럽트 처리

인터럽트 서비스 루틴(ISR)은 최대한 짧게 유지  
ISR 내에서 긴 연산이나 블로킹 함수 호출 금지

```
void ISR_UART(void) {
    // 최소한의 처리만 수행
    buffer[index++] = UART_READ();
    set_process_flag();
}
```

# 4. 주석 규칙

## 4.1 파일 헤더 주석

```
/*******************************************************************************
 * @file    main.c
 * @brief   Main program body
 * @version 1.0
 * @date    2025-01-17
 ******************************************************************************/
```

## 4.2 함수 주석

```
/**
 * @brief  센서 데이터를 읽어오는 함수
 * @param  sensor_id: 센서 식별자
 * @return uint8_t: 센서 데이터 값
 * @note   최소 100ms 주기로 호출해야 함
 */
uint8_t read_sensor(uint8_t sensor_id)
```

## 4.3 인라인 주석

복잡한 로직이나 중요한 경고사항에 대해서만 작성  
명확한 코드는 주석 불필요

# 5. 기본 검토 항목

- 명명 규칙 준수 여부
- 메모리 사용량 검토
- 실시간 제약 조건 충족 여부
- 에러 처리 적절성
- 하드웨어 의존성 검토

## 안전성 검토 항목

- 인터럽트 처리 안전성
- 공유 자원 접근 보호
- 경계 조건 처리
- 타이밍 제약 조건 충족



------

약어 필요 시, 아래에 입력 부탁드립니다. (ex. 날씨정보 →  WeatherAPI → WA)









------

추가 변경 사항 필요 시 요청 부탁 드립니다.
