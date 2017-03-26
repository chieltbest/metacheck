cmake_minimum_required(VERSION 3.0)
project(metacheck)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=2048")

# always rebuild anything that uses the random header
add_custom_command(OUTPUT src/random.hpp
		COMMAND touch src/random.hpp
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES main.cpp src/random.hpp)
add_executable(metacheck_test ${SOURCE_FILES})
target_link_libraries(metacheck_test)
