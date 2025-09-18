import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import '../models/balance_pid_settings.dart';
import '../utils/protocol_utils.dart';

class BleService {
  BluetoothDevice? _device;
  BluetoothCharacteristic? _commandCharacteristic;
  BluetoothCharacteristic? _statusCharacteristic;
  StreamSubscription? _statusSubscription;
  bool _isConnected = false;

  // BLE Service and Characteristic UUIDs (matching ESP32)
  static const String serviceUuid = "0000ff00-0000-1000-8000-00805f9b34fb";
  static const String commandCharUuid = "0000ff01-0000-1000-8000-00805f9b34fb";
  static const String statusCharUuid = "0000ff02-0000-1000-8000-00805f9b34fb";

  // Status callback
  Function(double angle, double velocity, int batteryLevel)? onStatusReceived;

  bool get isConnected => _isConnected;
  BluetoothDevice? get connectedDevice => _device;

  Future<bool> connect() async {
    try {
      // Check if Bluetooth is available and turned on
      if (await FlutterBluePlus.isAvailable == false) {
        print("Bluetooth not available");
        return false;
      }

      if (await FlutterBluePlus.isOn == false) {
        print("Bluetooth is turned off");
        return false;
      }

      // Start scanning
      print("Starting BLE scan...");
      FlutterBluePlus.startScan(timeout: const Duration(seconds: 10));

      // Listen for scan results
      BluetoothDevice? targetDevice;
      await for (List<ScanResult> results in FlutterBluePlus.scanResults) {
        for (ScanResult result in results) {
        // Look for device with our service UUID or device name containing "Balance"
        if (result.device.localName.toLowerCase().contains('balance') ||
            result.device.localName.toLowerCase().contains('esp32') ||
            result.advertisementData.serviceUuids.contains(serviceUuid)) {
          targetDevice = result.device;
          print("Found target device: ${result.device.localName}");
          break;
        }
        }
      }

      FlutterBluePlus.stopScan();

      if (targetDevice == null) {
        print("Balance Bot device not found");
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
        print("Target service not found");
        await disconnect();
        return false;
      }

      // Get characteristics
      for (BluetoothCharacteristic characteristic in targetService.characteristics) {
        String uuid = characteristic.uuid.toString().toLowerCase();
        if (uuid == commandCharUuid.toLowerCase()) {
          _commandCharacteristic = characteristic;
          print("Command characteristic found");
        } else if (uuid == statusCharUuid.toLowerCase()) {
          _statusCharacteristic = characteristic;
          print("Status characteristic found");
        }
      }

      if (_commandCharacteristic == null) {
        print("Command characteristic not found");
        await disconnect();
        return false;
      }

      // Subscribe to status notifications if available
      if (_statusCharacteristic != null) {
        try {
          await _statusCharacteristic!.setNotifyValue(true);
          _statusSubscription = _statusCharacteristic!.value.listen((data) {
            _handleStatusData(data);
          });
          print("Subscribed to status notifications");
        } catch (e) {
          print("Failed to subscribe to status notifications: $e");
        }
      }

      _isConnected = true;
      print("Successfully connected to Balance Bot");
      return true;

    } catch (e) {
      print("Connection error: $e");
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
      print("Disconnected from Balance Bot");
    } catch (e) {
      print("Disconnect error: $e");
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
      print("Move command sent: dir=$direction, turn=$turn, speed=$speed, balance=$balance, standup=$standup");
      return true;
    } catch (e) {
      print("Failed to send move command: $e");
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

      print("PID settings sent successfully");
      return true;
    } catch (e) {
      print("Failed to send PID settings: $e");
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
      print("Failed to parse status data: $e");
    }
  }

  void dispose() {
    disconnect();
  }
}