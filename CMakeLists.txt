cmake_minimum_required(VERSION 3.1.0)

project(plot VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows -fopenmp")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(Qt5_DIR D:/Qt/Qt5.12.3/5.12.3/msvc2017/lib/cmake/Qt5)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
if(Qt5_FOUND)
        message(STATUS "Found Qt5.")
else(Qt5_FOUND)
        message(FATAL_ERROR "Not Found Qt5.")
endif()

# find DCMTK library
set(DCMTK_DIR D:/dcmtk/build)
find_package(DCMTK)
if (DCMTK_FOUND)
    message(STATUS "Found DCMTK: " ${DCMTK_INCLUDE_DIR})
    include_directories(${DCMTK_INCLUDE_DIR})
else (MD_FOUND)
    message("DCMTK not found. Please set DCMTK_DIR")
        set(DCMTK_DIR D:/dcmtk/build)
        include_directories(${DCMTK_INCLUDE_DIR})
endif()

FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

file(GLOB SOURCES src/*)
file(GLOB QRCS qrc/*.qrc qrc/*.rc)
file(GLOB Q_PLOT_H include/plot.h)

add_executable(plot ${SOURCES} ${QRCS} ${Q_PLOT_H})
target_link_libraries(plot Qt5::Widgets ${DCMTK_LIBRARIES})
target_include_directories(plot PRIVATE ${PROJECT_SOURCE_DIR}/include)
