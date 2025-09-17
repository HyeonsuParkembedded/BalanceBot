# Balance Bot Controller

ESP32 기반 밸런스 봇을 BLE(Bluetooth Low Energy)로 제어하는 Flutter 앱입니다.

## 주요 기능

### 🎮 로봇 제어
- **실시간 조종**: 방향패드로 전진/후진/좌회전/우회전 제어
- **균형 제어**: 밸런스 모드 활성화/비활성화
- **기립 기능**: 넘어진 로봇을 세우는 기립 명령
- **실시간 상태 모니터링**: 현재 기울기와 속도 표시

### ⚙️ PID 제어 튜닝
- **이중 PID 구조**:
  - **Pitch PID**: 균형(기울기) 제어
  - **Velocity PID**: 속도 제어
- **실시간 조정**: 슬라이더와 텍스트 입력으로 PID 값 실시간 변경
- **설정 저장**: PID 값을 로컬에 자동 저장
- **기본값 복원**: 원터치로 기본 PID 값 복원

### 📡 BLE 통신
- **자동 기기 검색**: "Balance" 또는 "ESP32" 이름을 가진 기기 자동 검색
- **바이너리 프로토콜**: 효율적인 바이너리 통신 프로토콜 사용
- **실시간 상태 수신**: 로봇의 각도, 속도, 배터리 상태 실시간 수신
- **연결 상태 모니터링**: 연결 상태와 기기 정보 표시

## 설치 및 실행

### 1. 개발 환경 설정
```bash
flutter doctor
```

### 2. 의존성 설치
```bash
cd application
flutter pub get
```

### 3. 안드로이드 기기에서 실행
```bash
flutter run
```

**주의**: BLE 기능을 사용하므로 안드로이드 실제 기기에서만 테스트 가능합니다.

## 사용법

### 1. BLE 연결
1. 안드로이드 블루투스가 활성화된 상태에서 앱 실행
2. "연결" 버튼을 눌러 Balance Bot 검색 및 연결
3. 연결 성공 시 기기 이름과 배터리 상태 표시

### 2. 로봇 조종
- **방향패드**: 전진/후진/좌우 회전 명령
- **정지 버튼**: 모든 움직임 정지
- **균형 제어 스위치**: 균형 제어 모드 활성화/비활성화
- **기립 버튼**: 넘어진 로봇을 세우는 기립 동작

### 3. PID 튜닝
1. **Pitch PID 탭**: 균형 제어용 PID 값 조정
   - Kp: 기울기 오차에 비례하는 제어 강도
   - Ki: 누적된 오차에 대한 제어 강도
   - Kd: 기울기 변화율에 대한 제어 강도

2. **Velocity PID 탭**: 속도 제어용 PID 값 조정
   - 목표 속도와 현재 속도 차이를 제어

3. **실시간 적용**: 슬라이더 조정 시 즉시 로봇에 전송
4. **수동 적용**: 텍스트 입력 후 "적용" 버튼으로 전송

## 통신 프로토콜

ESP32와의 통신은 효율적인 바이너리 프로토콜을 사용합니다:

### 메시지 구조
```
Header (8 bytes) + Payload (가변)
- Start Marker: 0xAA
- Version: 0x01
- Message Type: 0x01~0xFF
- Sequence Number
- Payload Length
- Checksum
```

### 주요 메시지 타입
- **0x01**: 이동 명령 (Move Command)
- **0x03**: 상태 응답 (Status Response)
- **0x04**: 설정 변경 (Configuration Set)

### 이동 명령 예시
```
Direction: -1 (후진), 0 (정지), 1 (전진)
Turn: -100~100 (좌회전~우회전)
Speed: 0~100
Flags: BALANCE(0x01), STANDUP(0x02)
```

## 시스템 요구사항

### 앱
- **Flutter**: 3.0+
- **Android**: 6.0+ (API level 23+)
- **BLE 지원**: 필수
- **권한**: 위치, 블루투스 권한

### ESP32 펌웨어
- **ESP-IDF**: v5.5.0
- **BLE GATT 서버**: 구현됨
- **서비스 UUID**: 0x00FF
- **특성 UUID**:
  - 명령: 0xFF01
  - 상태: 0xFF02

## 디렉토리 구조

```
application/
├── lib/
│   ├── main.dart                 # 앱 진입점
│   ├── screens/
│   │   └── control_screen.dart   # 메인 제어 화면
│   ├── services/
│   │   └── ble_service.dart      # BLE 통신 서비스
│   ├── models/
│   │   └── balance_pid_settings.dart  # PID 설정 모델
│   ├── widgets/
│   │   ├── balance_pid_widget.dart     # PID 조정 UI
│   │   ├── connection_status_widget.dart # 연결 상태 UI
│   │   ├── robot_status_widget.dart    # 로봇 상태 UI
│   │   └── robot_control_widget.dart   # 조종 UI
│   └── utils/
│       └── protocol_utils.dart   # 프로토콜 유틸리티
├── android/                     # 안드로이드 설정
└── pubspec.yaml                 # 의존성 설정
```

## 문제 해결

### BLE 연결 안됨
1. 안드로이드 블루투스가 켜져 있는지 확인
2. 위치 권한이 허용되었는지 확인
3. ESP32가 BLE 서버 모드로 실행 중인지 확인

### PID 값이 적용 안됨
1. BLE 연결 상태 확인
2. ESP32 로그에서 설정 수신 여부 확인
3. "적용" 버튼을 눌러 수동 전송

### 앱 권한 문제
안드로이드 설정에서 앱 권한 확인:
- 위치 (정확한 위치)
- 주변 기기 (블루투스)

## 개발자 정보

이 앱은 ESP32 기반 밸런스 봇의 제어를 위해 개발되었습니다.
실시간 PID 튜닝과 직관적인 조종 인터페이스를 제공합니다.