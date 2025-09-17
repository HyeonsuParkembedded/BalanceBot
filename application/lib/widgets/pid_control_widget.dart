import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import '../models/pid_settings.dart';

class PidControlWidget extends StatefulWidget {
  final PidSettings pidSettings;
  final Function(PidSettings) onPidChanged;
  final bool isConnected;

  const PidControlWidget({
    super.key,
    required this.pidSettings,
    required this.onPidChanged,
    required this.isConnected,
  });

  @override
  State<PidControlWidget> createState() => _PidControlWidgetState();
}

class _PidControlWidgetState extends State<PidControlWidget> {
  late TextEditingController _kpController;
  late TextEditingController _kiController;
  late TextEditingController _kdController;
  late TextEditingController _outputLimitController;

  @override
  void initState() {
    super.initState();
    _initControllers();
  }

  void _initControllers() {
    _kpController = TextEditingController(text: widget.pidSettings.kp.toString());
    _kiController = TextEditingController(text: widget.pidSettings.ki.toString());
    _kdController = TextEditingController(text: widget.pidSettings.kd.toString());
    _outputLimitController = TextEditingController(text: widget.pidSettings.outputLimit.toString());
  }

  @override
  void didUpdateWidget(PidControlWidget oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.pidSettings != widget.pidSettings) {
      _updateControllers();
    }
  }

  void _updateControllers() {
    _kpController.text = widget.pidSettings.kp.toString();
    _kiController.text = widget.pidSettings.ki.toString();
    _kdController.text = widget.pidSettings.kd.toString();
    _outputLimitController.text = widget.pidSettings.outputLimit.toString();
  }

  void _updatePidValue() {
    try {
      final kp = double.parse(_kpController.text);
      final ki = double.parse(_kiController.text);
      final kd = double.parse(_kdController.text);
      final outputLimit = double.parse(_outputLimitController.text);

      final newSettings = PidSettings(
        kp: kp,
        ki: ki,
        kd: kd,
        outputLimit: outputLimit,
      );

      widget.onPidChanged(newSettings);
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('잘못된 입력값입니다. 숫자를 입력해주세요.')),
      );
    }
  }

  void _resetToDefault() {
    final defaultSettings = PidSettings();
    widget.onPidChanged(defaultSettings);
  }

  Widget _buildPidSlider({
    required String label,
    required double value,
    required double min,
    required double max,
    required int divisions,
    required Function(double) onChanged,
    required TextEditingController controller,
  }) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Text(
                  label,
                  style: Theme.of(context).textTheme.titleMedium,
                ),
                SizedBox(
                  width: 80,
                  child: TextField(
                    controller: controller,
                    keyboardType: const TextInputType.numberWithOptions(decimal: true),
                    inputFormatters: [
                      FilteringTextInputFormatter.allow(RegExp(r'^\d*\.?\d*')),
                    ],
                    decoration: const InputDecoration(
                      isDense: true,
                      contentPadding: EdgeInsets.symmetric(horizontal: 8, vertical: 8),
                    ),
                    onSubmitted: (value) => _updatePidValue(),
                  ),
                ),
              ],
            ),
            const SizedBox(height: 10),
            Slider(
              value: value.clamp(min, max),
              min: min,
              max: max,
              divisions: divisions,
              label: value.toStringAsFixed(3),
              onChanged: onChanged,
            ),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Text(min.toString(), style: Theme.of(context).textTheme.bodySmall),
                Text(max.toString(), style: Theme.of(context).textTheme.bodySmall),
              ],
            ),
          ],
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(
              'PID 제어 설정',
              style: Theme.of(context).textTheme.headlineSmall,
            ),
            Row(
              children: [
                ElevatedButton(
                  onPressed: _resetToDefault,
                  child: const Text('기본값'),
                ),
                const SizedBox(width: 10),
                ElevatedButton(
                  onPressed: widget.isConnected ? _updatePidValue : null,
                  child: const Text('적용'),
                ),
              ],
            ),
          ],
        ),
        const SizedBox(height: 20),
        Expanded(
          child: SingleChildScrollView(
            child: Column(
              children: [
                _buildPidSlider(
                  label: 'Kp (비례 이득)',
                  value: widget.pidSettings.kp,
                  min: 0.0,
                  max: 10.0,
                  divisions: 1000,
                  controller: _kpController,
                  onChanged: (value) {
                    final newSettings = widget.pidSettings.copy();
                    newSettings.kp = value;
                    widget.onPidChanged(newSettings);
                  },
                ),
                _buildPidSlider(
                  label: 'Ki (적분 이득)',
                  value: widget.pidSettings.ki,
                  min: 0.0,
                  max: 5.0,
                  divisions: 500,
                  controller: _kiController,
                  onChanged: (value) {
                    final newSettings = widget.pidSettings.copy();
                    newSettings.ki = value;
                    widget.onPidChanged(newSettings);
                  },
                ),
                _buildPidSlider(
                  label: 'Kd (미분 이득)',
                  value: widget.pidSettings.kd,
                  min: 0.0,
                  max: 2.0,
                  divisions: 200,
                  controller: _kdController,
                  onChanged: (value) {
                    final newSettings = widget.pidSettings.copy();
                    newSettings.kd = value;
                    widget.onPidChanged(newSettings);
                  },
                ),
                _buildPidSlider(
                  label: '출력 제한',
                  value: widget.pidSettings.outputLimit,
                  min: 10.0,
                  max: 255.0,
                  divisions: 245,
                  controller: _outputLimitController,
                  onChanged: (value) {
                    final newSettings = widget.pidSettings.copy();
                    newSettings.outputLimit = value;
                    widget.onPidChanged(newSettings);
                  },
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }

  @override
  void dispose() {
    _kpController.dispose();
    _kiController.dispose();
    _kdController.dispose();
    _outputLimitController.dispose();
    super.dispose();
  }
}