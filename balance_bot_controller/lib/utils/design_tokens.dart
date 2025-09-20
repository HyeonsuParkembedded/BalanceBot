/// @file design_tokens.dart
/// @brief BalanceBot 앱의 디자인 시스템 토큰 정의
/// @details design/design-tokens.json에서 자동 생성된 디자인 토큰들
/// @author BalanceBot Development Team
/// @date 2025
library design_tokens;

// Generated from design/design-tokens.json
import 'package:flutter/material.dart';

/// @class DesignTokens
/// @brief BalanceBot 앱 전체에서 사용되는 디자인 토큰 집합
/// @details 일관된 디자인을 위한 색상, 타이포그래피, 간격 등의 상수를 정의합니다.
/// 
/// 구성 요소:
/// - Colors: 브랜드 색상 및 상태 색상
/// - Typography: 폰트 패밀리 및 크기
/// - Spacing: 여백 및 간격 값
/// 
/// 사용법:
/// ```dart
/// Container(
///   color: DesignTokens.primary,
///   padding: EdgeInsets.all(DesignTokens.gutter),
///   child: Text('Hello', style: TextStyle(fontSize: DesignTokens.h1))
/// )
/// ```
class DesignTokens {
  // Colors
  /// @brief 기본 브랜드 색상 (Blue 500)
  /// @details 주요 UI 요소와 액센트에 사용되는 기본 색상
  static const Color primary = Color(0xFF3B82F6);
  
  /// @brief 진한 브랜드 색상 (Blue 700)  
  /// @details 호버 상태나 강조가 필요한 경우 사용
  static const Color primary700 = Color(0xFF2563EB);
  
  /// @brief 성공 상태 색상 (Green 500)
  /// @details 연결 성공, 완료 등 긍정적 상태 표시
  static const Color success = Color(0xFF10B981);
  
  /// @brief 위험 상태 색상 (Red 500)
  /// @details 오류, 경고, 위험 상태 표시
  static const Color danger = Color(0xFFEF4444);
  
  /// @brief 표면 색상 (White)
  /// @details 카드, 패널 등의 배경색
  static const Color surface = Color(0xFFFFFFFF);
  
  /// @brief 중성 색상 (Slate 500)
  /// @details 보조 텍스트, 비활성 요소에 사용
  static const Color muted = Color(0xFF64748B);
  
  /// @brief 배경 색상 (Slate 50)
  /// @details 앱 전체 배경색
  static const Color bg = Color(0xFFF8FAFC);

  // Typography
  /// @brief 기본 폰트 패밀리
  /// @details 앱 전체에서 사용되는 폰트 (Inter)
  static const String fontFamily = 'Inter';
  
  /// @brief 대제목 폰트 크기 (22px)
  /// @details 화면 제목, 주요 헤더에 사용
  static const double h1 = 22.0;
  
  /// @brief 중제목 폰트 크기 (18px)  
  /// @details 섹션 제목, 서브 헤더에 사용
  static const double h2 = 18.0;
  
  /// @brief 본문 폰트 크기 (14px)
  /// @details 일반 텍스트, 설명문에 사용
  static const double body = 14.0;
  
  /// @brief 소형 폰트 크기 (12px)
  /// @details 라벨, 캡션, 부가 정보에 사용
  static const double small = 12.0;

  // Spacing
  /// @brief 기본 거터 간격 (16px)
  /// @details 컨테이너 간 주요 간격, 여백에 사용
  static const double gutter = 16.0;
  
  /// @brief 기본 패딩 간격 (12px)
  /// @details 요소 내부 패딩, 작은 여백에 사용
  static const double padding = 12.0;
}
