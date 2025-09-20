import 'package:flutter/material.dart';

import './MainPage.dart';

void main() => runApp(ExampleApplication());

class ExampleApplication extends StatelessWidget {
  const ExampleApplication({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(home: MainPage());
  }
}
