cd msvc

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
msbuild /t:Rebuild /p:Configuration=Release;Platform=x64 /m fix8-vc140.sln
goto end

:usage
echo "Use build.cmd Build|Rebuild|Clean Debug|Release x64|Win32"

:end

