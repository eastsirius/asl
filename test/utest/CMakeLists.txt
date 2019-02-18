
set(ASL_PROJECT_NAME utest)

set(ASL_C_FLAGS -Wall -std=c++11)
set(ASL_LINK_FLAGS )
set(ASL_INCLUDE_PATH ${ASL_ROOT_PATH}/src)
set(ASL_LIBRARY_PATH )
set(ASL_LINK_LIBRARIES asl pthread dl)
set(ASL_SRC_BASE ${ASL_ROOT_PATH}/test)
set(ASL_SRC_FILES )
aux_source_directory(${ASL_SRC_BASE}/utest ASL_SRC_FILES)

include_directories(${ASL_INCLUDE_PATH})
link_directories(${ASL_LIBRARY_PATH})
add_definitions(${ASL_C_FLAGS})
add_executable(${ASL_PROJECT_NAME} ${ASL_SRC_FILES})
target_link_libraries(${ASL_PROJECT_NAME} ${ASL_LINK_LIBRARIES} ${ASL_LINK_FLAGS})
