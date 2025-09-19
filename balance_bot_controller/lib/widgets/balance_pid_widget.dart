import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import '../models/balance_pid_settings.dart';

class BalancePidWidget extends StatefulWidget {
  final BalancePidSettings pidSettings;
  final Function(BalancePidSettings) onPidChanged;
  final bool isConnected;

  const BalancePidWidget({
    super.key,
    required this.pidSettings,
    required this.onPidChanged,
    required this.isConnected,
  });

  @override
  State<BalancePidWidget> createState() => _BalancePidWidgetState();
}

class _BalancePidWidgetState extends State<BalancePidWidget> with SingleTickerProviderStateMixin {
  late TabController _tabController;

  // Pitch PID controllers
  late TextEditingController _pitchKpController;
  late TextEditingController _pitchKiController;
  late TextEditingController _pitchKdController;

  // Velocity PID controllers
  late TextEditingController _velocityKpController;
  late TextEditingController _velocityKiController;
  late TextEditingController _velocityKdController;

  @override
  void initState() {
    super.initState();
    _tabController = TabController(length: 2, vsync: this);
    _initControllers();
  }

  void _initControllers() {
    _pitchKpController = TextEditingController(text: widget.pidSettings.pitchKp.toString());
    _pitchKiController = TextEditingController(text: widget.pidSettings.pitchKi.toString());
    _pitchKdController = TextEditingController(text: widget.pidSettings.pitchKd.toString());

    _velocityKpController = TextEditingController(text: widget.pidSettings.velocityKp.toString());
    _velocityKiController = TextEditingController(text: widget.pidSettings.velocityKi.toString());
    _velocityKdController = TextEditingController(text: widget.pidSettings.velocityKd.toString());
  }

