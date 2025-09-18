import 'package:shared_preferences/shared_preferences.dart';

class PidSettings {
  double kp;
  double ki;
  double kd;
  double outputLimit;

  PidSettings({
    this.kp = 1.0,
    this.ki = 0.1,
    this.kd = 0.05,
    this.outputLimit = 100.0,
  });

  static Future<PidSettings> load() async {
    final prefs = await SharedPreferences.getInstance();
    return PidSettings(
      kp: prefs.getDouble('pid_kp') ?? 1.0,
      ki: prefs.getDouble('pid_ki') ?? 0.1,
      kd: prefs.getDouble('pid_kd') ?? 0.05,
      outputLimit: prefs.getDouble('pid_output_limit') ?? 100.0,
    );
  }

  Future<void> save() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setDouble('pid_kp', kp);
    await prefs.setDouble('pid_ki', ki);
    await prefs.setDouble('pid_kd', kd);
    await prefs.setDouble('pid_output_limit', outputLimit);
  }

  PidSettings copy() {
    return PidSettings(
      kp: kp,
      ki: ki,
      kd: kd,
      outputLimit: outputLimit,
    );
  }

  Map<String, double> toMap() {
    return {
      'kp': kp,
      'ki': ki,
      'kd': kd,
      'output_limit': outputLimit,
    };
  }

  @override
  String toString() {
    return 'PidSettings(kp: $kp, ki: $ki, kd: $kd, outputLimit: $outputLimit)';
  }
}