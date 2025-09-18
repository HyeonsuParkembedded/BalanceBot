import 'package:flutter/material.dart';

class RobotControlWidget extends StatelessWidget {
  final bool isConnected;
  final Function(int direction, int turn) onMoveCommand;

  const RobotControlWidget({
    super.key,
    required this.isConnected,
    required this.onMoveCommand,
  });

  @override
  Widget build(BuildContext context) {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          children: [
            Text(
              '로봇 조종',
              style: Theme.of(context).textTheme.titleMedium,
            ),
            const SizedBox(height: 16),
            Column(
              children: [
                // Forward button
                ElevatedButton(
                  onPressed: isConnected
                    ? () => onMoveCommand(1, 0) // Forward
                    : null,
                  style: ElevatedButton.styleFrom(
                    shape: const CircleBorder(),
                    padding: const EdgeInsets.all(20),
                  ),
                  child: const Icon(Icons.keyboard_arrow_up, size: 30),
                ),
                const SizedBox(height: 8),
                Row(
                  mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                  children: [
                    // Left button
                    ElevatedButton(
                      onPressed: isConnected
                        ? () => onMoveCommand(0, -50) // Turn left
                        : null,
                      style: ElevatedButton.styleFrom(
                        shape: const CircleBorder(),
                        padding: const EdgeInsets.all(20),
                      ),
                      child: const Icon(Icons.keyboard_arrow_left, size: 30),
                    ),
                    // Stop button
                    ElevatedButton(
                      onPressed: isConnected
                        ? () => onMoveCommand(0, 0) // Stop
                        : null,
                      style: ElevatedButton.styleFrom(
                        backgroundColor: Colors.red,
                        foregroundColor: Colors.white,
                        shape: const CircleBorder(),
                        padding: const EdgeInsets.all(20),
                      ),
                      child: const Icon(Icons.stop, size: 30),
                    ),
                    // Right button
                    ElevatedButton(
                      onPressed: isConnected
                        ? () => onMoveCommand(0, 50) // Turn right
                        : null,
                      style: ElevatedButton.styleFrom(
                        shape: const CircleBorder(),
                        padding: const EdgeInsets.all(20),
                      ),
                      child: const Icon(Icons.keyboard_arrow_right, size: 30),
                    ),
                  ],
                ),
                const SizedBox(height: 8),
                // Backward button
                ElevatedButton(
                  onPressed: isConnected
                    ? () => onMoveCommand(-1, 0) // Backward
                    : null,
                  style: ElevatedButton.styleFrom(
                    shape: const CircleBorder(),
                    padding: const EdgeInsets.all(20),
                  ),
                  child: const Icon(Icons.keyboard_arrow_down, size: 30),
                ),
              ],
            ),
            const SizedBox(height: 16),
            Text(
              isConnected ? '조종 가능' : 'BLE 연결 필요',
              style: Theme.of(context).textTheme.bodySmall?.copyWith(
                color: isConnected ? Colors.green : Colors.grey,
              ),
            ),
          ],
        ),
      ),
    );
  }
}