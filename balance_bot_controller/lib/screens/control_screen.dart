import 'package:flutter/material.dart';
import 'dart:math' as math;
import '../services/ble_service.dart';
import '../models/balance_pid_settings.dart';
import '../widgets/balance_pid_widget.dart';

class ControlScreen extends StatefulWidget {
  const ControlScreen({super.key});

  @override
  State<ControlScreen> createState() => _ControlScreenState();
}

class _ControlScreenState extends State<ControlScreen> {
  final BleService _bleService = BleService();
  BalancePidSettings _pidSettings = BalancePidSettings();
  bool _isConnected = false;
  bool _isRobotActive = false;
  int _batteryLevel = 0;

  // 조이스틱 상태 변수들
  double _leftJoystickX = 0.0;
  double _leftJoystickY = 0.0;
  double _rightJoystickX = 0.0;
  double _rightJoystickY = 0.0;

  // 세로 모드용 단일 조이스틱 상태
  double _singleJoystickX = 0.0;
  double _singleJoystickY = 0.0;

  // 조이스틱 설정
  static const double joystickSize = 120.0;
  static const double knobSize = 40.0;
  static const double maxDistance = (joystickSize - knobSize) / 2;

  // 성능 최적화를 위한 throttling
  DateTime _lastCommandTime = DateTime.now();
  static const _commandThrottleMs = 50;

  @override
  void initState() {
    super.initState();
    _loadPidSettings();
    _setupBleCallbacks();
  }

  void _setupBleCallbacks() {
    _bleService.onStatusReceived = (angle, velocity, batteryLevel) {
      if (!mounted) return;
      setState(() {
        _batteryLevel = batteryLevel;
      });
    };
  }

  void _loadPidSettings() async {
    final settings = await BalancePidSettings.load();
    if (!mounted) return;
    setState(() {
      _pidSettings = settings;
    });
  }

  void _updatePidSettings(BalancePidSettings newSettings) async {
    await newSettings.save();
    if (!mounted) return;
    setState(() {
      _pidSettings = newSettings;
    });

    if (_isConnected) {
      await _bleService.sendPidSettings(newSettings);
    }
  }

  void _connectToRobot() async {
    final success = await _bleService.connect();
    if (!mounted) return;
    setState(() {
      _isConnected = success;
    });

    if (success) {
      await _bleService.sendPidSettings(_pidSettings);
    }
  }

  void _disconnectFromRobot() async {
    await _bleService.disconnect();
    if (!mounted) return;
    setState(() {
      _isConnected = false;
      _isRobotActive = false;
      _batteryLevel = 0;
    });
  }

