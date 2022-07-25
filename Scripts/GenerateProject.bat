@echo off

pushd %~dp0\..\
call tools\BuildSystem\bin\premake5.exe vs2022
popd

PAUSE