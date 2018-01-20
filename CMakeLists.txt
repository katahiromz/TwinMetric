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

add_executable(TwinMetricPlus TwinMetricPlus.cpp)
target_include_directories(TwinMetricPlus PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(TwinMetricPlus ${FREETYPE_LIBRARIES})

add_executable(TwinMetricMinus TwinMetricMinus.cpp)
target_include_directories(TwinMetricMinus PRIVATE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(TwinMetricMinus ${FREETYPE_LIBRARIES})

##############################################################################
