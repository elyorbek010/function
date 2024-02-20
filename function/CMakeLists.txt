Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_library(function "src/function.cpp")
target_include_directories(function PRIVATE inc)

add_executable(function_tests "test/test.cpp")
target_link_libraries(function_tests PRIVATE function PRIVATE Catch2::Catch2WithMain)
target_include_directories(function_tests PRIVATE inc)

target_compile_options(function_tests PUBLIC "$<$<CONFIG:DEBUG>:${GCC_COMPILE_DEBUG_OPTIONS}>")
target_compile_options(function_tests PUBLIC "$<$<CONFIG:RELEASE>:${GCC_COMPILE_RELEASE_OPTIONS}>")