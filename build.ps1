# Check if build directory exists
if (!(Test-Path -Path "build")) {
    Write-Host "Configuring CMake project..."
    cmake -B build -G "Visual Studio 17 2022" -A x64 .
}

# Build the project
Write-Host "Building project..."
cmake --build build --config Release

Write-Host "Build completed."