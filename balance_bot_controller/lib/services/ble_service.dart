/// @file ble_service.dart
/// @brief ESP32 BalanceBot과의 블루투스 저에너지(BLE) 통신 서비스
/// @details GATT 프로토콜을 사용하여 로봇 제어 명령 전송 및 상태 수신을 담당
/// @author BalanceBot Development Team
/// @date 2025
library ble_service;

import 'dart:async';
import 'dart:typed_data';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import '../models/balance_pid_settings.dart';
import '../utils/protocol_utils.dart';

/// @class BleService
/// @brief ESP32 BalanceBot과의 BLE 통신을 관리하는 서비스 클래스
/// @details GATT 서비스를 통해 로봇과 양방향 통신을 수행합니다.
/// 
/// 주요 기능:
/// - 자동 디바이스 스캔 및 연결
/// - PID 파라미터 전송
/// - 조이스틱 제어 명령 전송  
/// - 실시간 로봇 상태 수신 (각도, 속도, 배터리)
/// - 연결 상태 관리
/// 
/// BLE 프로토콜:
/// - Service UUID: 0000ff00-0000-1000-8000-00805f9b34fb
/// - Command Characteristic: 0000ff01-0000-1000-8000-00805f9b34fb (Write)
/// - Status Characteristic: 0000ff02-0000-1000-8000-00805f9b34fb (Notify)
class BleService {
  /// @brief 연결된 블루투스 디바이스
  BluetoothDevice? _device;
  
  /// @brief 명령 전송용 GATT Characteristic
  BluetoothCharacteristic? _commandCharacteristic;
  
  /// @brief 상태 수신용 GATT Characteristic
  BluetoothCharacteristic? _statusCharacteristic;
  
  /// @brief 상태 알림 구독
  StreamSubscription? _statusSubscription;
  
  /// @brief BLE 연결 상태
  bool _isConnected = false;

  // BLE Service and Characteristic UUIDs (matching ESP32)
  /// @brief BLE 서비스 UUID (ESP32와 매칭)
  static const String serviceUuid = "0000ff00-0000-1000-8000-00805f9b34fb";
  
  /// @brief 명령 전송 Characteristic UUID
  static const String commandCharUuid = "0000ff01-0000-1000-8000-00805f9b34fb";
  
  /// @brief 상태 수신 Characteristic UUID  
  static const String statusCharUuid = "0000ff02-0000-1000-8000-00805f9b34fb";

  // Status callback
  /// @brief 로봇 상태 수신 콜백 함수
  /// @param angle 로봇 기울기 각도 (도)
  /// @param velocity 각속도 (도/초)
  /// @param batteryLevel 배터리 잔량 (0-100%)
  Function(double angle, double velocity, int batteryLevel)? onStatusReceived;

  /// @brief BLE 연결 상태 확인
  /// @return 연결되어 있으면 true, 아니면 false
  bool get isConnected => _isConnected;
  
  /// @brief 연결된 블루투스 디바이스 정보
  /// @return 연결된 디바이스 또는 null
  BluetoothDevice? get connectedDevice => _device;

