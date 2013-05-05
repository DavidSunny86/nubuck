Compiling Nubuck

1. Requirements

Nubuck runs on Windows only. For building the project from source you will need the compiler
that ships with Visual Studio.
Nubuck is being developed on Windows 7 using Visual Studio 2010 which is the only tested platform
at the moment.

Nubuck uses version 4.8.1 of the Qt libaray. See http://qt-project.org/downloads.
Newer versions of Qt 4 are probably fine, too. Do not use Qt 5.

LEDA must be build as a DLL.

2. Building from source using Cygwin

Open a text editor and paste the following lines:

call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86

call "C:\Libraries\QtSDK\Desktop\Qt\4.8.1\msvc2010\bin\qtenv2.bat"
start "Cygwin" "C:\Program Files (x86)\Cygwin\bin\mintty.exe" -i /Cygwin-Terminal.ico -

Paths must be changed depending on your installation. Save the file as devcon.bat and
close the editor. Start a new Cygwin session by double clicking on devcon.bat.

Change into your LEDA root directory. Type "make Nubuck". For building the dummy client
change into the directory "nubuck_client" and type "make".

3. Running Nubuck applications

In order to run Nubuck applications the environment variable LEDA_DIR must be set
in Windows, pointing to your LEDA root directory.
LEDA_DIR must use the Windows path format (using '\' not '/' to separate directories)
and must omit a trailing separator. Eg. C:\Libraries\LEDA\LEDA-6.4
