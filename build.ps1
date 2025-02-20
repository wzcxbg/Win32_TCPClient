# Check if build directory exists
if (!(Test-Path -Path "build")) {
    Write-Host "Configuring CMake project..."
    cmake -B build -G "Visual Studio 17 2022" -A x64 .
}

# Build the project
Write-Host "Building project..."
cmake --build build --config Release

Write-Host "Build completed."

# Launch the executable
$exePath = "build\Release\Win32TCPClient.exe"
if (Test-Path $exePath) {
    Write-Host "Launching application..."
    Start-Process $exePath
} else {
    Write-Host "Error: Could not find the executable at $exePath"
}