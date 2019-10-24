@echo off
if '%1'=='' goto usage
if '%2'=='' goto usage
if '%3'=='' goto usage
if '%4'=='' goto usage

echo "%1 %2 %3 %4"
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" %4
msbuild /t:%1 /p:Configuration=%2;Platform=%3 /m fix8-vc142.sln
goto end

:usage
echo "Use build.cmd Build|Rebuild|Clean Debug|Release x64|Win32 x64|x86"

:end
