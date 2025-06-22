cmake_minimum_required(VERSION 3.14)
project(TopTracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)

add_library(TopTracker
    TopTracker.cpp
)
target_include_directories(TopTracker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(TopTrackerTests
    TestTopTracker.cpp
)
target_link_libraries(TopTrackerTests
    PRIVATE
        TopTracker
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(TopTrackerTests)

