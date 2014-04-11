cmake_minimum_required(VERSION 2.8)

project(tests)

add_executable(test_mesh test_mesh.cpp)
add_executable(test_vol_mesh test_vol_mesh.cpp)
add_executable(test_ptr_vector test_ptr_vector.cpp)
add_executable(test_vector test_vector.cpp)

target_link_libraries(test_mesh        mesh3d)
target_link_libraries(test_ptr_vector  mesh3d)
target_link_libraries(test_vector      mesh3d)
target_link_libraries(test_vol_mesh    mesh3d)

add_test(NAME TestVector COMMAND test_vector)
add_test(NAME TestPtrVector COMMAND test_ptr_vector)
add_test(NAME TestMesh COMMAND test_mesh)
add_test(NAME TestVolMesh COMMAND test_vol_mesh)

if(USE_METIS)
	add_executable(test_part test_part.cpp)
	target_link_libraries(test_part ${METIS_LIBRARY} mesh3d)
	add_test(NAME TestPartition COMMAND test_part)
endif()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/mesh.vol DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
