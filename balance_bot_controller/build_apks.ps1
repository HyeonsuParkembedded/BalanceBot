# BalanceBot Controller APK 빌드 스크립트
# PowerShell 스크립트로 디버그 및 릴리즈 APK를 빌드합니다

Write-Host "=== BalanceBot Controller APK 빌드 시작 ===" -ForegroundColor Green

# 현재 날짜와 시간으로 버전 태그 생성
$timestamp = Get-Date -Format "yyyyMMdd_HHmm"
$apkDir = "apks"

# APK 저장 디렉토리 생성
if (!(Test-Path $apkDir)) {
    New-Item -ItemType Directory -Path $apkDir
    Write-Host "APK 디렉토리 생성: $apkDir" -ForegroundColor Yellow
}

Write-Host "`n1. 디버그 APK 빌드 중..." -ForegroundColor Cyan
flutter build apk --debug
if ($LASTEXITCODE -eq 0) {
    $debugSource = "build\app\outputs\flutter-apk\app-debug.apk"
    $debugTarget = "$apkDir\BalanceBot_Debug_$timestamp.apk"
    
    if (Test-Path $debugSource) {
        Copy-Item $debugSource $debugTarget
        Write-Host "✅ 디버그 APK 생성 완료: $debugTarget" -ForegroundColor Green
    } else {
        Write-Host "❌ 디버그 APK 파일을 찾을 수 없습니다" -ForegroundColor Red
    }
} else {
    Write-Host "❌ 디버그 APK 빌드 실패" -ForegroundColor Red
}

Write-Host "`n2. 릴리즈 APK 빌드 중..." -ForegroundColor Cyan
flutter build apk --release
if ($LASTEXITCODE -eq 0) {
    $releaseSource = "build\app\outputs\flutter-apk\app-release.apk"
    $releaseTarget = "$apkDir\BalanceBot_Release_$timestamp.apk"
    
    if (Test-Path $releaseSource) {
        Copy-Item $releaseSource $releaseTarget
        Write-Host "✅ 릴리즈 APK 생성 완료: $releaseTarget" -ForegroundColor Green
    } else {
        Write-Host "❌ 릴리즈 APK 파일을 찾을 수 없습니다" -ForegroundColor Red
    }
} else {
    Write-Host "❌ 릴리즈 APK 빌드 실패" -ForegroundColor Red
}

Write-Host "`n3. APK 파일 목록:" -ForegroundColor Cyan
if (Test-Path $apkDir) {
    Get-ChildItem $apkDir -Filter "*.apk" | ForEach-Object {
        $size = [math]::Round($_.Length / 1MB, 2)
        Write-Host "  📱 $($_.Name) ($size MB)" -ForegroundColor White
    }
} else {
    Write-Host "  APK 파일이 없습니다." -ForegroundColor Yellow
}

Write-Host "`n=== 빌드 완료 ===" -ForegroundColor Green
Write-Host "사용법: 생성된 APK를 안드로이드 기기에 설치하여 테스트하세요." -ForegroundColor Yellow