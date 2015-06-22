cmake_minimum_required(VERSION 3.0)

project(nubuck)

find_package(Qt4 REQUIRED)

macro(stack_push v_stack s_item)
    list(APPEND ${v_stack} ${s_item})
endmacro()

macro(stack_pop v_stack)
    list(REMOVE_AT ${v_stack} -1)
endmacro()

# FUNCTION find_subdirs
# lists all child directories of s_curDir containing file s_file
# cnf. http://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory
function(find_subdirs v_ret s_curDir s_file)
    file(GLOB children RELATIVE ${s_curDir} "${s_curDir}/*")
    foreach(child ${children})
        # IS_DIRECTORY, EXISTS are well-defined for full paths only
        get_filename_component(absolutePath "${s_curDir}/${child}" ABSOLUTE)
        if(IS_DIRECTORY ${absolutePath})
            if(EXISTS "${absolutePath}/${s_file}")
                list(APPEND subdirs ${child})
            endif(EXISTS "${absolutePath}/${s_file}")
        endif(IS_DIRECTORY ${absolutePath})
    endforeach()
    set(${v_ret} ${subdirs} PARENT_SCOPE)
endfunction()

# MACRO join
# let v_dirs = a;b;c and s_sep = /.
# then join returns a/b/c
macro(join v_ret v_dirs s_sep)
    set(${v_ret} "")

    set(get_relative_path_v_dirs_cpy ${${v_dirs}})
    
    if(get_relative_path_v_dirs_cpy)
        list(GET get_relative_path_v_dirs_cpy 0 ${v_ret})
        list(REMOVE_AT get_relative_path_v_dirs_cpy 0)

        foreach(dir ${get_relative_path_v_dirs_cpy})
            set(get_relative_path_c_ret ${${v_ret}})
            set(${v_ret} "${get_relative_path_c_ret}${s_sep}${dir}")
        endforeach()
    endif(get_relative_path_v_dirs_cpy)
endmacro()

macro(get_current_path v_ret)
    set(get_current_path_dirs ${nubuckBasePath} ${nubuckDirs})
    join(${v_ret} get_current_path_dirs "/")
endmacro()

macro(get_current_scope v_ret)
    set(get_current_scope_dirs ${nubuckBaseScope} ${nubuckDirs})
    join(${v_ret} get_current_scope_dirs "\\\\")
endmacro()

# MACRO add_sources
# must be called exactly once for each target
macro(add_sources)
    set(add_sources_localSources "")
    set(add_sources_localHeaders "")
    set(add_sources_localForms "")
    set(add_sources_curPath "")
    set(add_sources_curScope "")

    message("relpath is ${nubuckDirs}")

    get_current_path(add_sources_curPath)
    get_current_scope(add_sources_curScope)

    foreach(source ${ARGN})
        if(${source} MATCHES ".*\\.cpp" OR ${source} MATCHES ".*\\.c")
            list(APPEND add_sources_localSources "${add_sources_curPath}/${source}")
            message("adding file ${add_sources_curPath}/${source} to sources")
        endif(${source} MATCHES ".*\\.cpp" OR ${source} MATCHES ".*\\.c")

        if(${source} MATCHES ".*\\.h")
            list(APPEND add_sources_localHeaders "${add_sources_curPath}/${source}")
            message("adding file ${add_sources_curPath}/${source} to headers")
        endif(${source} MATCHES ".*\\.h")

        if(${source} MATCHES ".*\\.ui")
            list(APPEND add_sources_localForms "${add_sources_curPath}/${source}")
            message("adding file ${add_sources_curPath}/${source} to forms")
        endif(${source} MATCHES ".*\\.ui")
    endforeach()

    list(APPEND nubuckSources ${add_sources_localSources})
    list(APPEND nubuckHeaders ${add_sources_localHeaders})
    list(APPEND nubuckForms ${add_sources_localForms})

    if(add_sources_curScope)
        source_group(${add_sources_curScope} FILES ${add_sources_localSources})
        source_group(${add_sources_curScope} FILES ${add_sources_localHeaders})
        source_group(${add_sources_curScope} FILES ${add_sources_localForms})
    endif(add_sources_curScope)
endmacro()

message("building nubuck since 2014")

# MACRO recurse
macro(recurse)
    set(recurse_subdirs "")
    set(recurse_curPath "")

    get_current_path(recurse_curPath)
    find_subdirs(recurse_subdirs ${recurse_curPath} "sources.cmake")

    foreach(subdir ${recurse_subdirs})
        get_current_path(recurse_curPath)
        stack_push(nubuckDirs ${subdir})
        include("${recurse_curPath}/${subdir}/sources.cmake")
        stack_pop(nubuckDirs)
    endforeach()
