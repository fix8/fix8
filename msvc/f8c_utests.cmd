@echo off

cd ..\utests

set XML_42_UTEST_SCHEMA="..\schema\FIX42UTEST.xml"
set EXTRA_FIELDS="<field number='9999' name='SampleUserField'  type='STRING' messages='NewOrderSingle:N ExecutionReport:N OrderCancelRequest:Y' /><field number='9991' name='SampleUserField2' type='STRING' messages='NewOrderSingle:N ExecutionReport:N OrderCancelRequest:Y' />"
set Configuration=%2
set Platform=%3
set OutDir=%4
set F8C=not_set
if %Configuration% EQU Debug (
	set F8C=%OutDir%\f8cd
) else (
	set F8C=%OutDir%\f8c
)

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
	    %F8C% "-sVputest" "-nUTEST" "-Hprecomp.hpp" %XML_42_UTEST_SCHEMA% -F %EXTRA_FIELDS%

echo ************  generate done  ************

)


