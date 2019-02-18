
set(ASL_PROJECT_NAME asl)

set(ASL_C_FLAGS -Wall -std=c++11)
set(ASL_LINK_FLAGS )
set(ASL_INCLUDE_PATH ${ASL_ROOT_PATH}/src)
set(ASL_LIBRARY_PATH )
set(ASL_LINK_LIBRARIES )
set(ASL_SRC_BASE ${ASL_ROOT_PATH}/src)
set(ASL_SRC_FILES )
aux_source_directory(${ASL_SRC_BASE}/asl ASL_SRC_FILES)

include_directories(${ASL_INCLUDE_PATH})
link_directories(${ASL_LIBRARY_PATH})
add_definitions(${ASL_C_FLAGS})
add_library(${ASL_PROJECT_NAME} ${ASL_LIB_TYPE} ${ASL_SRC_FILES})
target_link_libraries(${ASL_PROJECT_NAME} ${ASL_LINK_LIBRARIES} ${ASL_LINK_FLAGS})
