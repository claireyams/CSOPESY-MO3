cmake_minimum_required(VERSION 3.16)
project(MarqueeConsole LANGUAGES CXX)

add_executable(marquee
    src/main.cpp
    src/Console.cpp
)
target_include_directories(marquee PRIVATE include)
target_compile_features(marquee PRIVATE cxx_std_17)
set_target_properties(marquee PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_SOURCE_DIR}>"
)
