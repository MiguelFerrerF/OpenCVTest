cmake_minimum_required(VERSION 3.16)

project(OpenCVTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# --- OpenCV ---
find_package(OpenCV REQUIRED)
message(STATUS "Found OpenCV: ${OpenCV_VERSION}")
message(STATUS "Include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "Libraries: ${OpenCV_LIBS}")

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(OpenCVTest
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(OpenCVTest SHARED ${PROJECT_SOURCES})
    else()
        add_executable(OpenCVTest ${PROJECT_SOURCES})
    endif()
endif()

# Vincular Qt + OpenCV
target_link_libraries(OpenCVTest PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${OpenCV_LIBS}
)
target_include_directories(OpenCVTest PRIVATE ${OpenCV_INCLUDE_DIRS})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(OpenCVTest)
endif()
