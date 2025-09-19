import 'package:flutter/material.dart';
import 'dart:math' as math;

class RCJoystickWidget extends StatefulWidget {
  final bool isConnected;
  final Function(int direction, int turn) onMoveCommand;
  final bool isRobotActive;
  final Function() onToggleRobot;
  final Function() onStandup;
  final int batteryLevel;
  final String deviceName;

  const RCJoystickWidget({
    super.key,
    required this.isConnected,
    required this.onMoveCommand,
    required this.isRobotActive,
    required this.onToggleRobot,
    required this.onStandup,
    required this.batteryLevel,
    required this.deviceName,
  });

  @override
  State<RCJoystickWidget> createState() => _RCJoystickWidgetState();
}

class _RCJoystickWidgetState extends State<RCJoystickWidget> {
  // 왼쪽 조이스틱 (전진/후진)
  double leftJoystickX = 0.0;
  double leftJoystickY = 0.0;

  // 오른쪽 조이스틱 (좌우 조향)
  double rightJoystickX = 0.0;
  double rightJoystickY = 0.0;

  // 성능 최적화를 위한 throttling
  DateTime _lastCommandTime = DateTime.now();
  static const _commandThrottleMs = 50; // 50ms throttling

  static const double joystickSize = 120.0;
  static const double knobSize = 40.0;
  static const double maxDistance = (joystickSize - knobSize) / 2;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(20),
      child: Row(
        children: [
          // 제일 왼쪽 조이스틱 (전진/후진)
          Column(
            children: [
              const Text(
                '전진/후진',
                style: TextStyle(fontSize: 14, fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 10),
              _buildJoystick(
                x: leftJoystickX,
                y: leftJoystickY,
                onUpdate: (x, y) {
                  setState(() {
                    leftJoystickX = x;
                    leftJoystickY = y;
                  });
                  _sendCommand();
                },
                verticalOnly: true, // 세로 방향만 허용
              ),
            ],
          ),

          const SizedBox(width: 20),

          // 중앙 상태 및 제어 영역
          Expanded(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                // BLE 연결 상태
                Container(
                  padding: const EdgeInsets.all(12),
                  decoration: BoxDecoration(
                    color: widget.isConnected ? Colors.green.withAlpha(25) : Colors.grey.withAlpha(25),
                    borderRadius: BorderRadius.circular(12),
                    border: Border.all(
                      color: widget.isConnected ? Colors.green : Colors.grey,
                      width: 2,
                    ),
                  ),
                  child: Column(
                    children: [
                      Icon(
                        widget.isConnected ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                        size: 32,
                        color: widget.isConnected ? Colors.green : Colors.grey,
                      ),
                      const SizedBox(height: 8),
                      Text(
                        widget.isConnected ? '연결됨' : '연결 안됨',
                        style: TextStyle(
                          color: widget.isConnected ? Colors.green : Colors.grey,
                          fontWeight: FontWeight.bold,
                          fontSize: 14,
                        ),
                      ),
                      if (widget.isConnected) ...[
                        const SizedBox(height: 4),
                        Text(
                          widget.deviceName,
                          style: const TextStyle(fontSize: 12, color: Colors.grey),
                        ),
                        Text(
                          '배터리: ${widget.batteryLevel}%',
                          style: const TextStyle(fontSize: 12, color: Colors.grey),
                        ),
                      ],
                    ],
                  ),
                ),

                const SizedBox(height: 16),

                // 로봇 제어 버튼들
                Column(
                  children: [
                    ElevatedButton.icon(
                      onPressed: widget.isConnected ? widget.onToggleRobot : null,
                      icon: Icon(
                        widget.isRobotActive ? Icons.pause_circle : Icons.play_circle,
                        size: 20,
                      ),
                      label: Text(
                        widget.isRobotActive ? '균형 제어 비활성화' : '균형 제어 활성화',
                        style: const TextStyle(fontSize: 12),
                      ),
                      style: ElevatedButton.styleFrom(
                        backgroundColor: widget.isRobotActive ? Colors.orange : Colors.blue,
                        foregroundColor: Colors.white,
                        padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
                      ),
                    ),
                    const SizedBox(height: 8),
                    ElevatedButton.icon(
                      onPressed: widget.isConnected ? widget.onStandup : null,
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

          const SizedBox(width: 20),

          // 오른쪽 조이스틱 (좌우 조향)
          Column(
            children: [
              const Text(
                '좌우 조향',
                style: TextStyle(fontSize: 14, fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 10),
              _buildJoystick(
                x: rightJoystickX,
                y: rightJoystickY,
                onUpdate: (x, y) {
                  setState(() {
                    rightJoystickX = x;
                    rightJoystickY = y;
                  });
                  _sendCommand();
                },
                horizontalOnly: true, // 가로 방향만 허용
              ),
            ],
          ),
        ],
      ),
    );
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
        if (!widget.isConnected) return;

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
                  color: widget.isConnected ? Colors.blue : Colors.grey,
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

  double _getSpeed() {
    // 왼쪽 조이스틱의 Y축 값 (전진/후진)
    return -leftJoystickY; // Y축을 뒤집어서 위쪽이 전진이 되도록
  }

  double _getTurn() {
    // 오른쪽 조이스틱의 X축 값 (좌우 조향)
    return rightJoystickX;
  }

  void _sendCommand() {
    if (!widget.isConnected) return;

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

    widget.onMoveCommand(direction, turnValue);
  }
}
