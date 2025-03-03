cmake_minimum_required(VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(murom VERSION 0.1 LANGUAGES C CXX)

set(EXECUTABLE_OUTPUT_PATH ./${CMAKE_BUILD_TYPE})

set(SRC_FILES
    main.cpp
)

list(TRANSFORM SRC_FILES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/src/)

add_executable(murom ${SRC_FILES})

find_package(SDL2 REQUIRED)

target_include_directories(murom
    PUBLIC
        ${SDL2_INCLUDE_DIR}    
)

target_link_directories(murom
    PUBLIC
        ${SDL2_LIBRARY}
)

target_link_libraries(murom
    SDL2
)