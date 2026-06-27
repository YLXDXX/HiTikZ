cmake_minimum_required(VERSION 3.16)
project(TikzManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Pdf PdfWidgets)

include(FetchContent)
FetchContent_Declare(
    qhotkey
    GIT_REPOSITORY https://github.com/Skycoder42/QHotkey.git
    GIT_TAG master
)
set(QHOTKEY_EXAMPLES OFF CACHE BOOL "")
set(QHOTKEY_INSTALL OFF CACHE BOOL "")
set(QT_DEFAULT_MAJOR_VERSION 6 CACHE STRING "")

option(WITH_QHOTKEY "Build with global hotkey support" ON)

if(WITH_QHOTKEY)
    FetchContent_MakeAvailable(qhotkey)
endif()

add_executable(TikzManager
    src/main.cpp
    src/mainwindow.cpp
    src/snippet_manager.cpp
    src/latex_compiler.cpp
    src/code_editor.cpp
    src/settings_dialog.cpp
    src/search_panel.cpp
    src/snippet_properties_dialog.cpp
)

target_link_libraries(TikzManager PRIVATE Qt6::Widgets Qt6::Pdf Qt6::PdfWidgets)

target_compile_definitions(TikzManager PRIVATE
    RESOURCE_DIR=${CMAKE_SOURCE_DIR}/resources
)

if(WITH_QHOTKEY AND TARGET qhotkey)
    target_link_libraries(TikzManager PRIVATE qhotkey)
    target_compile_definitions(TikzManager PRIVATE HAS_QHOTKEY)
endif()

enable_testing()

add_executable(test_snippet_manager
    tests/test_snippet_manager.cpp
    src/snippet_manager.cpp
)
target_include_directories(test_snippet_manager PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_snippet_manager PRIVATE Qt6::Core)
add_test(NAME SnippetManager COMMAND test_snippet_manager)

add_executable(test_latex_compiler
    tests/test_latex_compiler.cpp
    src/latex_compiler.cpp
)
target_include_directories(test_latex_compiler PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_latex_compiler PRIVATE Qt6::Core)
add_test(NAME LatexCompiler COMMAND test_latex_compiler)

add_executable(test_search
    tests/test_search.cpp
    src/snippet_manager.cpp
)
target_include_directories(test_search PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_search PRIVATE Qt6::Core)
add_test(NAME Search COMMAND test_search)
