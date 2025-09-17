import 'dart:typed_data';

class ProtocolUtils {
  // Protocol constants
  static const int startMarker = 0xAA;
  static const int protocolVersion = 0x01;

  // Message types
  static const int msgTypeMoveCmd = 0x01;
  static const int msgTypeStatusReq = 0x02;
  static const int msgTypeStatusResp = 0x03;
  static const int msgTypeConfigSet = 0x04;
  static const int msgTypeConfigGet = 0x05;
  static const int msgTypeError = 0xFF;

  // Command flags
  static const int cmdFlagBalance = 0x01;
  static const int cmdFlagStandup = 0x02;
  static const int cmdFlagEmergency = 0x04;

  static int _sequenceNumber = 0;

  static Uint8List buildMoveCommand({
    required int direction,  // -1, 0, 1
    required int turn,       // -100 to 100
    required int speed,      // 0 to 100
    required bool balance,
    required bool standup,
  }) {
    // Clamp values to valid ranges
    direction = direction.clamp(-1, 1);
    turn = turn.clamp(-100, 100);
    speed = speed.clamp(0, 100);

    // Build flags
    int flags = 0;
    if (balance) flags |= cmdFlagBalance;
    if (standup) flags |= cmdFlagStandup;

    // Create payload (8 bytes for move command)
    ByteData payload = ByteData(8);
    payload.setInt8(0, direction);
    payload.setInt8(1, turn);
    payload.setUint8(2, speed);
    payload.setUint8(3, flags);
    payload.setUint32(4, DateTime.now().millisecondsSinceEpoch & 0xFFFFFFFF, Endian.little);

    return _buildMessage(msgTypeMoveCmd, payload.buffer.asUint8List());
  }

  static Uint8List buildConfigMessage({
    required int configId,
    required double kp,
    required double ki,
    required double kd,
  }) {
    // Create payload for PID configuration (13 bytes: 1 + 4*3)
    ByteData payload = ByteData(13);
    payload.setUint8(0, configId);
    payload.setFloat32(1, kp, Endian.little);
    payload.setFloat32(5, ki, Endian.little);
    payload.setFloat32(9, kd, Endian.little);

    return _buildMessage(msgTypeConfigSet, payload.buffer.asUint8List());
  }

  static Uint8List buildStatusRequest() {
    return _buildMessage(msgTypeStatusReq, Uint8List(0));
  }

  static Map<String, dynamic>? parseStatusResponse(Uint8List data) {
    if (data.length < 8) {
      print("Status response too short: ${data.length} bytes");
      return null;
    }

    try {
      // Parse header
      if (data[0] != startMarker) {
        print("Invalid start marker: ${data[0]}");
        return null;
      }

      int msgType = data[2];
      if (msgType != msgTypeStatusResp) {
        print("Not a status response: $msgType");
        return null;
      }

      int payloadLen = (data[5] << 8) | data[4];
      if (data.length < 8 + payloadLen) {
        print("Incomplete message: expected ${8 + payloadLen}, got ${data.length}");
        return null;
      }

      // Parse payload (status response should be at least 20 bytes)
      if (payloadLen < 20) {
        print("Status payload too short: $payloadLen bytes");
        return null;
      }

      ByteData payload = ByteData.sublistView(data, 8, 8 + payloadLen);

      return {
        'angle': payload.getFloat32(0, Endian.little),
        'velocity': payload.getFloat32(4, Endian.little),
        'robot_state': payload.getUint8(8),
        'gps_status': payload.getUint8(9),
        'battery_level': payloadLen > 18 ? payload.getUint8(18) : 50,
        'error_flags': payloadLen > 19 ? payload.getUint8(19) : 0,
      };
    } catch (e) {
      print("Error parsing status response: $e");
      return null;
    }
  }

  static Uint8List _buildMessage(int msgType, Uint8List payload) {
    int payloadLen = payload.length;

    // Create message buffer (8 byte header + payload)
    ByteData message = ByteData(8 + payloadLen);

    // Build header
    message.setUint8(0, startMarker);
    message.setUint8(1, protocolVersion);
    message.setUint8(2, msgType);
    message.setUint8(3, _sequenceNumber++);
    message.setUint16(4, payloadLen, Endian.little);

    // Add payload
    for (int i = 0; i < payloadLen; i++) {
      message.setUint8(8 + i, payload[i]);
    }

    // Calculate and set checksum (simple sum of all bytes except checksum field)
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
      checksum += message.getUint8(i);
    }
    for (int i = 0; i < payloadLen; i++) {
      checksum += payload[i];
    }
    message.setUint16(6, checksum & 0xFFFF, Endian.little);

    return message.buffer.asUint8List();
  }

  static String getRobotStateName(int state) {
    switch (state) {
      case 0: return "INIT";
      case 1: return "IDLE";
      case 2: return "BALANCING";
      case 3: return "STANDING_UP";
      case 4: return "FALLEN";
      case 5: return "ERROR";
      default: return "UNKNOWN";
    }
  }
}