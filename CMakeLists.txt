cmake_minimum_required(VERSION 3.0)
project(metacheck)

add_subdirectory(mpl EXCLUDE_FROM_ALL)

# always rebuild anything that uses the random header to generate a new random seed
add_custom_command(OUTPUT src/metacheck/random.hpp
		COMMAND touch src/metacheck/random.hpp
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(metacheck_interface src/metacheck/metacheck.hpp src/metacheck/random.hpp)
target_compile_options(metacheck_interface PUBLIC -ftemplate-depth=2048 PRIVATE -Wall -Werror
		-ftemplate-backtrace-limit=0)
target_compile_features(metacheck_interface PUBLIC cxx_std_14)
target_link_libraries(metacheck_interface PRIVATE kvasir_mpl)
set_target_properties(metacheck_interface PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(metacheck_interface PUBLIC src/)


set(METACHECK_TEST_SOURCES test/metacheck_test_main.cpp #[[test/generators.cpp]] test/utility.cpp)

add_executable(metacheck_test ${METACHECK_TEST_SOURCES})
target_link_libraries(metacheck_test PUBLIC metacheck_interface PRIVATE kvasir_mpl)
target_compile_options(metacheck_test PRIVATE -Wall -Werror -ftemplate-backtrace-limit=0)

option(METACHECK_TEST_RANDOM 0)
if (METACHECK_TEST_RANDOM)
	target_compile_definitions(metacheck_test PRIVATE -DMETACHECK_RANDOM=${METACHECK_TEST_RANDOM})
endif ()


# target that includes metacheck and self-tests the library after inclusion
add_executable(metacheck_full ${METACHECK_TEST_SOURCES})
target_link_libraries(metacheck_full PUBLIC metacheck_interface PRIVATE kvasir_mpl)
add_custom_command(TARGET metacheck_full POST_BUILD
		COMMAND metacheck_full
		COMMENT "Testing metacheck internals")
option(BUILD_TESTING OFF)
if (BUILD_TESTING)
	add_dependencies(metacheck_full kvasir_mpl_test)
	add_custom_command(TARGET metacheck_full POST_BUILD
			COMMAND kvasir_mpl_test
			COMMENT "Testing kvasir::mpl internals")
endif ()