  @override
  void didUpdateWidget(BalancePidWidget oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.pidSettings != widget.pidSettings) {
      _updateControllers();
    }
  }

  void _updateControllers() {
    _pitchKpController.text = widget.pidSettings.pitchKp.toString();
    _pitchKiController.text = widget.pidSettings.pitchKi.toString();
    _pitchKdController.text = widget.pidSettings.pitchKd.toString();

    _velocityKpController.text = widget.pidSettings.velocityKp.toString();
    _velocityKiController.text = widget.pidSettings.velocityKi.toString();
    _velocityKdController.text = widget.pidSettings.velocityKd.toString();
  }

  void _updatePidValue() {
    try {
      final newSettings = BalancePidSettings(
        pitchKp: double.parse(_pitchKpController.text),
        pitchKi: double.parse(_pitchKiController.text),
        pitchKd: double.parse(_pitchKdController.text),
        velocityKp: double.parse(_velocityKpController.text),
        velocityKi: double.parse(_velocityKiController.text),
        velocityKd: double.parse(_velocityKdController.text),
        targetVelocity: widget.pidSettings.targetVelocity,
        maxTiltAngle: widget.pidSettings.maxTiltAngle,
      );

      widget.onPidChanged(newSettings);
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('잘못된 입력값입니다. 숫자를 입력해주세요.')),
      );
    }
  }

  void _resetToDefault() {
    final defaultSettings = BalancePidSettings();
    widget.onPidChanged(defaultSettings);
  }

  Widget _buildPidSlider({
    required String label,
    required String description,
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
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        label,
                        style: Theme.of(context).textTheme.titleMedium,
                      ),
                      Text(
                        description,
                        style: Theme.of(context).textTheme.bodySmall?.copyWith(
                          color: Colors.grey[600],
                        ),
                      ),
                    ],
                  ),
                ),
                SizedBox(
                  width: 80,
                  child: TextField(
                    controller: controller,
                    keyboardType: const TextInputType.numberWithOptions(decimal: true),
                    inputFormatters: [
                      FilteringTextInputFormatter.allow(RegExp(r'^-?\d*\.?\d*')),
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

  Widget _buildPitchPidTab() {
    return SingleChildScrollView(
      child: Column(
        children: [
          const SizedBox(height: 10),
          Text(
            '균형(Pitch) PID 제어',
            style: Theme.of(context).textTheme.titleLarge,
          ),
          const SizedBox(height: 10),
          _buildPidSlider(
            label: 'Pitch Kp (비례 이득)',
            description: '현재 기울기 오차에 비례하는 제어',
            value: widget.pidSettings.pitchKp,
            min: 0.0,
            max: 100.0,
            divisions: 1000,
            controller: _pitchKpController,
            onChanged: (value) {
              final newSettings = widget.pidSettings.copy();
              newSettings.pitchKp = value;
              widget.onPidChanged(newSettings);
            },
          ),
          _buildPidSlider(
            label: 'Pitch Ki (적분 이득)',
            description: '누적된 기울기 오차에 대한 제어',
            value: widget.pidSettings.pitchKi,
            min: 0.0,
            max: 10.0,
            divisions: 1000,
            controller: _pitchKiController,
            onChanged: (value) {
              final newSettings = widget.pidSettings.copy();
              newSettings.pitchKi = value;
              widget.onPidChanged(newSettings);
            },
          ),
          _buildPidSlider(
            label: 'Pitch Kd (미분 이득)',
            description: '기울기 변화율에 대한 제어',
            value: widget.pidSettings.pitchKd,
            min: 0.0,
            max: 20.0,
            divisions: 2000,
            controller: _pitchKdController,
            onChanged: (value) {
              final newSettings = widget.pidSettings.copy();
              newSettings.pitchKd = value;
              widget.onPidChanged(newSettings);
            },
          ),
        ],
      ),
    );
  }

  Widget _buildVelocityPidTab() {
    return SingleChildScrollView(
      child: Column(
        children: [
          const SizedBox(height: 10),
          Text(
            '속도(Velocity) PID 제어',
            style: Theme.of(context).textTheme.titleLarge,
          ),
          const SizedBox(height: 10),
          _buildPidSlider(
            label: 'Velocity Kp (비례 이득)',
            description: '목표 속도와 현재 속도 차이에 비례',
            value: widget.pidSettings.velocityKp,
            min: 0.0,
            max: 5.0,
            divisions: 500,
            controller: _velocityKpController,
            onChanged: (value) {
              final newSettings = widget.pidSettings.copy();
              newSettings.velocityKp = value;
              widget.onPidChanged(newSettings);
            },
          ),
          _buildPidSlider(
            label: 'Velocity Ki (적분 이득)',
            description: '누적된 속도 오차에 대한 제어',
            value: widget.pidSettings.velocityKi,
            min: 0.0,
            max: 2.0,
            divisions: 200,
            controller: _velocityKiController,
            onChanged: (value) {
              final newSettings = widget.pidSettings.copy();
              newSettings.velocityKi = value;
              widget.onPidChanged(newSettings);
            },
          ),
          _buildPidSlider(
            label: 'Velocity Kd (미분 이득)',
            description: '속도 변화율에 대한 제어',
            value: widget.pidSettings.velocityKd,
            min: 0.0,
            max: 1.0,
            divisions: 100,
            controller: _velocityKdController,
            onChanged: (value) {
              final newSettings = widget.pidSettings.copy();
              newSettings.velocityKd = value;
              widget.onPidChanged(newSettings);
            },
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Flexible(
              child: Text(
                'Balance PID 제어 설정',
                style: Theme.of(context).textTheme.headlineSmall?.copyWith(fontSize: 14),
                overflow: TextOverflow.ellipsis,
              ),
            ),
            Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                ElevatedButton(
                  onPressed: _resetToDefault,
                  style: ElevatedButton.styleFrom(
                    padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                  ),
                  child: const Text('기본값', style: TextStyle(fontSize: 12)),
                ),
                const SizedBox(width: 8),
                ElevatedButton(
                  onPressed: widget.isConnected ? _updatePidValue : null,
                  style: ElevatedButton.styleFrom(
                    padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                  ),
                  child: const Text('적용', style: TextStyle(fontSize: 12)),
                ),
              ],
            ),
          ],
        ),
        const SizedBox(height: 16),
        TabBar(
          controller: _tabController,
          tabs: const [
            Tab(text: '균형 제어 (Pitch)'),
            Tab(text: '속도 제어 (Velocity)'),
          ],
        ),
        SizedBox(
          height: 300, // Fixed height instead of Expanded
          child: TabBarView(
            controller: _tabController,
            children: [
              _buildPitchPidTab(),
              _buildVelocityPidTab(),
            ],
          ),
        ),
      ],
    );
  }

  @override
  void dispose() {
    _tabController.dispose();
    _pitchKpController.dispose();
    _pitchKiController.dispose();
    _pitchKdController.dispose();
    _velocityKpController.dispose();
    _velocityKiController.dispose();
    _velocityKdController.dispose();
    super.dispose();
  }
}