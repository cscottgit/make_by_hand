@echo off
REM make sure to run this from developer command prompt

mkdir ..\build
pushd ..\build

REM - keep it quiet
del *.obj >nul
del *.exe >nul
del *.o >nul


cl -Zi -c ..\handmade\win32_handmade.cpp

link /DEBUG -out:win32_handmade.exe ^
	win32_handmade.obj ^
	user32.lib gdi32.lib XInput.lib

popd




