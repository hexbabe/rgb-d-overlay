cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(rgb-d-overlay LANGUAGES CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)
FetchContent_Declare(
  viam-cpp-sdk
  GIT_REPOSITORY https://github.com/viamrobotics/viam-cpp-sdk.git
  GIT_TAG bb4e1bd3905f91c015233ed8e9b2aae8eb673f84
  # SOURCE_DIR ${CMAKE_SOURCE_DIR}/../viam-cpp-sdk
  CMAKE_ARGS -DVIAMCPPSDK_USE_DYNAMIC_PROTOS=ON
  EXCLUDE_FROM_ALL
  FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(viam-cpp-sdk)

find_package(ImageMagick COMPONENTS Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})

FILE(GLOB sources src/*.cpp)
add_executable(rgb-d-overlay ${sources})
target_link_libraries(rgb-d-overlay PRIVATE viam-cpp-sdk::viamsdk ${ImageMagick_LIBRARIES})
