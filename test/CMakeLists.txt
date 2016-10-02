add_executable(unittests "")

target_sources(unittests PRIVATE
  allocator_unittest.cc
  function_cast_unittest.cc
  function_traits_unittest.cc
  function_unittest.cc
  make_function_unittest.cc
  member_function_unittest.cc
  test_common.cc
  type_erased_function_unittest.cc
  type_erased_object_unittest.cc
  type_traits_unittest.cc
)

target_compile_options(unittests PRIVATE "${TEST_FLAGS}")

target_link_libraries(unittests gtest)
target_link_libraries(unittests gtest_main)
