@echo off

cd ..\test

set lost=0
for %%i in (Myfix_classes.cpp 
			Myfix_classes.hpp 
			Myfix_router.hpp 
			Myfix_traits.cpp 
			Myfix_types.cpp 
			Myfix_types.hpp
			Perf_classes.cpp 
			Perf_classes.hpp 
			Perf_router.hpp 
			Perf_traits.cpp 
			Perf_types.cpp 
			Perf_types.hpp) do (

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

if %1 EQU clean (
	set needClean=1
	set needBuild=0
)

if %1 EQU rebuild (
	set needClean=1
	set needBuild=1
)

if %1 EQU build (
	if %lost% == 1 (
		set needClean=1
		set needBuild=1
	)
)

if %needClean% == 1 (

	echo ************going to clean************
	for %%i in (Myfix_classes.cpp 
			Myfix_classes.hpp 
			Myfix_router.hpp 
			Myfix_traits.cpp 
			Myfix_types.cpp 
			Myfix_types.hpp
			Perf_classes.cpp 
			Perf_classes.hpp 
			Perf_router.hpp 
			Perf_traits.cpp 
			Perf_types.cpp 
			Perf_types.hpp) do (

		if exist %%i (			
			echo going to del %%i
			del %%i
		) 
	)
	echo ************  clean end  ************
)

if %needBuild% == 1 (

echo ************going to generate************

		if %2 EQU Debug (
			..\Debug\f8cd -rVn TEX ..\schema\FIX50SP2.xml -x ..\schema\FIXT11.xml
			..\Debug\f8cd -sVp Perf -n TEX ..\schema\FIX42PERF.xml
		) else (
			..\release\f8c -rVn TEX ..\schema\FIX50SP2.xml -x ..\schema\FIXT11.xml
			..\release\f8c -sVp Perf -n TEX ..\schema\FIX42PERF.xml
		)

echo ************  generate done  ************

)


