add_executable(benchmarks "")

target_sources(benchmarks PRIVATE
  benchmark.cc
  benchmark_functions.cc
  delegate.h
)

target_compile_definitions(benchmarks PRIVATE NDEBUG)
target_compile_options(benchmarks PRIVATE "${SPEED_FLAGS}")
