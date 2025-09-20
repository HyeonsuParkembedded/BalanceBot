# BalanceBot Controller

ESP32 기반 자가균형 로봇을 제어하는 Flutter 모바일 애플리케이션입니다.

## 🚀 주요 기능

- **BLE 무선 통신**: `flutter_blue_plus`를 사용한 안정적인 ESP32 Bluetooth 연결
- **실시간 제어**: 조이스틱 기반 로봇 조종
- **PID 튜닝**: 실시간 PID 파라미터 조정 
- **상태 모니터링**: 로봇의 각도, 속도, 배터리 상태 확인
- **서보 기립**: 넘어진 로봇 자동 기립 기능
- **현대적인 UI**: Material Design 3 기반 사용자 인터페이스

## 📦 사용된 주요 라이브러리

- **flutter_blue_plus**: 최신 Bluetooth Low Energy (BLE) 통신
- **shared_preferences**: 앱 설정 및 PID 값 저장
- **permission_handler**: Android 권한 관리
- **flutter_svg**: SVG 아이콘 및 그래픽 지원

## 📱 APK 빌드

### 자동 빌드 (권장)
```powershell
# PowerShell에서 실행
.\build_apks.ps1
```

### 수동 빌드
```bash
# 디버그 APK (개발/테스트용)
flutter build apk --debug

# 릴리즈 APK (배포용)
flutter build apk --release
```

빌드된 APK 파일은 다음 위치에 저장됩니다:
- **자동 빌드**: `apks/` 폴더에 타임스탬프와 함께 저장
- **수동 빌드**: `build/app/outputs/flutter-apk/` 폴더

## 🔧 개발 환경 설정

### 1. Flutter 설치
```bash
# Flutter 의존성 설치
flutter pub get

# 연결된 기기 확인
flutter devices
```

### 2. 디버그 실행
```bash
# 안드로이드 기기/에뮬레이터에서 실행
flutter run

# 핫 리로드로 개발
flutter run --hot
```

### 3. 요구사항
- Flutter SDK 3.0+
- Android SDK (안드로이드 빌드용)
- 안드로이드 기기의 Bluetooth 권한

## 🤖 로봇 연결

1. ESP32 BalanceBot 펌웨어가 설치된 로봇 전원 켜기
2. 앱에서 "기기 검색" 버튼 클릭
3. "BalanceBot" 기기 선택하여 연결
4. 연결 완료 후 제어 가능

## 📋 프로젝트 구조

```
lib/
├── main.dart              # 앱 진입점
├── models/                # 데이터 모델
│   ├── balance_pid_settings.dart  # PID 설정 모델
│   └── pid_settings.dart          # 기본 PID 모델
├── screens/               # 화면 UI
│   ├── control_screen.dart        # 메인 제어 화면
│   └── control_screen_v2.dart     # 향상된 제어 화면
├── services/              # 통신 서비스
│   └── ble_service.dart           # BLE 통신 (flutter_blue_plus)
├── utils/                 # 유틸리티
│   ├── design_tokens.dart         # 디자인 시스템
│   └── protocol_utils.dart        # 통신 프로토콜
└── widgets/               # 재사용 가능한 위젯
    ├── balance_pid_widget.dart    # PID 튜닝 위젯
    ├── connection_status_widget.dart  # 연결 상태
    ├── pid_control_widget.dart    # PID 제어 패널
    ├── rc_joystick_widget.dart    # 조이스틱 컨트롤
    ├── robot_control_widget.dart  # 로봇 제어
    └── robot_status_widget.dart   # 상태 표시
```

## 🛠️ 개발 참고사항

- **BLE 서비스**: `flutter_blue_plus`를 통한 ESP32와의 통신 프로토콜
- **디자인 시스템**: 일관된 UI를 위한 디자인 토큰 적용 
- **상태 관리**: Flutter 내장 StatefulWidget 사용
- **권한 관리**: Android Bluetooth 권한 자동 요청
- **아키텍처**: 깔끔한 서비스/위젯 분리 구조

## 🔧 빌드 요구사항

- **Flutter SDK**: 3.0 이상
- **Android SDK**: API 21 (Android 5.0) 이상  
- **Java**: JDK 17 (Android Gradle Plugin 호환)
- **Bluetooth**: BLE 지원 안드로이드 기기

## 🆕 최신 업데이트 (2025.09.20)

- ✅ **라이브러리 현대화**: `flutter_bluetooth_serial` → `flutter_blue_plus` 마이그레이션
- ✅ **Java 호환성**: Java 8 deprecation 경고 완전 해결
- ✅ **코드 정리**: 불필요한 third_party 의존성 제거
- ✅ **문서화**: Doxygen을 통한 완전한 코드 문서화
- ✅ **빌드 최적화**: 더 빠르고 안정적인 빌드 프로세스

## 📞 문의

프로젝트 관련 문의사항이나 버그 리포트는 GitHub Issues를 이용해 주세요.
