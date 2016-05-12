cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project (osakana_prj)

find_package(OpenGL REQUIRED)

set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules"
)

find_package(OpenCV REQUIRED)

find_package(GLEW REQUIRED)

set(Boost_USE_STATIC_LIBS ON)

find_package(GLFW REQUIRED)

find_package(Boost COMPONENTS system filesystem date_time regex REQUIRED)

include_directories(
	${OPENGL_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${GLEW_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
)

add_executable (osakana_prj
	src/main.cpp
	src/vec2d.h
	src/vec3d.h
	src/behavior.cpp
	src/behavior.h
	src/fish.cpp
	src/fish.h
	src/shark.cpp
	src/shark.h
	src/world.cpp
	src/world.h
	src/teximage.cpp
	src/teximage.h
	src/shader.cpp
	src/shader.h
	src/vision.cpp
	src/vision.h
	src/fish_loader.cpp
	src/fish_loader.h
	src/water_surface.h
	src/water_surface_cpu.cpp
	src/water_surface_gpu_swe.cpp
	src/ipcom.h
	src/ipcom.cpp
	src/compressor.h
	src/compressor.cpp
	src/food.cpp
	src/food.h
	src/noise.cpp
	src/noise.h
)


target_link_libraries(osakana_prj
	${OPENGL_LIBRARY}
	${GLFW_LIBRARIES}
	${OpenCV_LIBS}
	${GLEW_LIBRARIES}
	${Boost_LIBRARIES}
)


if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "-g -std=c++14")
	TARGET_LINK_LIBRARIES(osakana_prj m)
endif()
