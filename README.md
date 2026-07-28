# STM32-SmartDoorLock
STM32F411RE 기반 Key Matrix &amp; PIR 연동 Smart Door Lock

**Bare-metal(HAL 미사용) 레지스터 직접 제어로 구현한 Key Matrix + PIR 연동 스마트 도어락**

![Hardware](hardware.jpg)

---

## Overview

STM32F411RE(Nucleo-64) 보드로 만든 스마트 도어락 시스템입니다. PIR 센서가 사람을 감지하면 키패드 입력이 활성화되고, 4자리 비밀번호를 인증하면 서보모터로 문을 엽니다. HAL 라이브러리 없이 GPIO/EXTI/Timer(PWM)/I2C 레지스터를 직접 제어하는 Bare-metal 방식으로 개발했습니다.

## Features

- PIR 센서(HC-SR501) 기반 대기/활성 상태 전환 (EXTI 인터럽트)
- 4x4 매트릭스 키패드를 통한 4자리 비밀번호 입력 (상태머신 기반 스캔)
- I2C LCD 1602로 입력 상태 및 결과 실시간 표시
- 비밀번호 일치 시 서보모터 구동 + 성공 멜로디, 불일치 시 실패음
- 로그인 성공 직후 비밀번호 변경 기능
- 입력 중 취소(전체 지우기) 지원

## Tech Stack

| 항목 | 내용 |
|---|---|
| MCU | STM32F411RE (ARM Cortex-M4, Nucleo-64) |
| 개발 방식 | Bare-metal C (HAL 미사용, 레지스터 직접 제어) |
| 사용 주변장치 | GPIO, EXTI, TIM3/TIM5(PWM), I2C2, UART2 |

## System Architecture

![System Block Diagram](architecture3.png)

## Pin Map

| Peripheral | STM32 Pin | 설명 |
|---|---|---|
| Key Matrix Row 1~4 | PA6 ~ PA9 | 행 출력, 순차 스캔 |
| Key Matrix Column 1~4 | PB4 ~ PB7 | 열 입력, 내부 Pull-up |
| PIR Sensor | PD2 (EXTI2) | Rising Edge 인터럽트 |
| Servo Motor | PA0 (TIM5_CH1) | PWM 출력, 주기 20ms |
| Buzzer | PB0 (TIM3_CH3) | PWM 출력, 가청 주파수 |
| LCD SDA | PB9 (I2C2, AF9) | 데이터 |
| LCD SCL | PB10 (I2C2, AF4) | 클럭 |
| Debug UART TX/RX | PA2 / PA3 | 115200bps |

### Keypad Layout

| 행/열 | 1열 | 2열 | 3열 | 4열 |
|---|---|---|---|---|
| 1행 | 1 | 2 | 3 | A |
| 2행 | 4 | 5 | 6 | B |
| 3행 | 7 | 8 | 9 | C |
| 4행 | * | 0 | # | D |

`*` 입력 확인 · `D` 입력 취소(전체 지우기) · `#` 로그인 성공 직후 비밀번호 변경 진입

![4x4 Keypad](keypad_shape.png)

## How It Works

```text
시작 (대기 상태)
  ↓
PIR 센서 감지 (EXTI2 Rising Edge) → LCD: "Enter Password"
  ↓
키패드 입력 (최대 4자리, '*'로 표시, 'D'로 취소)
  ↓
'*' 입력 → 비밀번호 확인 (memcmp)
  ├── 일치 → 서보모터 구동 + 성공 멜로디 + LCD "Open!" + (선택) '#'로 비밀번호 변경
  └── 불일치 → 실패음 + LCD "Try Again" + 대기 상태로 복귀
```

## Troubleshooting Highlights

개발 중 마주친 대표적인 두 가지 문제와 해결 과정입니다. (자세한 내용은 [프로젝트 보고서](스마트도어락_프로젝트자료.md) 참고)

- **메타스테이블 현상**: 키 1회 입력에 값이 중복 인식되는 문제 → 블로킹 딜레이 대신 상태머신을 SET_ROW/READ_ROW로 분리해 해결
- **상태값 덮어쓰기**: 반복문 이후 코드가 조건 없이 상태를 덮어써 값이 반복 출력되는 문제 → 조건부 삼항 연산자로 상태 분기하도록 수정

## Future Improvements

- 비밀번호 Flash 저장 (재부팅 후에도 유지)
- 다중 사용자별 비밀번호 지원
- Bluetooth/Wi-Fi 연동 원격 제어·알림
- 연속 오류 시 입력 잠금 기능

## Documentation

전체 프로젝트 보고서(배경/목적, 구현 내용, 테스트 결과 등)는 [`스마트도어락_프로젝트자료.md`](스마트도어락_프로젝트자료.md)에서 확인할 수 있습니다.
