import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'dart:math' as math;
import '../services/ble_service.dart';
import '../models/balance_pid_settings.dart';
import '../widgets/balance_pid_widget.dart';
import '../utils/design_tokens.dart';

class ControlScreenV2 extends StatefulWidget {
  const ControlScreenV2({super.key});

  @override
  State<ControlScreenV2> createState() => _ControlScreenV2State();
}

class _ControlScreenV2State extends State<ControlScreenV2> {
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
  static const double joystickSize = 160.0;
  static const double knobSize = 56.0;
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

  Widget _buildModernJoystick({
    required double x,
    required double y,
    required Function(double, double) onUpdate,
    bool verticalOnly = false,
    bool horizontalOnly = false,
    required String label,
  }) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        Text(
          label,
          style: TextStyle(
            fontSize: DesignTokens.body,
            fontWeight: FontWeight.w600,
            color: DesignTokens.primary,
            fontFamily: DesignTokens.fontFamily,
          ),
        ),
        const SizedBox(height: 12),
        GestureDetector(
          onPanUpdate: (details) {
            if (!_isConnected) return;

            final RenderBox? renderBox = context.findRenderObject() as RenderBox?;
            if (renderBox == null) return;
            
            const center = Offset(joystickSize / 2, joystickSize / 2);
            final localPosition = details.localPosition;

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
              color: DesignTokens.surface,
              border: Border.all(color: DesignTokens.primary.withOpacity(0.3), width: 2),
              boxShadow: [
                BoxShadow(
                  color: DesignTokens.primary.withOpacity(0.08),
                  blurRadius: 20,
                  spreadRadius: 5,
                ),
              ],
            ),
            child: Stack(
              children: [
                // 중앙 십자선
                Center(
                  child: Icon(
                    verticalOnly ? Icons.swap_vert : (horizontalOnly ? Icons.swap_horiz : Icons.control_camera),
                    color: DesignTokens.muted.withOpacity(0.3),
                    size: 40,
                  ),
                ),
                // 조이스틱 노브
                Positioned(
                  left: (joystickSize / 2) + (x * maxDistance) - (knobSize / 2),
                  top: (joystickSize / 2) + (y * maxDistance) - (knobSize / 2),
                  child: Container(
                    width: knobSize,
                    height: knobSize,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      gradient: _isConnected ? LinearGradient(
                        begin: Alignment.topLeft,
                        end: Alignment.bottomRight,
                        colors: [
                          DesignTokens.primary,
                          DesignTokens.primary700,
                        ],
                      ) : LinearGradient(
                        colors: [
                          DesignTokens.muted,
                          DesignTokens.muted.withOpacity(0.8),
                        ],
                      ),
                      boxShadow: [
                        BoxShadow(
                          color: _isConnected 
                            ? DesignTokens.primary.withOpacity(0.4)
                            : Colors.black.withOpacity(0.2),
                          blurRadius: 8,
                          offset: const Offset(0, 4),
                        ),
                      ],
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }

  Widget _buildStatusCard() {
    return Container(
      padding: EdgeInsets.all(DesignTokens.padding * 1.5),
      decoration: BoxDecoration(
        color: DesignTokens.surface,
        borderRadius: BorderRadius.circular(16),
        boxShadow: [
          BoxShadow(
            color: DesignTokens.muted.withOpacity(0.1),
            blurRadius: 10,
            offset: const Offset(0, 4),
          ),
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              Container(
                width: 48,
                height: 48,
                decoration: BoxDecoration(
                  color: _isConnected ? DesignTokens.success.withOpacity(0.1) : DesignTokens.muted.withOpacity(0.1),
                  borderRadius: BorderRadius.circular(12),
                ),
                child: Icon(
                  _isConnected ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                  size: 24,
                  color: _isConnected ? DesignTokens.success : DesignTokens.muted,
                ),
              ),
              const SizedBox(width: 12),
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      _isConnected ? '연결됨' : '연결 안됨',
                      style: TextStyle(
                        fontSize: DesignTokens.h2,
                        fontWeight: FontWeight.bold,
                        color: _isConnected ? DesignTokens.success : DesignTokens.muted,
                        fontFamily: DesignTokens.fontFamily,
                      ),
                    ),
                    if (_isConnected) ...[
                      Text(
                        _bleService.connectedDevice?.platformName ?? "Unknown",
                        style: TextStyle(
                          fontSize: DesignTokens.small,
                          color: DesignTokens.muted,
                          fontFamily: DesignTokens.fontFamily,
                        ),
                      ),
                    ],
                  ],
                ),
              ),
              if (_isConnected)
                Container(
                  padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
                  decoration: BoxDecoration(
                    color: DesignTokens.primary.withOpacity(0.1),
                    borderRadius: BorderRadius.circular(20),
                  ),
                  child: Row(
                    children: [
                      Icon(
                        Icons.battery_charging_full,
                        size: 16,
                        color: DesignTokens.primary,
                      ),
                      const SizedBox(width: 4),
                      Text(
                        '$_batteryLevel%',
                        style: TextStyle(
                          fontSize: DesignTokens.small,
                          fontWeight: FontWeight.bold,
                          color: DesignTokens.primary,
                          fontFamily: DesignTokens.fontFamily,
                        ),
                      ),
                    ],
                  ),
                ),
            ],
          ),
          const SizedBox(height: 16),
          SizedBox(
            width: double.infinity,
            child: ElevatedButton.icon(
              onPressed: _isConnected ? _disconnectFromRobot : _connectToRobot,
              icon: Icon(_isConnected ? Icons.bluetooth_disabled : Icons.bluetooth),
              label: Text(
                _isConnected ? '연결 해제' : 'BLE 연결',
                style: TextStyle(fontFamily: DesignTokens.fontFamily),
              ),
              style: ElevatedButton.styleFrom(
                backgroundColor: _isConnected ? DesignTokens.danger : DesignTokens.primary,
                foregroundColor: DesignTokens.surface,
                padding: const EdgeInsets.symmetric(vertical: 12),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(12),
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildControlButtons() {
    return Row(
      children: [
        Expanded(
          child: Container(
            decoration: BoxDecoration(
              color: DesignTokens.surface,
              borderRadius: BorderRadius.circular(12),
              boxShadow: [
                BoxShadow(
                  color: DesignTokens.muted.withOpacity(0.1),
                  blurRadius: 10,
                  offset: const Offset(0, 4),
                ),
              ],
            ),
            child: Material(
              color: Colors.transparent,
              child: InkWell(
                onTap: _isConnected ? _toggleRobotActive : null,
                borderRadius: BorderRadius.circular(12),
                child: Padding(
                  padding: const EdgeInsets.symmetric(vertical: 16),
                  child: Column(
                    children: [
                      Icon(
                        _isRobotActive ? Icons.pause_circle_filled : Icons.play_circle_filled,
                        size: 32,
                        color: _isConnected 
                          ? (_isRobotActive ? DesignTokens.danger : DesignTokens.success)
                          : DesignTokens.muted,
                      ),
                      const SizedBox(height: 8),
                      Text(
                        _isRobotActive ? '균형 제어\n비활성화' : '균형 제어\n활성화',
                        textAlign: TextAlign.center,
                        style: TextStyle(
                          fontSize: DesignTokens.small,
                          fontWeight: FontWeight.w600,
                          color: _isConnected ? null : DesignTokens.muted,
                          fontFamily: DesignTokens.fontFamily,
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ),
        const SizedBox(width: 12),
        Expanded(
          child: Container(
            decoration: BoxDecoration(
              color: DesignTokens.surface,
              borderRadius: BorderRadius.circular(12),
              boxShadow: [
                BoxShadow(
                  color: DesignTokens.muted.withOpacity(0.1),
                  blurRadius: 10,
                  offset: const Offset(0, 4),
                ),
              ],
            ),
            child: Material(
              color: Colors.transparent,
              child: InkWell(
                onTap: _isConnected ? _requestStandup : null,
                borderRadius: BorderRadius.circular(12),
                child: Padding(
                  padding: const EdgeInsets.symmetric(vertical: 16),
                  child: Column(
                    children: [
                      Icon(
                        Icons.arrow_upward_rounded,
                        size: 32,
                        color: _isConnected ? DesignTokens.success : DesignTokens.muted,
                      ),
                      const SizedBox(height: 8),
                      Text(
                        '기립',
                        style: TextStyle(
                          fontSize: DesignTokens.small,
                          fontWeight: FontWeight.w600,
                          color: _isConnected ? null : DesignTokens.muted,
                          fontFamily: DesignTokens.fontFamily,
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ),
      ],
    );
  }

  Widget _buildPortraitLayout() {
    return Container(
      decoration: BoxDecoration(
        gradient: LinearGradient(
          begin: Alignment.topCenter,
          end: Alignment.bottomCenter,
          colors: [
            DesignTokens.bg,
            DesignTokens.bg.withOpacity(0.8),
          ],
        ),
      ),
      child: Padding(
        padding: EdgeInsets.all(DesignTokens.gutter),
        child: Column(
          children: [
            // 상단 제목
            Text(
              'Balance Bot Controller',
              style: TextStyle(
                fontSize: DesignTokens.h1,
                fontWeight: FontWeight.bold,
                color: DesignTokens.primary,
                fontFamily: DesignTokens.fontFamily,
              ),
            ),
            const SizedBox(height: 20),
            
            Expanded(
              child: SingleChildScrollView(
                child: Column(
                  children: [
                    // 상태 카드
                    _buildStatusCard(),
                    const SizedBox(height: 20),
                    
                    // 제어 버튼들
                    _buildControlButtons(),
                    const SizedBox(height: 20),

                    // PID 설정
                    Container(
                      padding: EdgeInsets.all(DesignTokens.padding),
                      decoration: BoxDecoration(
                        color: DesignTokens.surface,
                        borderRadius: BorderRadius.circular(12),
                        boxShadow: [
                          BoxShadow(
                            color: DesignTokens.muted.withOpacity(0.1),
                            blurRadius: 10,
                            offset: const Offset(0, 4),
                          ),
                        ],
                      ),
                      child: BalancePidWidget(
                        pidSettings: _pidSettings,
                        onPidChanged: _updatePidSettings,
                        isConnected: _isConnected,
                      ),
                    ),
                  ],
                ),
              ),
            ),
            
            // 조이스틱
            _buildModernJoystick(
              x: _singleJoystickX,
              y: _singleJoystickY,
              label: '이동 제어',
              onUpdate: (x, y) {
                if (!mounted) return;
                setState(() {
                  _singleJoystickX = x;
                  _singleJoystickY = y;
                });
                _sendMoveCommand();
              },
            ),
            
            const SizedBox(height: 32),
          ],
        ),
      ),
    );
  }

  Widget _buildLandscapeLayout() {
    return Container(
      decoration: BoxDecoration(
        gradient: LinearGradient(
          begin: Alignment.topCenter,
          end: Alignment.bottomCenter,
          colors: [
            DesignTokens.bg,
            DesignTokens.bg.withOpacity(0.8),
          ],
        ),
      ),
      child: Padding(
        padding: EdgeInsets.all(DesignTokens.gutter),
        child: Column(
          children: [
            // 상단 제목
            Text(
              'Balance Bot Controller',
              style: TextStyle(
                fontSize: DesignTokens.h1,
                fontWeight: FontWeight.bold,
                color: DesignTokens.primary,
                fontFamily: DesignTokens.fontFamily,
              ),
            ),
            const SizedBox(height: 16),
            
            // 메인 콘텐츠
            Expanded(
              child: Row(
                children: [
                  // 왼쪽 조이스틱
                  Expanded(
                    flex: 3,
                    child: Center(
                      child: _buildModernJoystick(
                        x: _leftJoystickX,
                        y: _leftJoystickY,
                        label: '전진/후진',
                        verticalOnly: true,
                        onUpdate: (x, y) {
                          if (!mounted) return;
                          setState(() {
                            _leftJoystickX = x;
                            _leftJoystickY = y;
                          });
                          _sendMoveCommand();
                        },
                      ),
                    ),
                  ),
                  
                  // 중앙 제어 패널
                  Expanded(
                    flex: 4,
                    child: SingleChildScrollView(
                      child: Column(
                        children: [
                          _buildStatusCard(),
                          const SizedBox(height: 16),
                          _buildControlButtons(),
                          const SizedBox(height: 16),
                          // PID 설정
                          Container(
                            padding: EdgeInsets.all(DesignTokens.padding),
                            decoration: BoxDecoration(
                              color: DesignTokens.surface,
                              borderRadius: BorderRadius.circular(12),
                              boxShadow: [
                                BoxShadow(
                                  color: DesignTokens.muted.withOpacity(0.1),
                                  blurRadius: 10,
                                  offset: const Offset(0, 4),
                                ),
                              ],
                            ),
                            child: BalancePidWidget(
                              pidSettings: _pidSettings,
                              onPidChanged: _updatePidSettings,
                              isConnected: _isConnected,
                            ),
                          ),
                        ],
                      ),
                    ),
                  ),
                  
                  // 오른쪽 조이스틱
                  Expanded(
                    flex: 3,
                    child: Center(
                      child: _buildModernJoystick(
                        x: _rightJoystickX,
                        y: _rightJoystickY,
                        label: '좌우 조향',
                        horizontalOnly: true,
                        onUpdate: (x, y) {
                          if (!mounted) return;
                          setState(() {
                            _rightJoystickX = x;
                            _rightJoystickY = y;
                          });
                          _sendMoveCommand();
                        },
                      ),
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final isPortrait = MediaQuery.of(context).orientation == Orientation.portrait;

    return Scaffold(
      body: SafeArea(
        child: isPortrait ? _buildPortraitLayout() : _buildLandscapeLayout(),
      ),
    );
  }

  @override
  void dispose() {
    _bleService.dispose();
    super.dispose();
  }
}
