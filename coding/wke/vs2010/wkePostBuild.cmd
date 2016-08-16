mkdir 2>NUL "..\demo\%CONFIGURATIONNAME%\bin"
mkdir 2>NUL "..\demo\%CONFIGURATIONNAME%\libwke"

xcopy /D /F /R /Y "%TARGETPATH%" "..\demo\%CONFIGURATIONNAME%\bin\"
xcopy /D /F /R /Y "%CONFIGURATIONBUILDDIR%\lib\wke.lib" "..\demo\%CONFIGURATIONNAME%\libwke\"
xcopy /D /F /R /Y "..\wke\wke.h" "..\..\..\shell\uilib\control\wke\"