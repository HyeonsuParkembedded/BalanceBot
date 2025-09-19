# 초보자를 위한 Figma + Flutter 연동 가이드

이 가이드는 Figma 사용 경험이 적은 분을 위해 `design/` 폴더에 있는 SVG 에셋을 Figma로 임포트하고, 컴포넌트를 만들고, Flutter 프로젝트에 디자인 토큰을 반영하는 최소한의 흐름을 단계별로 안내합니다.

## 사전 준비
- Figma 계정 (https://www.figma.com) 생성
- 권장 폰트: Inter (없으면 시스템 산세리프로 대체됨)

## 1) Figma에서 새 파일 만들기
- Figma에 로그인 → 왼쪽 상단 "New File" 클릭

## 2) SVG 임포트
- 로컬의 `c:\BalanceBot\design\components` 폴더를 열어 `joystick_track.svg`, `joystick_knob.svg`, `button_primary.svg` 등을 선택합니다.
- 파일을 드래그&드롭하면 벡터 레이어로 임포트됩니다.

## 3) 레이어 정리 및 그룹화
- 임포트된 각 SVG는 Frame 또는 Group으로 들어옵니다. 이름을 읽기 쉬운 것으로 바꿉니다(예: `Joystick / Track`, `Joystick / Knob`).
- 관련 레이어들을 선택하고 오른쪽 클릭 → "Group selection" 또는 Ctrl+G로 그룹화합니다.

## 4) 컴포넌트 생성
- 재사용할 UI 블록(버튼, 카드, 조이스틱)을 그룹으로 만든 뒤 선택합니다.
- 우클릭 → "Create component"를 선택합니다.
- 생성된 컴포넌트는 왼쪽의 Assets 패널에서 확인할 수 있습니다. 드래그해서 캔버스에 재사용하세요.

## 5) Styles(색상/타이포) 등록
- 색상 저장: 도형을 선택 → 오른쪽 Fill 패널에서 색을 클릭 → 상단의 Styles(4-점) 아이콘 → "+ Create style" 클릭 → 이름(예: `Primary / 500`) 입력 → 저장.
- 텍스트 스타일: 텍스트 레이어 선택 → 우측에서 폰트/크기/두께 설정 → Styles → "+ Create style"로 저장.

## 6) 간단한 프로토타이핑
- Prototype 탭을 열고 컴포넌트(예: 버튼)을 선택한 뒤 드래그하여 다른 프레임으로 연결하면 클릭 시 전환되는 시나리오를 만들 수 있습니다.

## 7) 디자인 토큰(색상)과 Flutter 연동(간단)
- `design/design-tokens.json` 파일에 색상 값이 있습니다. Flutter 프로젝트에서 이 값을 사용하려면 `lib/utils/design_tokens.dart` 같은 파일을 만들고 토큰을 상수로 복사하세요:

```dart
// lib/utils/design_tokens.dart
import 'package:flutter/material.dart';

class DesignTokens {
  static const primary = Color(0xFF3B82F6);
  static const primary700 = Color(0xFF2563EB);
  static const success = Color(0xFF10B981);
  static const danger = Color(0xFFEF4444);
  static const bg = Color(0xFFF8FAFC);
  static const surface = Color(0xFFFFFFFF);
  static const muted = Color(0xFF64748B);
}
```

## 8) SVG 에셋을 Flutter에서 사용하기 (임시)
- `flutter_svg` 패키지를 사용하면 로컬 SVG를 바로 렌더할 수 있습니다. `pubspec.yaml`에 추가:

```yaml
dependencies:
  flutter_svg: ^2.0.0
```

그리고 코드에서:

```dart
import 'package:flutter_svg/flutter_svg.dart';

SvgPicture.asset('assets/design/components/joystick_knob.svg');
```

## 9) 검토/피드백
- Figma에서 컴포넌트를 만들어보신 뒤 어떤 부분이 불편한지 알려주세요. 제가 컴포넌트 분해나 색상 대비를 개선해 드리겠습니다.

## 문제 발생 시 체크리스트
- SVG가 흐릿하면: Figma에서 해당 레이어의 Vector network를 확인하거나, 고해상도 PNG를 요청하세요.
- 텍스트가 바뀐다면: 폰트가 설치되어 있는지 확인하세요(Inter 권장).

## 자동화(원하면 제가 대신 해드릴 수 있는 것)
- `lib/utils/design_tokens.dart` 파일을 자동으로 생성하고 PR용으로 변경 사항을 준비
- Flutter 프로젝트 `assets/`에 SVG를 복사하고 `pubspec.yaml`에 asset entry 추가

Happy designing!
