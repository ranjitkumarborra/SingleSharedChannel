cmake_minimum_required (VERSION 2.6)
project (SysDig)
# The version number.
set (SysDig_VERSION_MAJOR 1)
set (SysDig_VERSION_MINOR 0)
 
configure_file (
	"${PROJECT_SOURCE_DIR}/single_shared_channel.h"
	"${PROJECT_BINARY_DIR}/sync_queue.h"
	"${PROJECT_BINARY_DIR}/udp_client.h"
	"${PROJECT_BINARY_DIR}/udp_server.h"
  )
 
include_directories("${PROJECT_BINARY_DIR}")
 
# add the executable
add_executable(SysDig main.cpp)