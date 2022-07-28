@echo off

pushd %~dp0\..\
call Tools\BuildSystem\PremakeBin\premake5.exe vs2022
popd

PAUSE