@echo off
REM You can download VSWHERE.EXE from https://github.com/microsoft/vswhere

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -legacy -property resolvedInstallationPath`) do (
  set VSinstallDir=%%i
)

set VSCMD_ARG_HOST_ARCH=x64
set VSCMD_ARG_TGT_ARCH=x64

REM This actually detects the SDKs that you have installed on your system and picks the default one
call "%VSinstallDir%\Common7\Tools\vsdevcmd\core\winsdk.bat"

echo WindowsSDKVersion=%WindowsSDKVersion%
echo WindowsSDKLibVersion=%WindowsSDKLibVersion%
echo UCRTVersion=%UCRTVersion%
echo WindowsSdkDir=%WindowsSdkDir%
echo UniversalCRTSdkDir=%UniversalCRTSdkDir%
echo WindowsSdkBinPath=%WindowsSdkBinPath%
echo WindowsSdkVerBinPath=%WindowsSdkVerBinPath%
echo WindowsLibPath=%WindowsLibPath%