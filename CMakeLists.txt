
cmake_minimum_required(VERSION 3.28)

project(uJson)

set(CMAKE_CXX_STANDARD 17)

add_library(uJson STATIC)
target_include_directories(uJson PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(uJson PRIVATE uJson/src/uJson.cpp)

if (DEFINED BUILD_TESTS)
    find_package(Git)
    if(NOT GIT_FOUND)
        message(FATAL_ERROR "git not found!")
    endif()

    set(EXTERNAL_LIB_LOCATION ${CMAKE_SOURCE_DIR}/external_libs)
    if(NOT EXISTS ${EXTERNAL_LIB_LOCATION}) 
        file(MAKE_DIRECTORY ${EXTERNAL_LIB_LOCATION})
    endif()

    if(NOT EXISTS ${EXTERNAL_LIB_LOCATION}/googletest) 
        execute_process(
        COMMAND             ${GIT_EXECUTABLE} clone "https://github.com/google/googletest.git" --single-branch -q --recursive ${${SOURCE_DIR}} --branch v1.8.x
        WORKING_DIRECTORY   ${EXTERNAL_LIB_LOCATION}
        RESULT_VARIABLE     git_result
        OUTPUT_VARIABLE     git_output)
        if(NOT git_result EQUAL "0")
            message(FATAL_ERROR "git not found! ${git_result}")
        endif()    
    endif()

    add_subdirectory(${EXTERNAL_LIB_LOCATION}/googletest)
    add_subdirectory(tests)
endif()