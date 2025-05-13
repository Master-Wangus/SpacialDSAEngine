@echo off

REM CHECK IF 'build' FOLDER EXISTS; CREATE IF IT DOES NOT
if not exist build (
    mkdir build
)

REM BUILDING DEBUG
echo Building Debug configuration...
pushd build
cmake .. || goto :error
cmake --build . --config Debug || goto :error
popd

REM BUILDING RELEASE
echo Building Release configuration...
pushd build
cmake .. || goto :error
cmake --build . --config Release || goto :error
popd

echo Build completed successfully!
goto :end

:error
echo *** ERROR: An error occurred during the build process ***
pause
exit /b %errorlevel%

:end
pause
