cmake_minimum_required(VERSION 3.16)

project(OpenCVTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

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
    find_package(Qt6 REQUIRED COMPONENTS Core)

    qt_add_executable(OpenCVTest
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        videocapturehandler.h videocapturehandler.cpp
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
target_link_libraries(OpenCVTest PRIVATE Qt6::Core Qt6::Multimedia)
target_include_directories(OpenCVTest PRIVATE ${OpenCV_INCLUDE_DIRS})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(OpenCVTest)
endif()
