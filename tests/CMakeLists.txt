
add_executable(uJson_tests uJson_tests.cpp)
target_link_libraries(uJson_tests PUBLIC gmock_main uJson)