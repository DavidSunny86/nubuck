set CFLAGS= ^
 /I%LEDA_DIR%\incl ^
 /DLEDA_DLL /DLEDA_MULTI_THREAD ^
 /EHsc /MDd
set LFLAGS=/LIBPATH:"%LEDA_DIR%" leda.lib nubuck.lib 

cl /nologo convex_hull_gui.cpp %CFLAGS% /link %LFLAGS%
