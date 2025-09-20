/// @file main.dart
/// @brief BalanceBot 컨트롤러 앱의 메인 진입점  
/// @details ESP32 기반 BalanceBot과 블루투스 통신을 통해 제어하는 Flutter 모바일 앱
/// @author BalanceBot Development Team
/// @date 2025
library main;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'screens/control_screen_v2.dart';
import 'utils/design_tokens.dart';

/// @brief 앱의 메인 진입점
/// @details Flutter 앱을 초기화하고 기기 방향을 설정한 후 BalanceBotApp을 실행합니다.
/// 
/// 수행하는 작업:
/// - WidgetsFlutterBinding 초기화
/// - 모든 화면 방향 허용 (세로/가로)
/// - BalanceBotApp 위젯 실행
void main() {
  WidgetsFlutterBinding.ensureInitialized();
  // 모든 방향 허용
  SystemChrome.setPreferredOrientations([
    DeviceOrientation.portraitUp,
    DeviceOrientation.portraitDown,
    DeviceOrientation.landscapeLeft,
    DeviceOrientation.landscapeRight,
  ]);
  runApp(const BalanceBotApp());
}

/// @class BalanceBotApp
/// @brief BalanceBot 컨트롤러 앱의 루트 위젯
/// @details MaterialApp을 구성하고 전체 앱의 테마와 라우팅을 관리합니다.
/// 
/// 특징:
/// - Material Design 3 사용
/// - DesignTokens를 통한 일관된 디자인 시스템
/// - 모든 방향 지원
/// - 디버그 배너 비활성화
class BalanceBotApp extends StatelessWidget {
  /// @brief BalanceBotApp 생성자
  /// @param key 위젯 키 (선택적)
  const BalanceBotApp({super.key});

  /// @brief MaterialApp UI를 빌드합니다
  /// @param context 빌드 컨텍스트
  /// @return MaterialApp 위젯
  /// 
  /// @details 앱의 전체 테마, 제목, 홈 화면을 설정합니다.
  /// DesignTokens를 사용하여 일관된 색상과 폰트를 적용합니다.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Balance Bot Controller',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: DesignTokens.primary),
        useMaterial3: true,
        fontFamily: DesignTokens.fontFamily,
      ),
      home: const ControlScreenV2(),
      debugShowCheckedModeBanner: false, // 디버그 배너 제거
    );
  }
}
