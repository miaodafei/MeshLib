cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (MeshViewer CXX)

add_executable(${PROJECT_NAME} MRViewerApp.cpp)

file(GLOB LIB_LIST "../*.json")
file(GLOB PNGS "*.png")
file(COPY ${LIB_LIST} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ${PNGS} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

IF(MR_EMSCRIPTEN)
file(GLOB LOCAL_WASM_FILES "../../wasm/*.*")
file(COPY ${LOCAL_WASM_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
target_link_libraries(${PROJECT_NAME} PRIVATE
    pthread
	zip
	gtest
	-Wl,--whole-archive
	MRCommonPlugins
	MRMesh
	MRViewer
	-Wl,--no-whole-archive
)
ELSE() # MR_EMSCRIPTEN
target_link_libraries(${PROJECT_NAME} PRIVATE
	MRViewer
	MRMesh
)
ENDIF() # MR_EMSCRIPTEN

install(TARGETS ${PROJECT_NAME} DESTINATION "${MR_BIN_DIR}")
install(FILES ${LIB_LIST} DESTINATION "${MR_RESOURCES_DIR}")
install(FILES ${PNGS} DESTINATION "${MR_RESOURCES_DIR}")
