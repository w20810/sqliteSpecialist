@echo off
echo "��������ű������ű�ֻ֧�ֱ����ļ��кʹ����ļ��д���ͬһ·����"

rd /s /q .\CMakeFiles
del cmake_install.cmake
del CMakeCache.txt

set "compileMode=%1%"
if "%compileMode%"=="release" (
	echo "����64λrelease"
	cmake -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 10 Win64" ../coding
) else (
	echo "����64λdebug"
	cmake -G "Visual Studio 10 Win64" ../coding
)

msbuild -m DocerSoSo.sln

rd /s /q .\CMakeFiles
del cmake_install.cmake
del CMakeCache.txt

if "%compileMode%"=="release" (
	echo "����32λrelease"
	cmake -DCMAKE_BUILD_TYPE=Release ../coding
) else (
	echo "����32λdebug"
	cmake ../coding
)

msbuild -m DocerSoSo.sln
