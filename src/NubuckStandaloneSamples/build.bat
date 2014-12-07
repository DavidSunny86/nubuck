set QTINCL= ^
 /I"%QTDIR%\include" ^
 /I"%QTDIR%\include\QtCore" ^
 /I"%QTDIR%\include\QtGui" ^
 /I"%QTDIR%\include\QtOpenGL"

set QTLIBS= ^
 "%QTDIR%\lib\qtmaind.lib" ^
 "%QTDIR%\lib\QtGuid4.lib" ^
 "%QTDIR%\lib\QtCored4.lib" ^
 "%QTDIR%\lib\QtOpenGLd4.lib"

set CFLAGS= ^
 /I%LEDA_DIR%\incl %QTINCL% ^
 /DLEDA_DLL /DLEDA_MULTI_THREAD ^
 /EHsc /MDd
set LFLAGS=/LIBPATH:"%LEDA_DIR%" leda.lib nubuck.lib %QTLIBS%

cl /nologo convex_hull.cpp %CFLAGS% /link %LFLAGS%
