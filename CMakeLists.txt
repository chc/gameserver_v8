cmake_minimum_required (VERSION 2.6)

project(gameserver_v8)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

file (GLOB MAIN_SOURCES "*.cpp" "*.h" "server/*.cpp" "server/*.h" "server/SAMP/*.cpp" "server/SAMP/*.h" "server/SAMP/RakNet/*.cpp" "server/SAMP/RakNet/*.h") 

set (ALL_SRCS ${MAIN_SOURCES})

include_directories (${CMAKE_CURRENT_SOURCE_DIR} "server/SAMP" "/Users/chc/v8")

source_group("" FILES ${MAIN_SOURCES})

#OpenGL specific stuff
IF(WIN32)

add_executable (assettool ${ALL_SRCS})
target_link_libraries(assettool )
target_link_libraries(assettool debug libpng zlib libsquishd pugixmld freetyped jansson_d assimpd)
target_link_libraries(assettool general libpng zlib libsquish pugixml freetype jansson assimp)

ELSE() #unix
add_executable (assettool ${ALL_SRCS})
set(V8_LIBS "/Users/chc/v8/out/x64.debug/libv8_base.a" "/Users/chc/v8/out/x64.debug/libv8_external_snapshot.a" "/Users/chc/v8/out/x64.debug/libv8_libbase.a" "/Users/chc/v8/out/x64.debug/libv8_libplatform.a" "/Users/chc/v8/out/x64.debug/libicudata.a" "/Users/chc/v8/out/x64.debug/libicui18n.a" "/Users/chc/v8/out/x64.debug/libicuuc.a") 
target_compile_options(assettool PUBLIC -fpermissive)
target_link_libraries(assettool c++ dl pthread ${V8_LIBS})
ENDIF()
 