import 'package:flutter/material.dart';

class ConnectionStatusWidget extends StatelessWidget {
  final bool isConnected;
  final String status;
  final VoidCallback onConnect;
  final VoidCallback onDisconnect;
  final int batteryLevel;
  final String deviceName;

  const ConnectionStatusWidget({
    super.key,
    required this.isConnected,
    required this.status,
    required this.onConnect,
    required this.onDisconnect,
    this.batteryLevel = 0,
    this.deviceName = "Unknown",
  });

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            Row(
              children: [
                Icon(
                  isConnected ? Icons.bluetooth_connected : Icons.bluetooth_disabled,
                  color: isConnected ? Colors.green : Colors.red,
                  size: 30,
                ),
                const SizedBox(width: 16),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        'BLE 연결 상태',
                        style: Theme.of(context).textTheme.titleMedium,
                      ),
                      Text(
                        status,
                        style: TextStyle(
                          color: isConnected ? Colors.green : Colors.red,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                      if (isConnected) ...[
                        Text(
                          '기기: $deviceName',
                          style: Theme.of(context).textTheme.bodySmall,
                        ),
                        if (batteryLevel > 0)
                          Row(
                            children: [
                              Icon(
                                batteryLevel > 20 ? Icons.battery_4_bar : Icons.battery_1_bar,
                                size: 16,
                                color: batteryLevel > 20 ? Colors.green : Colors.orange,
                              ),
                              const SizedBox(width: 4),
                              Text(
                                '배터리: $batteryLevel%',
                                style: Theme.of(context).textTheme.bodySmall,
                              ),
                            ],
                          ),
                      ],
                    ],
                  ),
                ),
                ElevatedButton(
                  onPressed: isConnected ? onDisconnect : onConnect,
                  style: ElevatedButton.styleFrom(
                    backgroundColor: isConnected ? Colors.red : Colors.blue,
                    foregroundColor: Colors.white,
                  ),
                  child: Text(isConnected ? '연결 해제' : '연결'),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}