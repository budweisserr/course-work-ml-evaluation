cmake_minimum_required(VERSION 4.0)
project(course-work-ml-evaluation)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
        src/main.cpp
)

set(QT_REQUIRED_COMPONENTS Widgets)

find_package(Qt5 COMPONENTS ${QT_REQUIRED_COMPONENTS} QUIET)

if(Qt5_FOUND)
    message(STATUS "Using Qt5")
    set(QT_PREFIX Qt5)
else()
    find_package(Qt6 COMPONENTS ${QT_REQUIRED_COMPONENTS} REQUIRED)
    message(STATUS "Using Qt6")
    set(QT_PREFIX Qt6)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(course-work-ml-evaluation ${SOURCES} )

target_link_libraries(course-work-ml-evaluation PRIVATE
        ${QT_PREFIX}::Widgets
)

target_include_directories(course-work-ml-evaluation PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/dto
)