cmake_minimum_required(VERSION 3.5)

project(lofi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)

find_package(Qt5Widgets)
add_executable(lofi
    main.cpp
    launcher.h launcher.cpp
    ini_parser.h ini_parser.cpp
    )

target_link_libraries(lofi Qt5::Widgets)
