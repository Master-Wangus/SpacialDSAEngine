@echo off

REM CHECK IF 'build' FOLDER EXISTS; CREATE IF IT DOES NOT
if not exist build (
    mkdir build
)

REM BUILDING DEBUG
pushd build
cmake ..
cmake --build . --config Debug
popd

REM BUILDING RELEASE
if not exist build (
    mkdir build
)

pushd build
cmake ..
cmake --build . --config Release
popd
