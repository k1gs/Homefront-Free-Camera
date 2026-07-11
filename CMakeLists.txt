cmake_minimum_required(VERSION 3.20)
project(homefront_freecam C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Build as x86 (32-bit)!\nRun cmake with: cmake -A Win32 ..")
endif()

set(MINHOOK_SRC
    third_party/minhook/src/buffer.c
    third_party/minhook/src/hook.c
    third_party/minhook/src/trampoline.c
    third_party/minhook/src/hde/hde32.c
)

add_library(minhook STATIC ${MINHOOK_SRC})
target_include_directories(minhook PUBLIC third_party/minhook/include)
set_target_properties(minhook PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")

set(MOD_SRC
    src/dllmain.cpp
    src/freecam.cpp
    src/hooks.cpp
    src/input.cpp
)

add_library(homefront_freecam SHARED ${MOD_SRC})
target_include_directories(homefront_freecam PRIVATE
    src
    third_party/minhook/include
)
target_link_libraries(homefront_freecam PRIVATE minhook)
set_target_properties(homefront_freecam PROPERTIES
    OUTPUT_NAME "homefront_freecam"
    MSVC_RUNTIME_LIBRARY "MultiThreaded"
)

add_executable(injector injector/injector.cpp)
set_target_properties(injector PROPERTIES
    OUTPUT_NAME "injector"
    MSVC_RUNTIME_LIBRARY "MultiThreaded"
)