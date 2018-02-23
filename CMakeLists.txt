cmake_minimum_required(VERSION 2.8)
project(how-to-use-libclang)

add_compile_options(-std=c++11 -Wall -Wextra -Wno-unused-parameter)

if(APPLE)
	set(CMAKE_MACOSX_RPATH 1)
endif()


list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")
find_package(LibClang REQUIRED)
include_directories(${LIBCLANG_INCLUDE_DIRS})

add_executable(list-all-children
		src/main.cc)
target_link_libraries(list-all-children ${LIBCLANG_LIBRARIES})

