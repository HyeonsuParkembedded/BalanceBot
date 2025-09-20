# BalanceBot 문서화 스크립트
# Doxygen을 사용하여 ESP32와 Flutter 프로젝트의 통합 문서를 생성합니다.

param(
    [string]$Language = "korean",
    [switch]$CleanBuild = $false
)

Write-Host "BalanceBot 문서 생성 시작..." -ForegroundColor Green

# 현재 디렉토리 확인
if (!(Test-Path "Doxyfile")) {
    Write-Error "Doxyfile을 찾을 수 없습니다. 프로젝트 루트 디렉토리에서 실행해주세요."
    exit 1
}

# 출력 디렉토리 정리 (CleanBuild 옵션)
if ($CleanBuild -and (Test-Path "docs")) {
    Write-Host "기존 문서 디렉토리 정리 중..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "docs"
}

# Doxygen 실행
Write-Host "Doxygen 문서 생성 중..." -ForegroundColor Cyan
try {
    & doxygen Doxyfile
    if ($LASTEXITCODE -eq 0) {
        Write-Host "문서 생성 완료!" -ForegroundColor Green
        Write-Host "생성된 문서 위치: docs/html/index.html" -ForegroundColor Cyan
        
        # HTML 문서 열기 제안
        $openDoc = Read-Host "생성된 문서를 브라우저에서 열까요? (y/N)"
        if ($openDoc -eq "y" -or $openDoc -eq "Y") {
            Start-Process "docs/html/index.html"
        }
    } else {
        Write-Error "Doxygen 실행 중 오류가 발생했습니다."
        exit 1
    }
} catch {
    Write-Error "Doxygen을 실행할 수 없습니다. Doxygen이 설치되어 있는지 확인해주세요."
    Write-Host "Doxygen 설치 방법:" -ForegroundColor Yellow
    Write-Host "  - Windows: https://www.doxygen.nl/download.html" -ForegroundColor Gray
    Write-Host "  - 또는 Chocolatey: choco install doxygen.install" -ForegroundColor Gray
    exit 1
}

Write-Host "문서화 프로세스가 완료되었습니다." -ForegroundColor Green