cmake_minimum_required(VERSION 4.0)
project(course-work-ml-evaluation)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
        src/main.cpp
)

set(ONNXRUNTIME_DIR "${CMAKE_SOURCE_DIR}/onnxruntime" CACHE PATH "Path to ONNX Runtime")

if(WIN32)
    set(ONNXRUNTIME_LIB "${ONNXRUNTIME_DIR}/lib/onnxruntime.lib")
    set(ONNXRUNTIME_DLL "${ONNXRUNTIME_DIR}/lib/onnxruntime.dll")
elseif(APPLE)
    set(ONNXRUNTIME_LIB "${ONNXRUNTIME_DIR}/lib/libonnxruntime.dylib")
elseif(UNIX)
    set(ONNXRUNTIME_LIB "${ONNXRUNTIME_DIR}/lib/libonnxruntime.so")
endif()

include_directories(${ONNXRUNTIME_DIR}/include)

add_executable(course-work-ml-evaluation ${SOURCES} )

target_include_directories(course-work-ml-evaluation PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)