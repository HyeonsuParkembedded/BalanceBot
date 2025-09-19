# Copy flutter_bluetooth_serial from pub cache into third_party and patch its Android build.gradle
# Usage: Run in PowerShell as administrator if necessary.

$packageName = "flutter_bluetooth_serial"

Write-Host "Diagnostics:"
Write-Host "USERPROFILE =" $env:USERPROFILE
Write-Host "LOCALAPPDATA =" $env:LOCALAPPDATA
Write-Host "APPDATA =" $env:APPDATA
Write-Host "Current directory:" (Get-Location)

# Common pub cache locations on Windows (plain strings)
$candidates = @(
    "$env:USERPROFILE\.pub-cache\hosted\pub.dev",
    "$env:LOCALAPPDATA\Pub\Cache\hosted\pub.dev",
    "$env:APPDATA\Pub\Cache\hosted\pub.dev"
)

$packageDir = $null
foreach ($pubCache in $candidates) {
    if (Test-Path $pubCache) {
        Write-Host "Searching in pub cache: $pubCache"
        $found = Get-ChildItem -Path $pubCache -Filter "$packageName*" -Directory -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        if ($found) { $packageDir = $found; break }
    } else {
        Write-Host "Pub cache path not present: $pubCache"
    }
}

if (-not $packageDir) {
    Write-Error "Package $packageName not found in any standard pub-cache location.\nPlease run 'flutter pub get' in your project first, or locate your pub cache and copy the package manually.\nChecked locations: $($candidates -join ', ')"
    exit 1
}

$src = $packageDir.FullName
$dst = Join-Path $PSScriptRoot $packageName

Write-Host "Copying from $src to $dst"
if (Test-Path $dst) { Remove-Item -Recurse -Force $dst }
Copy-Item -Recurse -Force -Path $src -Destination $dst

# Try to patch Android build file
$androidBuildGradle = Join-Path $dst "android\build.gradle"
if (-not (Test-Path $androidBuildGradle)) {
    $androidBuildGradle = Join-Path $dst "android\build.gradle.kts"
}

if (Test-Path $androidBuildGradle) {
    Write-Host "Patching $androidBuildGradle"
    $content = Get-Content $androidBuildGradle -Raw

    # Find package in AndroidManifest to use as namespace
    $manifest = Join-Path $dst "android\src\main\AndroidManifest.xml"
    $namespace = ""
    if (Test-Path $manifest) {
        $m = Select-String -Path $manifest -Pattern 'package\s*=\s*"([^"]+)"' -AllMatches
        if ($m) { $namespace = $m.Matches[0].Groups[1].Value }
    }

    if (-not $namespace) {
        Write-Warning "Could not determine namespace from AndroidManifest; using fallback 'com.example.flutter_bluetooth_serial'"
        $namespace = "com.example.flutter_bluetooth_serial"
    }

    if ($content -match "android\s*\{") {
        # naive insert: add namespace line after android { in groovy or kotlin dsl
        $newContent = $content -creplace "android\s*\{", "android {\n    namespace '$namespace'"
        Set-Content -Path $androidBuildGradle -Value $newContent -Force
        Write-Host "Injected namespace: $namespace"
    } else {
        Write-Warning "No android { block found in $androidBuildGradle"
    }
} else {
    Write-Warning "No android build file found to patch at expected locations"
}

Write-Host "Vendor copy complete. Update pubspec.yaml if necessary and run 'flutter pub get' in project root."