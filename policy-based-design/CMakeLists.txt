cmake_minimum_required(VERSION 3.6)

##################
# Project
get_filename_component(PROJECT_NAME_DIR ${CMAKE_SOURCE_DIR} NAME)
string(REPLACE " " "_" PROJECT_ID ${PROJECT_NAME_DIR})

set(PROJECT_MAIN ${PROJECT_ID})
project(${PROJECT_MAIN})

add_subdirectory(src)
add_subdirectory(tests)

####################
# Main app
include_directories(src)
add_executable(${PROJECT_MAIN} main.cpp)
target_link_libraries(${PROJECT_MAIN} PRIVATE ${PROJECT_LIB} ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(${PROJECT_MAIN} PUBLIC cxx_std_17)
