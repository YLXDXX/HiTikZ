cmake_minimum_required(VERSION 3.16)
project(TikzManager VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Pdf PdfWidgets)

add_executable(TikzManager
    src/main.cpp
    src/mainwindow.cpp
    src/snippet_manager.cpp
    src/latex_compiler.cpp
    src/code_editor.h
)

target_link_libraries(TikzManager PRIVATE Qt6::Widgets Qt6::Pdf Qt6::PdfWidgets)