  /// @brief ESP32 BalanceBot에 연결을 시도합니다
  /// @return 연결 성공 시 true, 실패 시 false
  /// @details 블루투스 어댑터 상태 확인 → 디바이스 스캔 → GATT 연결 → 서비스 검색 순으로 진행
  Future<bool> connect() async {
    try {
      // Check if Bluetooth is available and turned on
      if (await FlutterBluePlus.isSupported == false) {
        return false;
      }

      if (await FlutterBluePlus.adapterState.first != BluetoothAdapterState.on) {
        return false;
      }

      // Start scanning
      FlutterBluePlus.startScan(timeout: const Duration(seconds: 10));

      // Listen for scan results
      BluetoothDevice? targetDevice;
      await for (List<ScanResult> results in FlutterBluePlus.scanResults) {
        for (ScanResult result in results) {
        // Look for device with our service UUID or device name containing "Balance"
        if (result.device.platformName.toLowerCase().contains('balance') ||
            result.device.platformName.toLowerCase().contains('esp32') ||
            result.advertisementData.serviceUuids.contains(Guid(serviceUuid))) {
          targetDevice = result.device;
          break;
        }
        }
      }

      FlutterBluePlus.stopScan();

      if (targetDevice == null) {
        return false;
      }

      // Connect to device
      await targetDevice.connect();
      _device = targetDevice;

      // Discover services
      List<BluetoothService> services = await targetDevice.discoverServices();

      BluetoothService? targetService;
      for (BluetoothService service in services) {
        if (service.uuid.toString().toLowerCase() == serviceUuid.toLowerCase()) {
          targetService = service;
          break;
        }
      }

      if (targetService == null) {
        await disconnect();
        return false;
      }

      // Get characteristics
      for (BluetoothCharacteristic characteristic in targetService.characteristics) {
        String uuid = characteristic.uuid.toString().toLowerCase();
        if (uuid == commandCharUuid.toLowerCase()) {
          _commandCharacteristic = characteristic;
        } else if (uuid == statusCharUuid.toLowerCase()) {
          _statusCharacteristic = characteristic;
        }
      }

      if (_commandCharacteristic == null) {
        await disconnect();
        return false;
      }

      // Subscribe to status notifications if available
      if (_statusCharacteristic != null) {
        try {
          await _statusCharacteristic!.setNotifyValue(true);
          _statusSubscription = _statusCharacteristic!.lastValueStream.listen((data) {
            _handleStatusData(data);
          });
        } catch (e) {
          // Failed to subscribe
        }
      }

      _isConnected = true;
      return true;

    } catch (e) {
      await disconnect();
      return false;
    }
  }

  Future<void> disconnect() async {
    try {
      await _statusSubscription?.cancel();
      _statusSubscription = null;

      if (_device != null && _device!.isConnected) {
        await _device!.disconnect();
      }

      _device = null;
      _commandCharacteristic = null;
      _statusCharacteristic = null;
      _isConnected = false;
    } catch (e) {
      // Disconnect error
    }
  }

  Future<bool> sendMoveCommand({
    int direction = 0,  // -1, 0, 1
    int turn = 0,       // -100 to 100
    int speed = 0,      // 0 to 100
    bool balance = true,
    bool standup = false,
  }) async {
    if (!_isConnected || _commandCharacteristic == null) {
      return false;
    }

    try {
      // Build protocol message
      Uint8List message = ProtocolUtils.buildMoveCommand(
        direction: direction,
        turn: turn,
        speed: speed,
        balance: balance,
        standup: standup,
      );

      // Send command
      await _commandCharacteristic!.write(message, withoutResponse: true);
      return true;
    } catch (e) {
      return false;
    }
  }

  Future<bool> sendPidSettings(BalancePidSettings settings) async {
    if (!_isConnected || _commandCharacteristic == null) {
      return false;
    }

    try {
      // Send pitch PID configuration
      Uint8List pitchConfig = ProtocolUtils.buildConfigMessage(
        configId: 0x01, // Pitch PID config ID
        kp: settings.pitchKp,
        ki: settings.pitchKi,
        kd: settings.pitchKd,
      );

      await _commandCharacteristic!.write(pitchConfig, withoutResponse: true);
      await Future.delayed(const Duration(milliseconds: 50));

      // Send velocity PID configuration
      Uint8List velocityConfig = ProtocolUtils.buildConfigMessage(
        configId: 0x02, // Velocity PID config ID
        kp: settings.velocityKp,
        ki: settings.velocityKi,
        kd: settings.velocityKd,
      );

      await _commandCharacteristic!.write(velocityConfig, withoutResponse: true);

      return true;
    } catch (e) {
      return false;
    }
  }

  Future<bool> setBalanceMode(bool enabled) async {
    return await sendMoveCommand(balance: enabled);
  }

  Future<bool> requestStandup() async {
    return await sendMoveCommand(standup: true);
  }

  void _handleStatusData(List<int> data) {
    try {
      Map<String, dynamic>? status = ProtocolUtils.parseStatusResponse(Uint8List.fromList(data));

      if (status != null && onStatusReceived != null) {
        onStatusReceived!(
          status['angle'] ?? 0.0,
          status['velocity'] ?? 0.0,
          status['battery_level'] ?? 0,
        );
      }
    } catch (e) {
      // Failed to parse status data
    }
  }

  void dispose() {
    disconnect();
  }
}
