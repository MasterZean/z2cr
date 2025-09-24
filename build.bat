@ECHO OFF

set BUILD_PATH=%1

IF "%BUILD_PATH%"=="" (SET BUILD_PATH=BUILD)

echo Building under path: %BUILD_PATH%
echo:

mkdir %BUILD_PATH%

umk z2cr z2cr CLANGx64 -br +SSE2 %BUILD_PATH%

IF %ERRORLEVEL% NEQ 0 (
  echo Could not build z2c. Exiting!
  EXIT /B %errno%
)

umk z2cr zide CLANGx64 -br +GUI,MT,SS2 %BUILD_PATH%

IF %ERRORLEVEL% NEQ 0 (
  echo Could not build zide. Exiting!
  EXIT /B %errno%
)

copy /b LICENSE %BUILD_PATH%
copy /b README.md %BUILD_PATH%
copy /b detect.bat %BUILD_PATH%

rmdir /s /q %BUILD_PATH%\codegen
xcopy /e /k /h /i codegen %BUILD_PATH%\codegen

rmdir /s /q %BUILD_PATH%\licenses
xcopy /e /k /h /i licenses %BUILD_PATH%\licenses

rmdir /s /q %BUILD_PATH%\tests
xcopy /e /k /h /i  src\z2cr\tests %BUILD_PATH%\tests

rmdir /s /q %BUILD_PATH%\src\stdlib
xcopy /e /k /h /i  src\stdlib %BUILD_PATH%\src\stdlib

rmdir /s /q %BUILD_PATH%\src\samples
xcopy /e /k /h /i  src\samples %BUILD_PATH%\src\samples