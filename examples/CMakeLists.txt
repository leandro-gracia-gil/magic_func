# Generic event queue example.
add_executable(generic_event_queue "")

target_sources(generic_event_queue PRIVATE
  generic_event_queue/generic_event_queue.cc
  generic_event_queue/main.cc
)

# IMPORTANT: when using Visual Studio make sure to use /OPT:NOICF when using
# the generic event queue. This flag prevents the compiler from merging empty
# functions into the same address. Otherwise all events are fold together.
if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:NOICF")
endif()

target_compile_options(generic_event_queue PRIVATE "${SPEED_FLAGS_CPP14}")

# Generic event queue unit test.
add_executable(generic_event_queue_unittest "")

target_sources(generic_event_queue_unittest PRIVATE
  generic_event_queue/generic_event_queue.cc
  generic_event_queue/generic_event_queue_unittest.cc
)

target_link_libraries(generic_event_queue_unittest gtest)
target_link_libraries(generic_event_queue_unittest gtest_main)

target_compile_options(generic_event_queue_unittest PRIVATE "${TEST_FLAGS_CPP14}")
