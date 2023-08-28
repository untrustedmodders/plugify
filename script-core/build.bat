@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

IF NOT EXIST "../addons/wizard/bin" MKDIR "../assets/wizard/bin"

SET "files="
FOR /f "delims=" %%i IN ('dir /b /s ".\wizard\wizard\src\*.cs"') DO (
    SET files=!files! %%i
)

csc -target:library -out:../addons/wizard/bin/Wizard.dll %files%