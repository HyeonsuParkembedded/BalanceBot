import 'dart:convert';
import 'dart:io';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:http/http.dart' as http;
import '../models/pid_settings.dart';

class CommunicationService {
  BluetoothConnection? _bluetoothConnection;
  String? _wifiIpAddress;
  bool _isConnected = false;
  CommunicationType _connectionType = CommunicationType.none;

  bool get isConnected => _isConnected;
  CommunicationType get connectionType => _connectionType;

  Future<bool> connect() async {
    try {
      bool bluetoothSuccess = await _connectBluetooth();
      if (bluetoothSuccess) {
        _connectionType = CommunicationType.bluetooth;
        _isConnected = true;
        return true;
      }

      bool wifiSuccess = await _connectWiFi();
      if (wifiSuccess) {
        _connectionType = CommunicationType.wifi;
        _isConnected = true;
        return true;
      }

      return false;
    } catch (e) {
      print('Connection error: $e');
      return false;
    }
  }

  Future<bool> _connectBluetooth() async {
    try {
      if (!await FlutterBluetoothSerial.instance.isEnabled) {
        return false;
      }

      List<BluetoothDevice> devices = await FlutterBluetoothSerial.instance.getBondedDevices();

      BluetoothDevice? balanceBotDevice;
      for (BluetoothDevice device in devices) {
        if (device.name?.toLowerCase().contains('balance') == true ||
            device.name?.toLowerCase().contains('esp32') == true) {
          balanceBotDevice = device;
          break;
        }
      }

      if (balanceBotDevice == null) {
        return false;
      }

      _bluetoothConnection = await BluetoothConnection.toAddress(balanceBotDevice.address);
      return _bluetoothConnection?.isConnected ?? false;
    } catch (e) {
      print('Bluetooth connection error: $e');
      return false;
    }
  }

  Future<bool> _connectWiFi() async {
    try {
      List<String> possibleIps = [
        '192.168.4.1',
        '192.168.1.100',
        '10.0.0.100',
      ];

      for (String ip in possibleIps) {
        try {
          final response = await http.get(
            Uri.parse('http://$ip/status'),
            headers: {'Content-Type': 'application/json'},
          ).timeout(const Duration(seconds: 3));

          if (response.statusCode == 200) {
            _wifiIpAddress = ip;
            return true;
          }
        } catch (e) {
          continue;
        }
      }

      return false;
    } catch (e) {
      print('WiFi connection error: $e');
      return false;
    }
  }

  Future<void> disconnect() async {
    try {
      if (_bluetoothConnection != null) {
        await _bluetoothConnection!.close();
        _bluetoothConnection = null;
      }
      _wifiIpAddress = null;
      _isConnected = false;
      _connectionType = CommunicationType.none;
    } catch (e) {
      print('Disconnect error: $e');
    }
  }

  Future<bool> sendPidSettings(PidSettings settings) async {
    if (!_isConnected) return false;

    try {
      final message = {
        'type': 'pid_update',
        'data': settings.toMap(),
      };

      if (_connectionType == CommunicationType.bluetooth) {
        return await _sendBluetoothMessage(message);
      } else if (_connectionType == CommunicationType.wifi) {
        return await _sendWiFiMessage(message);
      }

      return false;
    } catch (e) {
      print('Send PID settings error: $e');
      return false;
    }
  }

  Future<bool> setRobotActive(bool active) async {
    if (!_isConnected) return false;

    try {
      final message = {
        'type': 'robot_control',
        'data': {'active': active},
      };

      if (_connectionType == CommunicationType.bluetooth) {
        return await _sendBluetoothMessage(message);
      } else if (_connectionType == CommunicationType.wifi) {
        return await _sendWiFiMessage(message);
      }

      return false;
    } catch (e) {
      print('Set robot active error: $e');
      return false;
    }
  }

  Future<bool> _sendBluetoothMessage(Map<String, dynamic> message) async {
    try {
      if (_bluetoothConnection == null || !_bluetoothConnection!.isConnected) {
        return false;
      }

      String jsonMessage = jsonEncode(message);
      _bluetoothConnection!.output.add(utf8.encode('$jsonMessage\n'));
      await _bluetoothConnection!.output.allSent;
      return true;
    } catch (e) {
      print('Bluetooth send error: $e');
      return false;
    }
  }

  Future<bool> _sendWiFiMessage(Map<String, dynamic> message) async {
    try {
      if (_wifiIpAddress == null) return false;

      final response = await http.post(
        Uri.parse('http://$_wifiIpAddress/control'),
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode(message),
      ).timeout(const Duration(seconds: 5));

      return response.statusCode == 200;
    } catch (e) {
      print('WiFi send error: $e');
      return false;
    }
  }

  void dispose() {
    disconnect();
  }
}

enum CommunicationType {
  none,
  bluetooth,
  wifi,
}