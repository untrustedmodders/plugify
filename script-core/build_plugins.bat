@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

IF NOT EXIST "../addons/wizard/plugins" MKDIR "../assets/wizard/plugins"

SET /a count=0
FOR /r ".\Plugins\Plugins\src\" %%i in (*.cs) DO (
	csc -target:library -reference:../addons/wizard/bin/wizard.dll -out:../addons/wizard/plugins/%%~ni.dll %%i
)