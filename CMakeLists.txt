cmake_minimum_required(VERSION 3.1.0)

project(plot VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-mwindows")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
if(Qt5_FOUND)
        message(STATUS "Found Qt5.")
else(Qt5_FOUND)
        message(FATAL_ERROR "Not Found Qt5.")
endif()
	
file(GLOB SOURCES src/*)
file(GLOB QRCS qrc/*.qrc qrc/*.rc)
file(GLOB Q_PLOT_H include/plot.h)

add_executable(plot ${SOURCES} ${QRCS} ${Q_PLOT_H})
target_link_libraries(plot Qt5::Widgets)
target_include_directories(plot PRIVATE ${PROJECT_SOURCE_DIR}/include)
