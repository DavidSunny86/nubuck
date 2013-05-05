INCL = -I"..\incl"

LIBS = "..\leda.lib" "..\nubuck.lib"

DEF = -D
DEFINES = $(DEF)LEDA_DLL

# /wdC4005 disables warning 'macro redefinition (of MAXINT)'
CC = cl /nologo /wd4005 /EHsc /MDd $(DEFINES) $(INCL) /c

LD = cl /nologo $(LIBS)

OBJS 	= main.obj
TARGET	= nubuck_client

$(TARGET): $(OBJS)
	@echo "===== LINKING ====="
	$(LD) /Fe$(TARGET) $(OBJS) 

%.obj: %.cpp
	@echo "===== COMPILING SOURCE FILE $< ====="
	$(CC) /Fo$@ $<

.PHONY: clean
clean:
	rm -rf $(OBJS) $(TARGET)
