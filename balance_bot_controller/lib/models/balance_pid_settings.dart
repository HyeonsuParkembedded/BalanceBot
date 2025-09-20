/// @file balance_pid_settings.dart
/// @brief BalanceBot PID 제어 파라미터 설정 모델
/// @details 로봇의 균형 제어와 속도 제어를 위한 PID 파라미터를 관리
/// @author BalanceBot Development Team
/// @date 2025
library balance_pid_settings;

import 'package:shared_preferences/shared_preferences.dart';

/// @class BalancePidSettings
/// @brief BalanceBot의 PID 제어 파라미터를 관리하는 데이터 모델
/// @details 두 개의 독립적인 PID 제어기 파라미터를 저장하고 관리합니다.
/// 
/// PID 제어기 구성:
/// - Pitch PID: 로봇의 균형(기울기) 제어
/// - Velocity PID: 전진/후진 속도 제어
/// 
/// 주요 기능:
/// - SharedPreferences를 통한 설정 영구 저장
/// - 실시간 파라미터 수정 및 적용
/// - 기본값 제공 및 초기화
/// - ESP32로 전송을 위한 맵 변환
class BalancePidSettings {
  // Pitch PID (for balance control)
  /// @brief Pitch PID 비례 게인 (균형 제어용)
  /// @details 로봇의 기울기 오차에 비례하는 제어 출력 계수
  double pitchKp;
  
  /// @brief Pitch PID 적분 게인 (균형 제어용)  
  /// @details 누적된 기울기 오차를 보정하는 제어 출력 계수
  double pitchKi;
  
  /// @brief Pitch PID 미분 게인 (균형 제어용)
  /// @details 기울기 변화율에 대응하는 제어 출력 계수
  double pitchKd;

  // Velocity PID (for forward/backward movement)
  /// @brief Velocity PID 비례 게인 (속도 제어용)
  /// @details 목표 속도와 현재 속도 차이에 비례하는 제어 출력 계수
  double velocityKp;
  
  /// @brief Velocity PID 적분 게인 (속도 제어용)
  /// @details 누적된 속도 오차를 보정하는 제어 출력 계수
  double velocityKi;
  
  /// @brief Velocity PID 미분 게인 (속도 제어용)
  /// @details 속도 변화율에 대응하는 제어 출력 계수
  double velocityKd;

  // Other settings
  /// @brief 목표 속도 설정값 (단위: m/s 또는 상대값)
  double targetVelocity;
  
  /// @brief 최대 허용 기울기 각도 (단위: 도)
  /// @details 안전을 위한 기울기 제한값
  double maxTiltAngle;

  /// @brief BalancePidSettings 생성자
  /// @param pitchKp Pitch PID 비례 게인 (기본값: 50.0)
  /// @param pitchKi Pitch PID 적분 게인 (기본값: 0.0)
  /// @param pitchKd Pitch PID 미분 게인 (기본값: 2.0)
  /// @param velocityKp Velocity PID 비례 게인 (기본값: 1.0)
  /// @param velocityKi Velocity PID 적분 게인 (기본값: 0.1)
  /// @param velocityKd Velocity PID 미분 게인 (기본값: 0.0)
  /// @param targetVelocity 목표 속도 (기본값: 0.0)
  /// @param maxTiltAngle 최대 기울기 각도 (기본값: 45.0도)
  BalancePidSettings({
    this.pitchKp = 50.0,
    this.pitchKi = 0.0,
    this.pitchKd = 2.0,
    this.velocityKp = 1.0,
    this.velocityKi = 0.1,
    this.velocityKd = 0.0,
    this.targetVelocity = 0.0,
    this.maxTiltAngle = 45.0,
  });

  /// @brief SharedPreferences에서 PID 설정을 로드합니다
  /// @return 로드된 BalancePidSettings 인스턴스
  /// @details 저장된 설정이 없으면 기본값을 사용합니다.
  static Future<BalancePidSettings> load() async {
    final prefs = await SharedPreferences.getInstance();
    return BalancePidSettings(
      pitchKp: prefs.getDouble('pitch_kp') ?? 50.0,
      pitchKi: prefs.getDouble('pitch_ki') ?? 0.0,
      pitchKd: prefs.getDouble('pitch_kd') ?? 2.0,
      velocityKp: prefs.getDouble('velocity_kp') ?? 1.0,
      velocityKi: prefs.getDouble('velocity_ki') ?? 0.1,
      velocityKd: prefs.getDouble('velocity_kd') ?? 0.0,
      targetVelocity: prefs.getDouble('target_velocity') ?? 0.0,
      maxTiltAngle: prefs.getDouble('max_tilt_angle') ?? 45.0,
    );
  }

  /// @brief 현재 PID 설정을 SharedPreferences에 저장합니다
  /// @details 모든 PID 파라미터를 영구 저장소에 저장합니다.
  Future<void> save() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setDouble('pitch_kp', pitchKp);
    await prefs.setDouble('pitch_ki', pitchKi);
    await prefs.setDouble('pitch_kd', pitchKd);
    await prefs.setDouble('velocity_kp', velocityKp);
    await prefs.setDouble('velocity_ki', velocityKi);
    await prefs.setDouble('velocity_kd', velocityKd);
    await prefs.setDouble('target_velocity', targetVelocity);
    await prefs.setDouble('max_tilt_angle', maxTiltAngle);
  }

  /// @brief 현재 설정의 복사본을 생성합니다
  /// @return 동일한 값을 가진 새로운 BalancePidSettings 인스턴스
  BalancePidSettings copy() {
    return BalancePidSettings(
      pitchKp: pitchKp,
      pitchKi: pitchKi,
      pitchKd: pitchKd,
      velocityKp: velocityKp,
      velocityKi: velocityKi,
      velocityKd: velocityKd,
      targetVelocity: targetVelocity,
      maxTiltAngle: maxTiltAngle,
    );
  }

  /// @brief Pitch PID 파라미터를 맵 형태로 변환합니다
  /// @return Pitch PID 파라미터 맵 {'kp': value, 'ki': value, 'kd': value}
  /// @details ESP32로 전송하기 위한 데이터 형식으로 변환
  Map<String, dynamic> toPitchPidMap() {
    return {
      'kp': pitchKp,
      'ki': pitchKi,
      'kd': pitchKd,
    };
  }

  /// @brief Velocity PID 파라미터를 맵 형태로 변환합니다
  /// @return Velocity PID 파라미터 맵 {'kp': value, 'ki': value, 'kd': value}
  /// @details ESP32로 전송하기 위한 데이터 형식으로 변환
  Map<String, dynamic> toVelocityPidMap() {
    return {
      'kp': velocityKp,
      'ki': velocityKi,
      'kd': velocityKd,
    };
  }

  /// @brief PID 설정을 문자열로 변환합니다
  /// @return PID 파라미터 정보를 포함한 문자열
  /// @details 디버깅 및 로깅용 문자열 표현
  @override
  String toString() {
    return 'BalancePidSettings(pitch: Kp=$pitchKp Ki=$pitchKi Kd=$pitchKd, velocity: Kp=$velocityKp Ki=$velocityKi Kd=$velocityKd)';
  }
}