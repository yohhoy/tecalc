cmake_minimum_required(VERSION 3.11)
project(tecalc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# use Catch2 unit testing framework
Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.7
)
FetchContent_MakeAvailable(Catch2)

include_directories(include)

enable_testing()
add_executable(${PROJECT_NAME} test/unittest.cpp)
target_link_libraries(${PROJECT_NAME} Catch2::Catch2)
add_test(NAME unittest COMMAND ${PROJECT_NAME})
