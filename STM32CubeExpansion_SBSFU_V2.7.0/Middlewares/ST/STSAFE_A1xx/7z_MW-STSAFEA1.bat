set archive=en.STSW-SAFEA1-MW
set folder=mw
set version=3.3.7

cls
cd..

ROBOCOPY %folder% %folder%_V%version% /E /NS /NC /NFL /NDL /XD %cd%%folder%\.git /XF *.bat *.pdsc

"C:\Program Files\7-Zip\7z.exe" a -tzip -mx=9 -mm=LZMA %archive%.zip %folder%_V%version% ^
                               -x!%folder%_V%version%\.git ^
                               -x!%folder%_V%version%\_htmresc\mini-st.css ^
                               -x!%folder%_V%version%\CoreModules\Doxygen ^
							   -x!%folder%_V%version%\STSAFE-A1xx_Middleware.chw ^
                               -x!%folder%_V%version%\STSAFE-A_Release_Notes.md ^
                               -x!%folder%_V%version%\STSAFE-A1xx_Middleware_DB.docx ^
                               -x!%folder%_V%version%\STSAFE-A1xx_Middleware_UM.docx
							   
rmdir /S /Q %folder%_V%version%
rem pause
