import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import '../utils/design_tokens.dart';

class SampleSvgDemo extends StatelessWidget {
  const SampleSvgDemo({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: DesignTokens.bg,
      appBar: AppBar(
        title: const Text('SVG Demo'),
        backgroundColor: DesignTokens.primary,
      ),
      body: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Text('Components: joystick', style: TextStyle(fontSize: DesignTokens.body)),
            const SizedBox(height: 12),
            SvgPicture.asset(
              'assets/design/components/joystick.svg',
              width: 200,
              height: 200,
            ),
            const SizedBox(height: 24),
            const Text('Screen mockup: portrait', style: TextStyle(fontSize: DesignTokens.body)),
            const SizedBox(height: 12),
            SvgPicture.asset(
              'assets/design/screens/portrait_main.svg',
              width: 300,
            ),
          ],
        ),
      ),
    );
  }
}
