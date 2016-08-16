@echo off
echo "启动编译脚本。本脚本只支持编译文件夹和代码文件夹处于同一路径下"

rd /s /q .\CMakeFiles
del cmake_install.cmake
del CMakeCache.txt

set "compileMode=%1%"
if "%compileMode%"=="release" (
	echo "编译64位release"
	cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 10 Win64" ../coding
) else (
	echo "编译64位debug"
	cmake -G "Visual Studio 10 Win64" ../coding
)

msbuild -m DocerSoSo.sln

rd /s /q .\CMakeFiles
del cmake_install.cmake
del CMakeCache.txt

if "%compileMode%"=="release" (
	echo "编译32位release"
	cmake -DCMAKE_BUILD_TYPE=Release ../coding
) else (
	echo "编译32位debug"
	cmake ../coding
)

msbuild -m DocerSoSo.sln
