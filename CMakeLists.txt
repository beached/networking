cmake_minimum_required( VERSION 2.8.12 )

project( networking_prj )

include( ExternalProject )

find_package( Boost 1.58.0 COMPONENTS system iostreams filesystem regex unit_test_framework REQUIRED )

enable_testing( )
add_definitions( -DBOOST_ALL_NO_LIB ) 

find_package( Threads REQUIRED )

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL 'MSVC' )
	add_compile_options( -D_WIN32_WINNT=0x0601 /std:c++latest ) 
else( )
	if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang" )
		add_compile_options(-std=c++14 -march=native -pthread -Weverything -Wno-c++98-compat -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O3")
	elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
		add_compile_options(-std=c++14 -march=native -pthread -Wall -Wno-deprecated-declarations -Wduplicated-cond -Wlogical-op -Wnull-dereference -Wold-style-cast -Wshadow)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O3")
	endif( )
endif( )

externalproject_add(
    header_libraries_prj
    GIT_REPOSITORY "https://github.com/beached/header_libraries.git"
    SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/header_libraries"
    GIT_TAG "master"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

set( HEADER_FOLDER "include" )
set( SOURCE_FOLDER "src" )
set( TEST_FOLDER "tests" )

include_directories( SYSTEM "${CMAKE_BINARY_DIR}/install/include" )
include_directories( ${HEADER_FOLDER} )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )

link_directories( "${CMAKE_BINARY_DIR}/install/lib" )
link_directories( ${Boost_LIBRARY_DIRS} )

set( HEADER_FILES
	${HEADER_FOLDER}/epoll.h
	${HEADER_FOLDER}/socket.h
)

set( SOURCE_FILES
	${SOURCE_FOLDER}/socket.cpp
)

add_definitions( -DBOOST_TEST_DYN_LINK -DBOOST_ALL_NO_LIB -DBOOST_ALL_DYN_LINK ) 

add_custom_target( check COMMAND ${CMAKE_CTEST_COMMAND} )

add_library( networking_lib ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( networking_lib header_libraries_prj )

add_executable( echo_test_bin EXCLUDE_FROM_ALL ${HEADER_FILES} ${TEST_FOLDER}/echo.cpp )
target_link_libraries( echo_test_bin networking_lib )
add_dependencies( echo_test_bin header_libraries_prj )
add_test( echo_test echo_test_bin )
add_dependencies( check echo_test_bin )

add_executable( epoll_echo_test_bin EXCLUDE_FROM_ALL ${HEADER_FILES} ${TEST_FOLDER}/epoll_echo.cpp )
target_link_libraries( epoll_echo_test_bin networking_lib )
add_dependencies( epoll_echo_test_bin header_libraries_prj )
add_test( epoll_echo_test epoll_echo_test_bin )
add_dependencies( check epoll_echo_test_bin )

install( TARGETS networking_lib DESTINATION lib )
install( DIRECTORY ${HEADER_FOLDER}/ DESTINATION include/daw/net )

