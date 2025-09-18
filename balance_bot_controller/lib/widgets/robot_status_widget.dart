import 'package:flutter/material.dart';

class RobotStatusWidget extends StatelessWidget {
  final bool isActive;
  final bool isConnected;
  final VoidCallback onToggle;
  final VoidCallback onStandup;
  final double currentAngle;
  final double currentVelocity;

  const RobotStatusWidget({
    super.key,
    required this.isActive,
    required this.isConnected,
    required this.onToggle,
    required this.onStandup,
    this.currentAngle = 0.0,
    this.currentVelocity = 0.0,
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
                  isActive ? Icons.play_circle_filled : Icons.pause_circle_filled,
                  color: isActive ? Colors.green : Colors.orange,
                  size: 30,
                ),
                const SizedBox(width: 16),
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        '로봇 상태',
                        style: Theme.of(context).textTheme.titleMedium,
                      ),
                      Text(
                        isActive ? '균형 제어 활성화' : '균형 제어 비활성화',
                        style: TextStyle(
                          color: isActive ? Colors.green : Colors.orange,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ],
                  ),
                ),
                Switch(
                  value: isActive,
                  onChanged: isConnected ? (_) => onToggle() : null,
                  activeColor: Colors.green,
                ),
              ],
            ),
            if (isConnected) ...[
              const SizedBox(height: 16),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        '현재 기울기',
                        style: Theme.of(context).textTheme.bodySmall,
                      ),
                      Text(
                        '${currentAngle.toStringAsFixed(2)}°',
                        style: Theme.of(context).textTheme.titleMedium?.copyWith(
                          color: currentAngle.abs() > 30 ? Colors.red : Colors.black,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ],
                  ),
                  Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        '현재 속도',
                        style: Theme.of(context).textTheme.bodySmall,
                      ),
                      Text(
                        '${currentVelocity.toStringAsFixed(2)} cm/s',
                        style: Theme.of(context).textTheme.titleMedium?.copyWith(
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ],
                  ),
                  ElevatedButton.icon(
                    onPressed: isConnected ? onStandup : null,
                    icon: const Icon(Icons.accessibility_new),
                    label: const Text('기립'),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.orange,
                      foregroundColor: Colors.white,
                    ),
                  ),
                ],
              ),
            ],
          ],
        ),
      ),
    );
  }
}