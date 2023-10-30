cmake_minimum_required(VERSION 3.25)
project(windows)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/res)
set(TORCH_INSTALL_PREFIX D:\\Libraries\\libtorch\\2.0.1+cu118\\${CMAKE_BUILD_TYPE})
find_package(Torch REQUIRED PATHS ${TORCH_INSTALL_PREFIX})

set(BUILD_TESTING OFF)
set(SAMPLERATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libsamplerate/include)
set(VITS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/vits)

add_subdirectory(libsamplerate)
add_subdirectory(vits)
add_subdirectory(ui)

#file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)

add_library(wav_utils SHARED src/utils.cpp)

add_custom_target(main)
add_dependencies(main samplerate vits whisper_a2o)

add_executable(test src/main.cpp)

add_dependencies(test vits wav_utils)

target_include_directories(test
        PRIVATE vits
        PRIVATE ${TORCH_INCLUDE_DIRS}
)

target_link_libraries(test
        PRIVATE vits
        PRIVATE wav_utils
        PRIVATE ${TORCH_LIBRARIES}
)

add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/res/resource.dll
        ${CMAKE_BINARY_DIR}/bin/resource.dll
)
