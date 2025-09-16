@echo off
echo Building for ESP32-S3...

echo.
echo Building ESP32 firmware...
pio run -e esp32-s3-devkitc-1

if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo ESP32 build complete. Ready for upload.
echo Use 'pio run -e esp32-s3-devkitc-1 -t upload' to flash the device.
pause