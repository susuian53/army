@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
msbuild Army.vcxproj /p:Configuration=Debug /p:Platform=x64
