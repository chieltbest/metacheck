cmake_minimum_required(VERSION 3.0)
project(metacheck)

add_subdirectory(mpl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -ftemplate-depth=2048 -Wall
	-ftemplate-backtrace-limit=0")

# always rebuild anything that uses the random header
add_custom_command(OUTPUT src/random.hpp
		COMMAND touch src/random.hpp
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES main.cpp src/random.hpp)
add_executable(metacheck_test ${SOURCE_FILES})
target_link_libraries(metacheck_test kvasir_mpl)

option(METACHECK_TEST_RANDOM 0)
if (METACHECK_TEST_RANDOM)
	target_compile_definitions(metacheck_test PRIVATE -DMETACHECK_RANDOM=${METACHECK_TEST_RANDOM})
endif ()
