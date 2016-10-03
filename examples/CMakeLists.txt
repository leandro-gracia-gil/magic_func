# Generic event queue example.
add_executable(generic_event_queue "")

target_sources(generic_event_queue PRIVATE
  generic_event_queue/generic_event_queue.cc
  generic_event_queue/main.cc
)

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
