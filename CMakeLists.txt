cmake_minimum_required (VERSION 2.6)

project(gameserver_v8)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


file (GLOB MAIN_SOURCES "*.cpp" "*.h" "server/*.cpp" "server/*.h" "server/SAMP/*.cpp" "server/SAMP/*.h" "server/SAMP/RakNet/*.cpp" "server/SAMP/RakNet/*.h" "server/python/*.h" "server/python/*.cpp") 

set (ALL_SRCS ${MAIN_SOURCES})

include_directories (${CMAKE_CURRENT_SOURCE_DIR} "server/SAMP")

source_group("" FILES ${MAIN_SOURCES})

add_executable (gameserver ${ALL_SRCS})
target_link_libraries(gameserver)

