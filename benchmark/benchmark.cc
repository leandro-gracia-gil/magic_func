#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>

#include <magic_func/function.h>
#include <magic_func/make_function.h>
#include <magic_func/member_function.h>

#include "delegate.h"

static constexpr size_t kNumExperiments = 100;
static constexpr size_t kNumIterations = 10000000;

using Clock = std::chrono::high_resolution_clock;

using mf::Function;
using mf::MakeFunction;
using mf::MemberFunction;

void FreeFunction(size_t& value);

struct Object {
  Object() : value(0) {}
  void Function(int delta);
  size_t value;
};

namespace {

template <typename T, typename... Args>
void TestFunction(double& mean, double& stdev,
                  const T& function, Args&&... args) {
  std::unique_ptr<double[]> experiment_mean(new double[kNumExperiments]);
  mean = 0.0;

  for (size_t i = 0; i < kNumExperiments; ++i) {
    // Run a fixed set of iterations of the function call in each experiment.
    auto start = Clock::now();
    for (size_t j = 0; j < kNumIterations; ++j)
      function(std::forward<Args>(args)...);
    auto end = Clock::now();

    Clock::duration duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    experiment_mean[i] = (long double) duration.count() / kNumIterations;
    mean += experiment_mean[i];
  }

  mean /= (double) kNumExperiments;
  stdev = 0.0;
  for (size_t i = 0; i < kNumExperiments; ++i) {
    double diff = experiment_mean[i] - mean;
    stdev += diff * diff;
  }

  stdev = sqrt(stdev / (double)(kNumExperiments - 1));
}

}  // anonymous namespace

void BenchmarkFunction() {
  std::cout << "# Calling a function (mean, stdev)." << std::endl;

  double mean_std = 0.0, stdev_std = 0.0;
  {
    size_t call_count = 0;
    auto function = std::function<void(size_t&)>(&FreeFunction);
    TestFunction(mean_std, stdev_std, function, call_count);
  }
  std::cout << "std::function " << mean_std << " " << stdev_std << std::endl;

  double mean_mf = 0.0, stdev_mf = 0.0;
  {
    size_t call_count = 0;
    auto function = MF_MakeFunction(&FreeFunction);
    TestFunction(mean_mf, stdev_mf, function, call_count);
  }
  std::cout << "mf::Function " << mean_mf << " " << stdev_mf << std::endl;

  double mean_del = 0.0, stdev_del = 0.0;
  {
    size_t call_count = 0;
    auto function = delegate<void(size_t&)>::from<&FreeFunction>();
    TestFunction(mean_del, stdev_del, function, call_count);
  }
  std::cout << "delegate " << mean_del << " " << stdev_del << std::endl;
  std::cout << "Speed-up " << (mean_del / mean_mf) << "x (delegate) -- "
            << (mean_std / mean_mf) << "x (std)\n" << std::endl;
}

void BenchmarkFunctionLambda() {
  std::cout << "# Calling a lambda (mean, stdev)." << std::endl;

  double mean_std = 0.0, stdev_std = 0.0;
  {
    size_t call_count = 0;
    auto lambda = [&]() { ++call_count; };
    std::function<void()> function = lambda;
    TestFunction(mean_std, stdev_std, function);
  }
  std::cout << "std::function " << mean_std << " " << stdev_std << std::endl;

  double mean_mf = 0.0, stdev_mf = 0.0;
  {
    size_t call_count = 0;
    auto lambda = [&]() { ++call_count; };
    Function<void()> function = MakeFunction(lambda);
    TestFunction(mean_mf, stdev_mf, function);
  }
  std::cout << "mf::Function " << mean_mf << " " << stdev_mf << std::endl;

  double mean_del = 0.0, stdev_del = 0.0;
  {
    size_t call_count = 0;
    auto lambda = [&]() { ++call_count; };
    delegate<void()> function(lambda);
    TestFunction(mean_del, stdev_del, function);
  }
  std::cout << "delegate " << mean_del << " " << stdev_del << std::endl;
  std::cout << "Speed-up " << (mean_del / mean_mf) << "x (delegate) -- "
            << (mean_std / mean_mf) << "x (std)\n" << std::endl;
}

void BenchmarkBoundMemberFunctionAddressAndPointer() {
  std::cout
      << "# Calling a member function bound to an object pointer (mean, stdev)."
      << std::endl;

  double mean_std = 0.0, stdev_std = 0.0;
  {
    Object object;
    std::function<void(size_t)> function = std::bind(&Object::Function, &object,
        std::placeholders::_1);

    TestFunction(mean_std, stdev_std, function, 1);
  }
  std::cout << "std::function " << mean_std << " " << stdev_std << std::endl;

  double mean_mf = 0.0, stdev_mf = 0.0;
  {
    Object object;
    auto function = MF_MakeFunction(&Object::Function, &object);
    TestFunction(mean_mf, stdev_mf, function, 1);
  }
  std::cout << "mf::Function " << mean_mf << " " << stdev_mf << std::endl;

  double mean_del = 0.0, stdev_del = 0.0;
  {
    Object obj;
    auto function(delegate<void(int)>::from<Object, &Object::Function>(&obj));
    TestFunction(mean_del, stdev_del, function, 1);
  }
  std::cout << "delegate " << mean_del << " " << stdev_del << std::endl;
  std::cout << "Speed-up " << (mean_del / mean_mf) << "x (delegate) -- "
            << (mean_std / mean_mf) << "x (std)\n" << std::endl;
}

int main() {
  BenchmarkFunction();
  BenchmarkBoundMemberFunctionAddressAndPointer();
  BenchmarkFunctionLambda();
  return 0;
}
