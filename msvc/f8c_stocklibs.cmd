@echo off

cd ..\stocklib
set Configuration=%2
set Platform=%3
set OutDir=%4
set F8C=not_set
if %Configuration% EQU Debug (
	set F8C=%OutDir%\f8cd
) else (
	set F8C=%OutDir%\f8c
)

for %%f in (FIX40 FIX41 FIX42 FIX43 FIX44) do (
	call :build %%f 0 %1
)
for %%f in (FIX50 FIX50SP1 FIX50SP2) do (
	call :build %%f 1 %1
)
cd ..\msvc
goto :eof

:build
	set fixver=%~1
	set xmlt=%~2
	set act=%~3
	set xml_schema="..\schema\%fixver%.xml"
	set xmlt_schema="..\schema\FIXT11.xml"
	set lost=0
	for %%i in (%fixver%_classes.cpp %fixver%_classes.hpp %fixver%_router.hpp %fixver%_traits.cpp %fixver%_types.cpp %fixver%_types.hpp) do (
		if not exist %%i (
			set lost=1  
			echo %%i   NOT exist   
		) else (
			echo %%i   exist
		)
	)
	set needBuild=0
	set needClean=0
	echo going to do [%1]
	if %act% EQU clean (
		set needClean=1
		set needBuild=0
	)
	if %act% EQU rebuild (
		set needClean=1
		set needBuild=1
	)
	if %act% EQU build (
		if %lost% == 1 (
			set needClean=1
			set needBuild=1
		)
	)
	if %needClean% == 1 (
		echo ************going to clean %fixver% ************
		for %%i in (%fixver%_classes.cpp %fixver%_classes.hpp %fixver%_router.hpp %fixver%_traits.cpp %fixver%_types.cpp %fixver%_types.hpp) do (
			if exist %%i (			
				echo going to del %%i
				del %%i
			) 
		)
		echo ************  clean end %fixver% ************
	)

	if %needBuild% == 1 (
		echo ************going to generate %fixver% ************
		if %xmlt% == 1 (
			%F8C% --verbose --namespace %fixver% -Hprecomp.hpp %xml_schema% -p %fixver% --fixt %xmlt_schema%
		) else (
			%F8C% --verbose --namespace %fixver% -Hprecomp.hpp %xml_schema% -p %fixver%
		)
		echo ************  generate done %fixver% ************
	)
	goto :eof
