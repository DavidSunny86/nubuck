UIHEADERNAME	= ./makeutils/uiheadername
MOCNAME			= ./makeutils/mocname
OBJNAME			= ./makeutils/objname
WINDIR			= ./makeutils/windir

UIGEN_DIR 		= UI/GeneratedFiles
MOCGEN_DIR		= UI/moc
FORMS_DIR		= UI/forms
BUILD_DIR		= build
TARGET			= nubuck
OUTPUT_DIR		= ../../.

QTINCL = \
-I"$(QTDIR)\include" \
-I"$(QTDIR)\include\QtCore" \
-I"$(QTDIR)\include\QtGui" \
-I"$(QTDIR)\include\QtOpenGL"

INCL = $(QTINCL) \
-I"." \
-I$(UIGEN_DIR) \
-I"..\..\incl"

QTLIBS = \
"$(QTDIR)\lib\qtmaind.lib" \
"$(QTDIR)\lib\QtGuid4.lib" \
"$(QTDIR)\lib\QtCored4.lib" \
"$(QTDIR)\lib\QtOpenGLd4.lib"

STDLIBS = \
"kernel32.lib" \
"user32.lib" \
"gdi32.lib" \
"winspool.lib" \
"comdlg32.lib" \
"advapi32.lib" \
"shell32.lib" \
"ole32.lib" \
"oleaut32.lib" \
"uuid.lib" \
"odbc32.lib" \
"odbccp32.lib"

LIBS = $(QTLIBS) $(STDLIBS) \
"..\..\leda.lib" \
"opengl32.lib" "glu32.lib"

DEF = -D
DEFINES = \
$(DEF)NUBUCK_LIB \
$(DEF)PARANOID \
$(DEF)UNICODE \
$(DEF)QT_DLL \
$(DEF)QT_CORE_LIB  \
$(DEF)QT_OPENGL_LIB  \
$(DEF)QT_GUI_LIB \
$(DEF)GLEW_BUILD \
$(DEF)LEDA_DLL

# /wdC4005 disables warning 'macro redefinition (of MAXINT)'
CC = cl /nologo /wd4005 /EHsc /MDd $(DEFINES) $(INCL) /c

LD = cl /nologo /LD $(LIBS)

UIGENFILES	= $(shell find $(FORMS_DIR) -iname '*.ui' -exec $(UIHEADERNAME) $(UIGEN_DIR) {} \;)
MOCGENFILES	= $(shell find . -iname '*.h' -exec grep -l Q_OBJECT {} \; | xargs $(MOCNAME) $(MOCGEN_DIR))
OBJS 		= $(shell find . \( -iname '*.cpp' -or -iname '*.c' \) -exec $(OBJNAME) $(BUILD_DIR) {} \;)

# used to enable one-pass compilation
MOCOBJS		= $(shell $(OBJNAME) $(BUILD_DIR) $(MOCGENFILES))

all: $(BUILD_DIR) $(UIGEN_DIR) $(MOCGEN_DIR) $(UIGENFILES) $(MOCGENFILES) $(OBJS) $(MOCOBJS) $(TARGET)

$(BUILD_DIR):
	if [ ! -d $(BUILD_DIR) ]; then mkdir $(BUILD_DIR); fi

$(UIGEN_DIR):
	if [ ! -d $(UIGEN_DIR) ]; then mkdir $(UIGEN_DIR); fi

$(MOCGEN_DIR):
	if [ ! -d $(MOCGEN_DIR) ]; then mkdir $(MOCGEN_DIR); fi
	@echo $(OBJS)

$(UIGEN_DIR)/ui_%.h: $(FORMS_DIR)/%.ui
	@echo "===== COMPILING UI FILE $< ====="
	uic $< > $@

$(MOCGEN_DIR)/%_moc.cpp: %.h
	@echo "===== MOCING FILE $< ====="
	mkdir -p $(shell dirname $@)
	moc $(DEFINES) $(INCL) $< > $@

$(BUILD_DIR)/%.obj: %.cpp
	@echo "===== COMPILING SOURCE FILE $< ====="
	mkdir -p $(shell dirname $@)
	$(CC) /Fo$(shell $(WINDIR) $@) $< 

$(BUILD_DIR)/%.obj: %.c
	@echo "===== COMPILING SOURCE FILE $< ====="
	mkdir -p $(shell dirname $@)
	$(CC) /Fo$(shell $(WINDIR) $@) $< 

$(TARGET): $(UIGENFILES) $(MOCGENFILES) $(OBJS)
	@echo "===== LINKING ====="
	$(LD) /Fe$(TARGET) $(OBJS)
	@echo "===== MOVING .dll and .lib TO OUTPUT DIR ====="
	mv $(TARGET).dll $(OUTPUT_DIR)
	mv $(TARGET).lib $(OUTPUT_DIR)

.PHONY: clean
clean:
	rm -rf $(UIGEN_DIR) $(MOCGEN_DIR) $(BUILD_DIR)
