@echo off
set Mode=Build
if not '%1'=='' (
   set Mode=%1
)
echo '%Mode% Release x64'
call build.cmd %Mode% Release x64
echo '%Mode% Debug x64'
call build.cmd %Mode% Debug x64
echo '%Mode% Release Win32'
call build.cmd %Mode% Release Win32
echo '%Mode% Debug Win32'
call build.cmd %Mode% Debug Win32
