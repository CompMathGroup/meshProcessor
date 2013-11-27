cmake_minimum_required(VERSION 2.8)

set (mesh3d_SOURCES
	vol_mesh.cpp
	mesh.cpp
	common.cpp
	vtk_stream.cpp
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_library(mesh3d STATIC ${mesh3d_SOURCES})

find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(
		doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
endif(DOXYGEN_FOUND) 
