Local vendor copy for flutter_bluetooth_serial.

How to refresh from pub cache:

1. Close any running editors/builds.
2. Run the included PowerShell script `vendor_from_pubcache.ps1` which will copy the package from your pub cache into this folder and apply a namespace patch to the Android build file.

If you want to use a git fork instead, replace this folder with your fork and update `pubspec.yaml` accordingly.
