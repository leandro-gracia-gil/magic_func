cmake_minimum_required(VERSION 3.1)
project(magic_func)

# Path to googletest prebuilt libraries.
set(GOOGLETEST "${CMAKE_SOURCE_DIR}/googletest")

# Set default compile flag settings.
#
# Test flags do not introduce speed optimizations and enable C++ exceptions.
# This is needed because some test use exceptions to catch MagicFunc errors.
#
# Speed flags try to get the code as optimized as possible.
if(MSVC)
  # IMPORTANT: when using Visual Studio make sure to use /OPT:NOICF as otherwise
  # MagicFunc's type id mechanism might not work correctly and might fail to
  # detect errors.
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:NOICF")

  # Required to match the settings in googletest.
  # This is only because we build unit tests. It's not a MagicFunc requirement.
  foreach(flag_var
          CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
          CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
  endforeach()

  set(TEST_FLAGS /EHsc)
  set(TEST_FLAGS_CPP14 /EHsc)
  set(SPEED_FLAGS /GR- /Ob2 /Ot /GS-)
  set(SPEED_FLAGS_CPP14 /GR- /Ob2 /Ot /GS-)
else()
  set(TEST_FLAGS -std=c++11)
  set(TEST_FLAGS_CPP14 -std=c++14)
  set(SPEED_FLAGS -std=c++11 -O3 -fno-exceptions -fno-rtti)
  set(SPEED_FLAGS_CPP14 -std=c++14 -O3 -fno-exceptions -fno-rtti)
endif()

# Main includes for magic func.
include_directories("include")

# Different components to build.
add_subdirectory("test")
add_subdirectory("benchmark")
add_subdirectory("examples")

# Googletest dependency for unit testing.
add_subdirectory("googletest")
