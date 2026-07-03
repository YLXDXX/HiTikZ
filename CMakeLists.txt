cmake_minimum_required(VERSION 3.16)
project(TikzManager VERSION 1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Pdf PdfWidgets Concurrent)

option(WITH_KGLOBALACCEL "Build with KDE KGlobalAccel support" ON)
option(WITH_QHOTKEY "Build with global hotkey support" ON)

if(WITH_KGLOBALACCEL)
    find_package(KF6GlobalAccel QUIET)
    if(KF6GlobalAccel_FOUND)
        set(HAS_KGLOBALACCEL ON)
        message(STATUS "KF6GlobalAccel found — using KDE global shortcuts")
    else()
        set(HAS_KGLOBALACCEL OFF)
        message(STATUS "KF6GlobalAccel not found — will fallback to QHotkey")
    endif()
endif()

if(NOT HAS_KGLOBALACCEL AND WITH_QHOTKEY)
    include(FetchContent)
    FetchContent_Declare(
        qhotkey
        GIT_REPOSITORY https://github.com/Skycoder42/QHotkey.git
        GIT_TAG master
    )
    set(QHOTKEY_EXAMPLES OFF CACHE BOOL "")
    set(QHOTKEY_INSTALL OFF CACHE BOOL "")
    set(QT_DEFAULT_MAJOR_VERSION 6 CACHE STRING "")
    FetchContent_MakeAvailable(qhotkey)
endif()

add_executable(TikzManager
    src/main.cpp
    src/mainwindow.cpp
    src/snippet_manager.cpp
    src/latex_compiler.cpp
    src/code_editor.cpp
    src/tikz_highlighter.cpp
    src/tikz_completer.cpp
    src/settings_dialog.cpp
    src/search_panel.cpp
    src/snippet_properties_dialog.cpp
    src/kde_global_shortcut.cpp
    src/pdf_preview_widget.cpp
    src/flow_layout.cpp
)

target_link_libraries(TikzManager PRIVATE Qt6::Widgets Qt6::Pdf Qt6::PdfWidgets Qt6::Concurrent)

target_compile_definitions(TikzManager PRIVATE
    RESOURCE_DIR=${CMAKE_SOURCE_DIR}/resources
)

if(HAS_KGLOBALACCEL)
    target_link_libraries(TikzManager PRIVATE KF6::GlobalAccel)
    target_compile_definitions(TikzManager PRIVATE HAS_KGLOBALACCEL)
endif()

if(NOT HAS_KGLOBALACCEL AND WITH_QHOTKEY AND TARGET qhotkey)
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

add_executable(test_packages_libraries
    tests/test_packages_libraries.cpp
    src/latex_compiler.cpp
    src/snippet_manager.cpp
)
target_include_directories(test_packages_libraries PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_packages_libraries PRIVATE Qt6::Core)
add_test(NAME PackagesLibraries COMMAND test_packages_libraries)

add_executable(test_highlighter_regex
    tests/test_highlighter_regex.cpp
)
target_link_libraries(test_highlighter_regex PRIVATE Qt6::Core)
add_test(NAME HighlighterRegex COMMAND test_highlighter_regex)

add_executable(test_multitab
    tests/test_multitab.cpp
    src/mainwindow.cpp
    src/code_editor.cpp
    src/tikz_highlighter.cpp
    src/tikz_completer.cpp
    src/settings_dialog.cpp
    src/search_panel.cpp
    src/snippet_properties_dialog.cpp
    src/kde_global_shortcut.cpp
    src/pdf_preview_widget.cpp
    src/flow_layout.cpp
    src/snippet_manager.cpp
    src/latex_compiler.cpp
)
target_include_directories(test_multitab PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_multitab PRIVATE Qt6::Widgets Qt6::Pdf Qt6::PdfWidgets Qt6::Concurrent)
target_compile_definitions(test_multitab PRIVATE
    RESOURCE_DIR=${CMAKE_SOURCE_DIR}/resources
)
add_test(NAME MultiTab COMMAND test_multitab)

add_executable(test_draft_recovery
    tests/test_draft_recovery.cpp
)
target_link_libraries(test_draft_recovery PRIVATE Qt6::Core)
add_test(NAME DraftRecovery COMMAND test_draft_recovery)

add_executable(test_tex_import
    tests/test_tex_import.cpp
)
target_link_libraries(test_tex_import PRIVATE Qt6::Core)
add_test(NAME TexImport COMMAND test_tex_import)

add_executable(test_params
    tests/test_params.cpp
)
target_link_libraries(test_params PRIVATE Qt6::Core)
add_test(NAME Params COMMAND test_params)

add_executable(test_fixes
    tests/test_fixes.cpp
)
target_link_libraries(test_fixes PRIVATE Qt6::Core)
add_test(NAME Fixes COMMAND test_fixes)
