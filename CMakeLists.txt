cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project (osakana_prj)

find_package(OpenGL REQUIRED)

if(WIN32)
	if(GLUT_ROOT_PATH)
	  message(WARNING "warning: GLUT_ROOT_PATH is depreciated. ")
	  set(DEPRECIATED_WIN32_INCLUDE ${GLUT_ROOT_PATH}/include)
	  set(DEPRECIATED_WIN32_RELEASE ${GLUT_ROOT_PATH}/Release)
	endif()
	IF (CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(ARCH x64)
	ENDIF()
	find_path( GLUT_INCLUDE_DIR NAMES GL/glut.h 
		PATHS 
		$ENV{GLUT_ROOT_DIR}/include
		${DEPRECIATED_WIN32_INCLUDE})
	find_library( GLUT_glut_LIBRARY NAMES glut glut32 freeglut
		PATHS
		$ENV{GLUT_ROOT_DIR}/Release
		${OPENGL_LIBRARY_DIR}
		${DEPRECIATED_WIN32_RELEASE}
		)

	SET(GLUT_LIBRARIES ${GLUT_glut_LIBRARY})
	MARK_AS_ADVANCED(
		GLUT_INCLUDE_DIR
		GLUT_glut_LIBRARY
	)
else ()
	find_package(GLUT REQUIRED)
endif ()

find_package(OpenCV REQUIRED)
find_package(GLEW REQUIRED)

include_directories(
	${OPENGL_INCLUDE_DIR}
	${GLUT_INCLUDE_DIR}
	${GLEW_INCLUDE_DIRS}
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
	src/target.cpp
	src/target.h
	src/shader.cpp
	src/shader.h
	src/vision.cpp
	src/vision.h
	src/fish_loader.cpp
	src/fish_loader.h
	src/water_surface.h
	src/water_surface_cpu.cpp
	src/water_surface_gpu_swe.cpp
	
	src/win/dirent.h
)


target_link_libraries(osakana_prj
	${GLUT_LIBRARIES}
	${OPENGL_LIBRARY}
	${OpenCV_LIBS}
	${GLEW_LIBRARIES}
)

set_property(TARGET osakana_prj PROPERTY CXX_STANDARD 14)
set_property(TARGET osakana_prj PROPERTY CXX_STANDARD_REQUIRED ON)

IF(UNIX)
	set(CMAKE_CXX_FLAGS "-g")
	TARGET_LINK_LIBRARIES(osakana_prj m)
ENDIF(UNIX)
