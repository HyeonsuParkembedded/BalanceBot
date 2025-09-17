import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import '../services/ble_service.dart';
import '../models/balance_pid_settings.dart';
import '../widgets/balance_pid_widget.dart';
import '../widgets/connection_status_widget.dart';
import '../widgets/robot_status_widget.dart';
import '../widgets/robot_control_widget.dart';

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
  String _connectionStatus = "연결 안됨";
  String _robotState = "IDLE";
  double _currentAngle = 0.0;
  double _currentVelocity = 0.0;
  int _batteryLevel = 0;

  @override
  void initState() {
    super.initState();
    _loadPidSettings();
    _setupBleCallbacks();
  }

  void _setupBleCallbacks() {
    _bleService.onStatusReceived = (angle, velocity, batteryLevel) {
      setState(() {
        _currentAngle = angle;
        _currentVelocity = velocity;
        _batteryLevel = batteryLevel;
      });
    };
  }

  void _loadPidSettings() async {
    final settings = await BalancePidSettings.load();
    setState(() {
      _pidSettings = settings;
    });
  }

  void _updatePidSettings(BalancePidSettings newSettings) async {
    await newSettings.save();
    setState(() {
      _pidSettings = newSettings;
    });

    if (_isConnected) {
      await _bleService.sendPidSettings(newSettings);
    }
  }

  void _connectToRobot() async {
    try {
      setState(() {
        _connectionStatus = "연결 중...";
      });

      final success = await _bleService.connect();
      setState(() {
        _isConnected = success;
        _connectionStatus = success ? "연결됨" : "연결 실패";
      });

      if (success) {
        await _bleService.sendPidSettings(_pidSettings);
      }
    } catch (e) {
      setState(() {
        _isConnected = false;
        _connectionStatus = "연결 오류: ${e.toString()}";
      });
    }
  }

  void _disconnectFromRobot() async {
    await _bleService.disconnect();
    setState(() {
      _isConnected = false;
      _connectionStatus = "연결 안됨";
      _isRobotActive = false;
      _currentAngle = 0.0;
      _currentVelocity = 0.0;
      _batteryLevel = 0;
    });
  }

  void _toggleRobotActive() async {
    if (!_isConnected) return;

    try {
      final newState = !_isRobotActive;
      await _bleService.setBalanceMode(newState);
      setState(() {
        _isRobotActive = newState;
      });
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('로봇 상태 변경 실패: ${e.toString()}')),
      );
    }
  }

  void _requestStandup() async {
    if (!_isConnected) return;

    try {
      await _bleService.requestStandup();
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('기립 명령을 전송했습니다')),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('기립 명령 실패: ${e.toString()}')),
      );
    }
  }

  void _sendMoveCommand(int direction, int turn) async {
    if (!_isConnected) return;

    await _bleService.sendMoveCommand(
      direction: direction,
      turn: turn,
      speed: 50,  // Default speed
      balance: _isRobotActive,
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: const Text('Balance Bot Controller'),
        centerTitle: true,
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            ConnectionStatusWidget(
              isConnected: _isConnected,
              status: _connectionStatus,
              onConnect: _connectToRobot,
              onDisconnect: _disconnectFromRobot,
              batteryLevel: _batteryLevel,
              deviceName: _bleService.connectedDevice?.localName ?? "Unknown",
            ),
            const SizedBox(height: 20),
            RobotStatusWidget(
              isActive: _isRobotActive,
              isConnected: _isConnected,
              onToggle: _toggleRobotActive,
              onStandup: _requestStandup,
              currentAngle: _currentAngle,
              currentVelocity: _currentVelocity,
            ),
            const SizedBox(height: 20),
            RobotControlWidget(
              isConnected: _isConnected,
              onMoveCommand: _sendMoveCommand,
            ),
            const SizedBox(height: 30),
            Expanded(
              child: BalancePidWidget(
                pidSettings: _pidSettings,
                onPidChanged: _updatePidSettings,
                isConnected: _isConnected,
              ),
            ),
          ],
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