  void _toggleRobotActive() async {
    if (!_isConnected) return;

    try {
      final newState = !_isRobotActive;
      await _bleService.setBalanceMode(newState);
      if (!mounted) return;
      setState(() {
        _isRobotActive = newState;
      });
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('로봇 상태 변경 실패: ${e.toString()}')),
      );
    }
  }

  void _requestStandup() async {
    if (!_isConnected) return;

    try {
      await _bleService.requestStandup();
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('기립 명령을 전송했습니다')),
      );
    } catch (e) {
      if (!mounted) return;
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('기립 명령 실패: ${e.toString()}')),
      );
    }
  }

  void _sendMoveCommand() async {
    if (!_isConnected) return;

    // Throttling을 적용하여 성능 최적화
    final now = DateTime.now();
    if (now.difference(_lastCommandTime).inMilliseconds < _commandThrottleMs) {
      return;
    }
    _lastCommandTime = now;

    final speed = _getSpeed();
    final turn = _getTurn();

    // -1.0 ~ 1.0 값을 -100 ~ 100으로 변환
    final direction = (speed * 100).round();
    final turnValue = (turn * 100).round();

    await _bleService.sendMoveCommand(
      direction: direction,
      turn: turnValue,
      speed: 50,  // Default speed
      balance: _isRobotActive,
    );
  }

  double _getSpeed() {
    // 세로 모드에서는 단일 조이스틱 Y축, 가로 모드에서는 왼쪽 조이스틱 Y축
    final isPortrait = MediaQuery.of(context).orientation == Orientation.portrait;
    return isPortrait ? -_singleJoystickY : -_leftJoystickY;
  }

  double _getTurn() {
    // 세로 모드에서는 단일 조이스틱 X축, 가로 모드에서는 오른쪽 조이스틱 X축
    final isPortrait = MediaQuery.of(context).orientation == Orientation.portrait;
    return isPortrait ? _singleJoystickX : _rightJoystickX;
  }

  Widget _buildJoystick({
    required double x,
    required double y,
    required Function(double, double) onUpdate,
    bool verticalOnly = false,
    bool horizontalOnly = false,
  }) {
    return GestureDetector(
      onPanUpdate: (details) {
        if (!_isConnected) return;

        final RenderBox renderBox = context.findRenderObject() as RenderBox;
        const center = Offset(joystickSize / 2, joystickSize / 2);
        final localPosition = renderBox.globalToLocal(details.globalPosition);

        double deltaX = localPosition.dx - center.dx;
        double deltaY = localPosition.dy - center.dy;

        // 방향 제한 적용
        if (verticalOnly) deltaX = 0;
        if (horizontalOnly) deltaY = 0;

        final distance = math.sqrt(deltaX * deltaX + deltaY * deltaY);

        if (distance <= maxDistance) {
          onUpdate(deltaX / maxDistance, deltaY / maxDistance);
        } else {
          final angle = math.atan2(deltaY, deltaX);
          onUpdate(
            verticalOnly ? 0 : math.cos(angle),
            horizontalOnly ? 0 : math.sin(angle),
          );
        }
      },
      onPanEnd: (details) {
        // 조이스틱을 중앙으로 되돌림
        onUpdate(0, 0);
      },
      child: Container(
        width: joystickSize,
        height: joystickSize,
        decoration: BoxDecoration(
          shape: BoxShape.circle,
          color: Colors.grey[300],
          border: Border.all(color: Colors.grey[600]!, width: 2),
        ),
        child: Stack(
          children: [
            // 조이스틱 노브
            Positioned(
              left: (joystickSize / 2) + (x * maxDistance) - (knobSize / 2),
              top: (joystickSize / 2) + (y * maxDistance) - (knobSize / 2),
              child: Container(
                width: knobSize,
                height: knobSize,
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  color: _isConnected ? Colors.blue : Colors.grey,
                  boxShadow: [
                    BoxShadow(
                      color: Colors.black.withAlpha(76),
                      blurRadius: 4,
                      offset: const Offset(2, 2),
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildPortraitLayout() {
    return Column(
      children: [
        // 상단 제목
        Container(
          padding: const EdgeInsets.symmetric(vertical: 8.0),
          child: const Text(
            'Balance Bot Controller',
            style: TextStyle(
              fontSize: 18,
              fontWeight: FontWeight.bold,
              color: Colors.blue,
            ),
          ),
        ),
        // 상단: 제어 영역
        Expanded(
          flex: 3,
          child: SingleChildScrollView(
            child: Column(
              children: [
                // BLE 연결 상태
                Container(
                  padding: const EdgeInsets.all(12),
                  decoration: BoxDecoration(
                    color: _isConnected ? Colors.green.withAlpha(25) : Colors.grey.withAlpha(25),
                    borderRadius: BorderRadius.circular(12),
                    border: Border.all(
                      color: _isConnected ? Colors.green : Colors.grey,
                      width: 2,
                    ),
                  ),
                  child: Column(
                    children: [
                      Icon(
                        _isConnected ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                        size: 32,
                        color: _isConnected ? Colors.green : Colors.grey,
                      ),
                      const SizedBox(height: 8),
                      Text(
                        _isConnected ? '연결됨' : '연결 안됨',
                        style: TextStyle(
                          color: _isConnected ? Colors.green : Colors.grey,
                          fontWeight: FontWeight.bold,
                          fontSize: 14,
                        ),
                      ),
                      if (_isConnected) ...[
                        const SizedBox(height: 4),
                        Text(
                          _bleService.connectedDevice?.platformName ?? "Unknown",
                          style: const TextStyle(fontSize: 12, color: Colors.grey),
                        ),
                        Text(
                          '배터리: $_batteryLevel%',
                          style: const TextStyle(fontSize: 12, color: Colors.grey),
                        ),
                      ],
                      const SizedBox(height: 12),
                      // BLE 연결 버튼
                      ElevatedButton.icon(
                        onPressed: _isConnected ? _disconnectFromRobot : _connectToRobot,
                        icon: Icon(_isConnected ? Icons.bluetooth_disabled : Icons.bluetooth),
                        label: Text(_isConnected ? '연결 해제' : 'BLE 연결'),
                        style: ElevatedButton.styleFrom(
                          backgroundColor: _isConnected ? Colors.red : Colors.blue,
                          foregroundColor: Colors.white,
                        ),
                      ),
                    ],
                  ),
                ),
                const SizedBox(height: 16),
                // 로봇 제어 버튼들
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  children: [
                    ElevatedButton.icon(
                      onPressed: _isConnected ? _toggleRobotActive : null,
                      icon: Icon(
                        _isRobotActive ? Icons.pause_circle : Icons.play_circle,
                        size: 20,
                      ),
                      label: Text(
                        _isRobotActive ? '균형 제어\n비활성화' : '균형 제어\n활성화',
                        style: const TextStyle(fontSize: 11),
                        textAlign: TextAlign.center,
                      ),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: _isRobotActive ? Colors.orange : Colors.blue,
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 8),
                      ),
                    ),
                    ElevatedButton.icon(
                      onPressed: _isConnected ? _requestStandup : null,
                      icon: const Icon(Icons.vertical_align_top, size: 20),
                      label: const Text('기립', style: TextStyle(fontSize: 12)),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: Colors.green,
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
                      ),
                    ),
                  ],
                ),
              ],
            ),
          ),
        ),
        // 하단: 단일 조이스틱
        Expanded(
          flex: 2,
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              const Text(
                '전진/후진 & 좌우 조향',
                style: TextStyle(fontSize: 14, fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 10),
              _buildJoystick(
                x: _singleJoystickX,
                y: _singleJoystickY,
                onUpdate: (x, y) {
                  if (!mounted) return;
                  setState(() {
                    _singleJoystickX = x;
                    _singleJoystickY = y;
                  });
                  _sendMoveCommand();
                },
              ),
            ],
          ),
        ),
      ],
    );
  }

  Widget _buildLandscapeLayout() {
    return Column(
      children: [
        // 상단 제목
        Container(
          padding: const EdgeInsets.symmetric(vertical: 8.0),
          child: const Text(
            'Balance Bot Controller',
            style: TextStyle(
              fontSize: 18,
              fontWeight: FontWeight.bold,
              color: Colors.blue,
            ),
          ),
        ),
        // 메인 컨텐츠 - 3개 컬럼 구조
        Expanded(
          child: Row(
            children: [
              // 첫 번째 컬럼: 전진/후진 조이스틱만
              Expanded(
                flex: 1,
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const Text(
                      '전진/후진',
                      style: TextStyle(fontSize: 14, fontWeight: FontWeight.bold),
                    ),
                    const SizedBox(height: 10),
                    _buildJoystick(
                      x: _leftJoystickX,
                      y: _leftJoystickY,
                      onUpdate: (x, y) {
                        if (!mounted) return;
                        setState(() {
                          _leftJoystickX = x;
                          _leftJoystickY = y;
                        });
                        _sendMoveCommand();
                      },
                      verticalOnly: true,
                    ),
                  ],
                ),
              ),

              const SizedBox(width: 16),

              // 두 번째 컬럼: 중앙 제어 영역 (BLE + 로봇 제어 + PID)
              Expanded(
                flex: 2,
                child: SingleChildScrollView(
                  child: Column(
                    children: [
                      // BLE 연결 상태
                      Container(
                        padding: const EdgeInsets.all(12),
                        decoration: BoxDecoration(
                          color: _isConnected ? Colors.green.withAlpha(25) : Colors.grey.withAlpha(25),
                          borderRadius: BorderRadius.circular(12),
                          border: Border.all(
                            color: _isConnected ? Colors.green : Colors.grey,
                            width: 2,
                          ),
                        ),
                        child: Column(
                          children: [
                            Icon(
                              _isConnected ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                              size: 32,
                              color: _isConnected ? Colors.green : Colors.grey,
                            ),
                            const SizedBox(height: 8),
                            Text(
                              _isConnected ? '연결됨' : '연결 안됨',
                              style: TextStyle(
                                color: _isConnected ? Colors.green : Colors.grey,
                                fontWeight: FontWeight.bold,
                                fontSize: 14,
                              ),
                            ),
                            if (_isConnected) ...[
                              const SizedBox(height: 4),
                              Text(
                                _bleService.connectedDevice?.platformName ?? "Unknown",
                                style: const TextStyle(fontSize: 12, color: Colors.grey),
                              ),
                              Text(
                                '배터리: $_batteryLevel%',
                                style: const TextStyle(fontSize: 12, color: Colors.grey),
                              ),
                            ],
                            const SizedBox(height: 12),
                            // BLE 연결 버튼
                            ElevatedButton.icon(
                              onPressed: _isConnected ? _disconnectFromRobot : _connectToRobot,
                              icon: Icon(_isConnected ? Icons.bluetooth_disabled : Icons.bluetooth),
                              label: Text(_isConnected ? '연결 해제' : 'BLE 연결'),
                              style: ElevatedButton.styleFrom(
                                backgroundColor: _isConnected ? Colors.red : Colors.blue,
                                foregroundColor: Colors.white,
                              ),
                            ),
                          ],
                        ),
                      ),

                      const SizedBox(height: 16),

                      // 로봇 제어 버튼들
                      Column(
                        children: [
                          ElevatedButton.icon(
                            onPressed: _isConnected ? _toggleRobotActive : null,
                            icon: Icon(
                              _isRobotActive ? Icons.pause_circle : Icons.play_circle,
                              size: 20,
                            ),
                            label: Text(
                              _isRobotActive ? '균형 제어 비활성화' : '균형 제어 활성화',
                              style: const TextStyle(fontSize: 12),
                            ),
                            style: ElevatedButton.styleFrom(
                              backgroundColor: _isRobotActive ? Colors.orange : Colors.blue,
                              foregroundColor: Colors.white,
                              padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
                            ),
                          ),
                          const SizedBox(height: 8),
                          ElevatedButton.icon(
                            onPressed: _isConnected ? _requestStandup : null,
                            icon: const Icon(Icons.vertical_align_top, size: 20),
                            label: const Text('기립', style: TextStyle(fontSize: 12)),
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.green,
                              foregroundColor: Colors.white,
                              padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
                            ),
                          ),
                        ],
                      ),

                      const SizedBox(height: 16),

                      // PID 설정
                      BalancePidWidget(
                        pidSettings: _pidSettings,
                        onPidChanged: _updatePidSettings,
                        isConnected: _isConnected,
                      ),
                    ],
                  ),
                ),
              ),

              const SizedBox(width: 16),

              // 세 번째 컬럼: 좌우 조향 조이스틱만
              Expanded(
                flex: 1,
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const Text(
                      '좌우 조향',
                      style: TextStyle(fontSize: 14, fontWeight: FontWeight.bold),
                    ),
                    const SizedBox(height: 10),
                    _buildJoystick(
                      x: _rightJoystickX,
                      y: _rightJoystickY,
                      onUpdate: (x, y) {
                        if (!mounted) return;
                        setState(() {
                          _rightJoystickX = x;
                          _rightJoystickY = y;
                        });
                        _sendMoveCommand();
                      },
                      horizontalOnly: true,
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    final isPortrait = MediaQuery.of(context).orientation == Orientation.portrait;

    return Scaffold(
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(8.0),
          child: isPortrait ? _buildPortraitLayout() : _buildLandscapeLayout(),
        ),
      ),
    );
  }

  @override
  void dispose() {
    _bleService.dispose();
    super.dispose();
  }
}
