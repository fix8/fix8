@echo off

cd ..\utests

set lost=0
for %%i in (utest_classes.cpp 
			utest_classes.hpp 
			utest_router.hpp 
			utest_traits.cpp 
			utest_types.cpp 
			utest_types.hpp) do (

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
	for %%i in (utest_classes.cpp 
			utest_classes.hpp 
			utest_router.hpp 
			utest_traits.cpp 
			utest_types.cpp 
			utest_types.hpp) do (

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
			..\debug\f8cd -Vp utest -n UTEST ..\schema\FIX42UTEST.xml
		) else (
			..\release\f8c -Vp utest -n UTEST ..\schema\FIX42UTEST.xml
		)

echo ************  generate done  ************

)


