import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'screens/control_screen.dart';

void main() {
  runApp(const BalanceBotApp());
}

class BalanceBotApp extends StatelessWidget {
  const BalanceBotApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Balance Bot Controller',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blue),
        useMaterial3: true,
      ),
      home: const ControlScreen(),
    );
  }
}