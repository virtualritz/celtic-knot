@ECHO OFF
IF "%~1"=="" ECHO Error: Must specify a version number as a parameter. E.g. "buildSetup 1.0.0"
IF "%~1"=="" GOTO End
SET productversion=%1

cd windows\wixui

REM Create the msi installer user interface
ECHO Creating MSI installer user interface

candle.exe knot\WixUI_Knot.wxs *.wxs

lit.exe -out WixUI_Knot.wixlib *.wixobj

cd ..\..

REM Create the MSI installer
ECHO Creating MSI installer

candle windows\knot.wxs -out windows\knot.wixobj

light.exe -out knot3d-%productversion%-win32-bin.msi windows\knot.wixobj "windows\wixui\WixUI_Knot.wixlib" -loc "windows\wixui\WixUI_en-us.wxl"

REM Create the BootStrapper installer
ECHO Creating BootStrapper installer

candle windows\bootstrap\bootstrap.wxs -out windows\bootstrap\bootstrap.wixobj

light -out Knot3DSetup-%productversion%-win32-bin.exe -ext WixBalExtension windows\bootstrap\bootstrap.wixobj


:End

