cmake_minimum_required(VERSION 4.0)
project(course-work-ml-evaluation)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
        src/main.cpp
)

add_executable(course-work-ml-evaluation ${SOURCES} )

target_include_directories(course-work-ml-evaluation PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/dto
)