endmacro()

set(nubuckDirs "")

set(leda_dir "")
file(TO_CMAKE_PATH $ENV{LEDA_DIR} leda_dir)

set(nubuckBasePath ${CMAKE_CURRENT_SOURCE_DIR})
set(nubuckBaseScope "private")
include("sources.cmake") # top-level sources

set(nubuckBasePath "${leda_dir}/incl/Nubuck")
set(nubuckBaseScope "public")
include("${leda_dir}/incl/Nubuck/sources.cmake")

# dump all sources to console
# message("sources...")
# foreach(source ${nubuckSources})
#     message("${source}")
# endforeach()

# qt preprocessing
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# can't use autouic because .ui file location doesn't conform
# to cmake convention
message("nubuckForms = ${nubuckForms}")
qt4_wrap_ui(nubuckHeaders_ui ${nubuckForms})
message("nubuckHeaders_ui = ${nubuckHeaders_ui}")

qt4_add_resources(nubuckQtRes "${leda_dir}/res/nubuck.qrc")

source_group("qt_generated\\moc" REGULAR_EXPRESSION ".*_automoc\\.cpp")
source_group("qt_generated\\uic" REGULAR_EXPRESSION "ui_.*\\.h")
source_group("qt_generated\\rcc" REGULAR_EXPRESSION "qrc_.*\\.cxx")

include_directories("${leda_dir}/incl" "${leda_dir}/src/Nubuck")
link_directories("${leda_dir}")

# microprofiler
link_directories("microprofiler")
add_compile_options(/Gh /GH) # _penter, _pexit hooks

add_compile_options(/W1)

add_definitions(-DLEDA_DLL -DLEDA_MULTI_THREAD)
add_definitions(-DUNICODE)

# TARGET nubuck
add_library(nubuck SHARED ${nubuckSources} ${nubuckHeaders} ${nubuckHeaders_ui} ${nubuckQtRes})
target_link_libraries(nubuck leda opengl32 glu32 Qt4::QtGui Qt4::QtOpenGL Qt4::QtSvg)
target_link_libraries(nubuck micro-profiler)
set_target_properties(nubuck PROPERTIES 
    COMPILE_DEFINITIONS "NUBUCK_LIB;NUBUCK_MT;PARANOID;GLEW_BUILD")
set_target_properties(nubuck PROPERTIES
    COMPILE_FLAGS "/MP /Gm-") # multi-processor compilation is generally a good idea

# copy .dll to leda_dir
add_custom_command(TARGET nubuck POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "$<TARGET_FILE:nubuck>"
    "${leda_dir}/$<TARGET_FILE_NAME:nubuck>")

# copy .lib to leda_dir
add_custom_command(TARGET nubuck POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "$<TARGET_LINKER_FILE:nubuck>"
    "${leda_dir}/$<TARGET_LINKER_FILE_NAME:nubuck>")

# TARGET nubuck_standalone
add_executable(nubuck_standalone nubuck_standalone.cpp)
target_link_libraries(nubuck_standalone nubuck)

# nubuck operators

macro(add_operator s_operatorName)
    # source groups must be set in each source file
    source_group("qt_generated\\moc" REGULAR_EXPRESSION ".*_automoc\\.cpp")
    source_group("qt_generated\\uic" REGULAR_EXPRESSION "ui_.*\\.h")
    source_group("qt_generated\\rcc" REGULAR_EXPRESSION "qrc_.*\\.cxx")

    add_library(${s_operatorName} SHARED 
        ${nubuckSources} ${nubuckHeaders} ${nubuckHeaders_ui})
    target_link_libraries(${s_operatorName} leda nubuck)

    add_custom_command(TARGET ${s_operatorName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:${s_operatorName}>"
        "${leda_dir}/res/Operators/$<TARGET_FILE_NAME:${s_operatorName}>")
endmacro()

set(operatorsBasePath "${leda_dir}/src/NubuckOperators")

set(operatorDirs "")

find_subdirs(operatorDirs ${operatorsBasePath} "CMakeLists.txt")

set(operatorName "")

foreach(operatorDir ${operatorDirs})
    message("found operator ${operatorDir}")
    set(operatorName ${operatorDir})

    set(nubuckSources "")
    set(nubuckHeaders "")
    set(nubuckForms "")
    set(nubuckHeaders_ui "")

    set(nubuckBasePath "${operatorsBasePath}/${operatorDir}")
    set(nubuckBaseScope "sources")

    add_subdirectory("${operatorsBasePath}/${operatorDir}" ${operatorDir})
endforeach()
