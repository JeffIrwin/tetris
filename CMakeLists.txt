
cmake_minimum_required(VERSION 3.7)

set(PROJECT tetris)

if (LINUX)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
	# Windows
	# TODO:  set debug/release flags
endif()

project(${PROJECT})

set(SRC_DIR src)

set(SUBMODULE_DIR submodules)

# TODO: add glfw, fmt to pgh submodules

#set(COLORMAPPER_DIR ${SUBMODULE_DIR}/colormapper)
set(GLFW_DIR ${SUBMODULE_DIR}/glfw)
set(FMT_DIR  ${SUBMODULE_DIR}/fmt)
set(PNG_DIR  ${SUBMODULE_DIR}/lodepng)

#add_subdirectory(${COLORMAPPER_DIR})
add_subdirectory(${GLFW_DIR})
add_subdirectory(${FMT_DIR} )

include_directories(
	${SRC_DIR}
	${GLFW_DIR}/deps/
	${FMT_DIR}/include/
	${PNG_DIR}/
	)

add_executable(${PROJECT}
	${SRC_DIR}/main.cpp
	${PNG_DIR}/lodepng.cpp
	)

target_link_libraries(${PROJECT}
	#colormapper
	glfw
	fmt
	)

