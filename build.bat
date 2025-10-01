@ECHO OFF

set BUILD_PATH=%1

IF "%BUILD_PATH%"=="" (SET BUILD_PATH=BUILD)

echo Building under path: %BUILD_PATH%
echo:

mkdir %BUILD_PATH%

umk z2cr z2cr CLANGx64 -br +SSE2 %BUILD_PATH%\z2c.exe

IF %ERRORLEVEL% NEQ 0 (
  echo Could not build z2c. Exiting!
  EXIT /B %errno%
)

umk z2cr zide CLANGx64 -br +GUI,MT,SS2 %BUILD_PATH%\zide.exe

IF %ERRORLEVEL% NEQ 0 (
  echo Could not build zide. Exiting!
  EXIT /B %errno%
)

xcopy /k /h /y /q LICENSE %BUILD_PATH%\
xcopy /k /h /y /q README.md %BUILD_PATH%\
xcopy /k /h /y /q detect.bat %BUILD_PATH%\
xcopy /k /h /y /q build.bat %BUILD_PATH%\

if exist %BUILD_PATH%\codegen\ rmdir /s /q %BUILD_PATH%\codegen
xcopy /e /k /h /i codegen %BUILD_PATH%\codegen

if exist %BUILD_PATH%\licenses\ rmdir /s /q %BUILD_PATH%\licenses
xcopy /e /k /h /i licenses %BUILD_PATH%\licenses

if exist %BUILD_PATH%\tests\ rmdir /s /q %BUILD_PATH%\tests
xcopy /e /k /h /i  src\z2cr\tests %BUILD_PATH%\tests

if exist %BUILD_PATH%\source\stdlib\ rmdir /s /q %BUILD_PATH%\source\stdlib
xcopy /e /k /h /i  source\stdlib %BUILD_PATH%\source\stdlib

if exist %BUILD_PATH%\source\samples\ rmdir /s /q %BUILD_PATH%\source\samples
xcopy /e /k /h /i  source\samples %BUILD_PATH%\source\samples