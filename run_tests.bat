@echo off

:: Check if build directory exists
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

:: Navigate to build directory
cd build

:: Configure with CMake
echo Configuring project with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Debug
if %ERRORLEVEL% neq 0 (
    echo Error: CMake configuration failed!
    pause
    exit /b 1
)

:: Build the project
echo.
echo Building project and tests...
cmake --build . --config Debug
if %ERRORLEVEL% neq 0 (
    echo Error: Build failed!
    pause
    exit /b 1
)

:: Run the tests
echo.
echo ===============================================
echo              Running Tests
echo ===============================================
echo.

:: Run all tests using our custom target
cmake --build . --target run_all_tests --config Debug
if %ERRORLEVEL% neq 0 (
    echo.
    echo Some tests failed! Check the output above.
) else (
    echo.
    echo ===============================================
    echo           All Tests Passed!
    echo ===============================================
)

echo.
echo Press any key to exit...
pause >nul 