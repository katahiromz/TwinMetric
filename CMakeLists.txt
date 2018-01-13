# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008" .
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles" .
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 2.4)

# enable CTest
enable_testing()

# project name and language
project(TwinMetric CXX)

# use FreeType
find_package(FreeType REQUIRED)

# set output directory (build/)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

##############################################################################

add_executable(TwinMetric TwinMetric.cpp)
target_include_directories(TwinMetric PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(TwinMetric ${FREETYPE_LIBRARIES})

add_executable(TwinMetric2 TwinMetric2.cpp)
target_include_directories(TwinMetric2 PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(TwinMetric2 ${FREETYPE_LIBRARIES})

##############################################################################
