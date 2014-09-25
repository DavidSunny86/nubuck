cmake_minimum_required(VERSION 3.0)

project(nubuck_widgets)

find_package(Qt4 REQUIRED)

set(leda_dir "")
file(TO_CMAKE_PATH $ENV{LEDA_DIR} leda_dir)

# qt preprocessing
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt4_add_resources(nubuckQtRes "${leda_dir}/res/nubuck.qrc")

include_directories("${leda_dir}/incl" "${leda_dir}/src/Nubuck")
link_directories("${leda_dir}")

add_compile_options(/W1)

add_definitions(-DLEDA_DLL -DLEDA_MULTI_THREAD)
add_definitions(-DUNICODE)

set(nubuckSources "nbw_spinbox_plugin.cpp;../nbw_spinbox/nbw_spinbox.cpp")
set(nubuckHeaders 
    "nbw_spinbox_plugin.h"
    "${leda_dir}/incl/Nubuck/UI/nbw_spinbox.h"
    "../nbw_spinbox/nbw_spinbox_controls.h")
set(nubuckForms "")
set(nubuckHeaders_ui "")

add_library(nubuck_widgets SHARED ${nubuckSources} ${nubuckHeaders} ${nubuckHeaders_ui} ${nubuckQtRes})
target_link_libraries(nubuck_widgets leda Qt4::QtGui Qt4::QtOpenGL Qt4::QtSvg Qt4::QtDesigner)
set_target_properties(nubuck_widgets PROPERTIES
    COMPILE_DEFINITIONS "NUBUCK_BUILD_DESIGNER_PLUGIN;QT_PLUGIN;QT_SHARED;QDESIGNER_EXPORT_WIDGETS")

install(TARGETS nubuck_widgets
    DESTINATION ${QT_PLUGINS_DIR}/designer)
