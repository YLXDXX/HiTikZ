cmake_minimum_required(VERSION 3.16)
project(TikzManager VERSION 0.1 LANGUAGES CXX)

# Single source of truth for the version number is the project() call above.
# It is exposed to the C++ code as APP_VERSION (see target_compile_definitions).

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Follow the GNU/Linux directory conventions. When the user does not pass
# -DCMAKE_INSTALL_PREFIX, self-built software installs under /usr/local.
include(GNUInstallDirs)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local"
        CACHE PATH "Install prefix" FORCE)
endif()

# Name of the installed program / .desktop / icon (all lowercase).
set(APP_ID hitikz)

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

# Main window sources (split into modules for maintainability)
set(MAINWINDOW_SOURCES
    src/mainwindow.cpp
    src/mainwindow_tabs.cpp
    src/mainwindow_compile.cpp
    src/mainwindow_params.cpp
    src/mainwindow_shortcuts.cpp
    src/mainwindow_drafts.cpp
)

# Search panel sources (split into modules for maintainability)
set(SEARCH_PANEL_SOURCES
    src/search_panel.cpp
    src/search_panel_tags.cpp
    src/search_panel_menus.cpp
)

# TikZ completer sources (split into modules for maintainability)
set(TIKZ_COMPLETER_SOURCES
    src/tikz_completer.cpp
    src/tikz_completer_context.cpp
    src/tikz_completer_models.cpp
)

# Snippet manager sources (split into modules for maintainability)
set(SNIPPET_MANAGER_SOURCES
    src/snippet_manager.cpp
    src/snippet_manager_search.cpp
    src/snippet_manager_io.cpp
)

# LaTeX compiler sources (split into modules for maintainability)
set(LATEX_COMPILER_SOURCES
    src/latex_compiler.cpp
    src/latex_compiler_wrap.cpp
    src/latex_compiler_extract.cpp
    src/latex_compiler_convert.cpp
)

# Keyword database sources (split into modules for maintainability)
set(TIKZ_KEYWORDS_SOURCES
    src/tikz_keywords.cpp
    src/tikz_keywords_data.cpp
    src/tikz_keywords_basic.cpp
    src/tikz_keywords_commands.cpp
    src/tikz_keywords_options.cpp
    src/tikz_keywords_pgfplots.cpp
    src/tikz_keywords_extended.cpp
)

add_executable(TikzManager
    src/main.cpp
    ${MAINWINDOW_SOURCES}
    ${SNIPPET_MANAGER_SOURCES}
    ${LATEX_COMPILER_SOURCES}
    src/code_editor.cpp
    src/tikz_highlighter.cpp
    ${TIKZ_COMPLETER_SOURCES}
    ${TIKZ_KEYWORDS_SOURCES}
    src/tikz_document_state.cpp
    src/settings_dialog.cpp
    ${SEARCH_PANEL_SOURCES}
    src/snippet_properties_dialog.cpp
    src/kde_global_shortcut.cpp
    src/pdf_preview_widget.cpp
    src/flow_layout.cpp
)

target_link_libraries(TikzManager PRIVATE Qt6::Widgets Qt6::Pdf Qt6::PdfWidgets Qt6::Concurrent)

# Installed program name is all-lowercase "hitikz" (matches hitikz.desktop).
set_target_properties(TikzManager PROPERTIES OUTPUT_NAME ${APP_ID})

target_compile_definitions(TikzManager PRIVATE
    RESOURCE_DIR=${CMAKE_SOURCE_DIR}/resources
    INSTALL_RESOURCE_DIR=${CMAKE_INSTALL_FULL_DATADIR}/${APP_ID}/resources
    APP_VERSION="${PROJECT_VERSION}"
)

if(HAS_KGLOBALACCEL)
    target_link_libraries(TikzManager PRIVATE KF6::GlobalAccel)
    target_compile_definitions(TikzManager PRIVATE HAS_KGLOBALACCEL)
endif()

if(NOT HAS_KGLOBALACCEL AND WITH_QHOTKEY AND TARGET qhotkey)
    target_link_libraries(TikzManager PRIVATE qhotkey)
    target_compile_definitions(TikzManager PRIVATE HAS_QHOTKEY)
endif()

# ---------------------------------------------------------------------------
# Installation
#
#   cmake --install build                 # install to CMAKE_INSTALL_PREFIX (/usr/local by default)
#   cmake --install build --prefix /usr   # or a custom prefix
#   sudo cmake --build build --target uninstall
# ---------------------------------------------------------------------------
install(TARGETS TikzManager
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Bundled presets and templates.
install(DIRECTORY resources/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${APP_ID}/resources)

# Desktop entry and application icon.
install(FILES ${CMAKE_SOURCE_DIR}/${APP_ID}.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${CMAKE_SOURCE_DIR}/${APP_ID}.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

# Uninstall target (CMake has no built-in one) — removes files listed in the
# install manifest generated at install time.
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_uninstall.cmake)
endif()

enable_testing()

add_executable(test_snippet_manager
    tests/test_snippet_manager.cpp
    ${SNIPPET_MANAGER_SOURCES}
)
target_include_directories(test_snippet_manager PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_snippet_manager PRIVATE Qt6::Core)
add_test(NAME SnippetManager COMMAND test_snippet_manager)

add_executable(test_latex_compiler
    tests/test_latex_compiler.cpp
    ${LATEX_COMPILER_SOURCES}
)
target_include_directories(test_latex_compiler PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_latex_compiler PRIVATE Qt6::Core)
add_test(NAME LatexCompiler COMMAND test_latex_compiler)

add_executable(test_search
    tests/test_search.cpp
    ${SNIPPET_MANAGER_SOURCES}
)
target_include_directories(test_search PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_search PRIVATE Qt6::Core)
add_test(NAME Search COMMAND test_search)

add_executable(test_packages_libraries
    tests/test_packages_libraries.cpp
    ${LATEX_COMPILER_SOURCES}
    ${SNIPPET_MANAGER_SOURCES}
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
    ${MAINWINDOW_SOURCES}
    src/code_editor.cpp
    src/tikz_highlighter.cpp
    ${TIKZ_COMPLETER_SOURCES}
    src/settings_dialog.cpp
    ${SEARCH_PANEL_SOURCES}
    src/snippet_properties_dialog.cpp
    src/kde_global_shortcut.cpp
    src/pdf_preview_widget.cpp
    src/flow_layout.cpp
    ${SNIPPET_MANAGER_SOURCES}
    ${LATEX_COMPILER_SOURCES}
    ${TIKZ_KEYWORDS_SOURCES}
    src/tikz_document_state.cpp
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

add_executable(test_completer
    tests/test_completer.cpp
    ${TIKZ_COMPLETER_SOURCES}
    ${TIKZ_KEYWORDS_SOURCES}
    src/tikz_document_state.cpp
)
target_include_directories(test_completer PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_completer PRIVATE Qt6::Widgets)
add_test(NAME Completer COMMAND test_completer)

add_executable(test_document_state
    tests/test_document_state.cpp
    src/tikz_document_state.cpp
)
target_include_directories(test_document_state PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_document_state PRIVATE Qt6::Widgets)
add_test(NAME DocumentState COMMAND test_document_state)

add_executable(test_enhanced_highlighter
    tests/test_enhanced_highlighter.cpp
    src/tikz_highlighter.cpp
    src/tikz_document_state.cpp
)
target_include_directories(test_enhanced_highlighter PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_enhanced_highlighter PRIVATE Qt6::Widgets)
add_test(NAME EnhancedHighlighter COMMAND test_enhanced_highlighter)
