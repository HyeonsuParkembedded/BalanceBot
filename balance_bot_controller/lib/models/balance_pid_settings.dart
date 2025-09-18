import 'package:shared_preferences/shared_preferences.dart';

class BalancePidSettings {
  // Pitch PID (for balance control)
  double pitchKp;
  double pitchKi;
  double pitchKd;

  // Velocity PID (for forward/backward movement)
  double velocityKp;
  double velocityKi;
  double velocityKd;

  // Other settings
  double targetVelocity;
  double maxTiltAngle;

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

  Map<String, dynamic> toPitchPidMap() {
    return {
      'kp': pitchKp,
      'ki': pitchKi,
      'kd': pitchKd,
    };
  }

  Map<String, dynamic> toVelocityPidMap() {
    return {
      'kp': velocityKp,
      'ki': velocityKi,
      'kd': velocityKd,
    };
  }

  @override
  String toString() {
    return 'BalancePidSettings(pitch: Kp=$pitchKp Ki=$pitchKi Kd=$pitchKd, velocity: Kp=$velocityKp Ki=$velocityKi Kd=$velocityKd)';
  }
}