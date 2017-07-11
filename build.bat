@echo Off
set config=%1
if "%config%" == "" (
   set config=Release
)
 
set version=1.0.0
if not "%PackageVersion%" == "" (
   set version=%PackageVersion%
)

set nuget=
if "%nuget%" == "" (
	set nuget=nuget
)

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
"%programfiles(x86)%\MSBuild\12.0\Bin\MSBuild.exe" fix8-vc140.sln /p:Configuration="%config%";Platform="x64" /m /v:M /fl /flp:LogFile=msbuild.log;Verbosity=diag /nr:false

mkdir Build
mkdir Build\native\bin\x64\v120\Release\Desktop
mkdir Build\native\lib\x64\v120\Release\Desktop
mkdir Build\tools

%nuget% pack "fix8.eze.nuspec" -NoPackageAnalysis -verbosity detailed -o Build -Version %version% -p Configuration="%config%"
