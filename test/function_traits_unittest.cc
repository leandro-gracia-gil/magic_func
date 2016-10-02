// Copyright (c) 2016, Leandro Graciá Gil
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <magic_func/function_traits.h>
#include <gtest/gtest.h>

#include "test_common.h"

using namespace mf;
using namespace mf::test;

namespace {

class Callable {
 public:
  const char* operator()(const int&, int&&) { return nullptr; }
};

int SampleFunction(const char*, float&, Callable&&);

struct TestObject {
  int Function(const char*, float&, Callable&&);
  int ConstFunction(const char*, float&, Callable&&) const;
  int VolatileFunction(const char*, float&, Callable&&) volatile;
  int ConstVolatileFunction(const char*, float&, Callable&&) const volatile;
};

}  // anonymous namespace

TEST(FunctionTraits, FreeFunctionType) {
  using T = FunctionTraits<int(const char*, float&, Callable&&)>;
  EXPECT_TRUE((std::is_same<T::Return, int>::value));
  EXPECT_TRUE((std::is_same<
      T::Args, std::tuple<const char*, float&, Callable&&>>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionType, int(const char*, float&, Callable&&)>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionPointerType,
      int (*) (const char*, float&, Callable&&)>::value));
  EXPECT_EQ(3, T::kNumArgs);
}

TEST(FunctionTraits, FreeFunctionPointer) {
  using T = FunctionTraits<decltype(&SampleFunction)>;
  EXPECT_TRUE((std::is_same<T::Return, int>::value));
  EXPECT_TRUE((std::is_same<
      T::Args, std::tuple<const char*, float&, Callable&&>>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionType, int(const char*, float&, Callable&&)>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionPointerType,
      decltype(&SampleFunction)>::value));
  EXPECT_EQ(3, T::kNumArgs);
}

TEST(FunctionTraits, MemberFunctionPointer) {
  using T = FunctionTraits<decltype(&TestObject::Function)>;
  EXPECT_TRUE((std::is_same<T::Class, TestObject>::value));
  EXPECT_TRUE((std::is_same<T::Return, int>::value));
  EXPECT_TRUE((std::is_same<
      T::Args, std::tuple<const char*, float&, Callable&&>>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionType, int(const char*, float&, Callable&&)>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionPointerType,
      decltype(&TestObject::Function)>::value));
  EXPECT_FALSE(T::kIsConst);
  EXPECT_FALSE(T::kIsVolatile);
  EXPECT_EQ(3, T::kNumArgs);
}

TEST(FunctionTraits, MemberConstFunctionPointer) {
  using T = FunctionTraits<decltype(&TestObject::ConstFunction)>;
  EXPECT_TRUE((std::is_same<T::Class, const TestObject>::value));
  EXPECT_TRUE((std::is_same<T::Return, int>::value));
  EXPECT_TRUE((std::is_same<
      T::Args, std::tuple<const char*, float&, Callable&&>>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionType, int(const char*, float&, Callable&&)>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionPointerType,
      decltype(&TestObject::ConstFunction)>::value));
  EXPECT_TRUE(T::kIsConst);
  EXPECT_FALSE(T::kIsVolatile);
  EXPECT_EQ(3, T::kNumArgs);
}

TEST(FunctionTraits, MemberVolatileFunctionPointer) {
  using T = FunctionTraits<decltype(&TestObject::VolatileFunction)>;
  EXPECT_TRUE((std::is_same<T::Class, volatile TestObject>::value));
  EXPECT_TRUE((std::is_same<T::Return, int>::value));
  EXPECT_TRUE((std::is_same<
      T::Args, std::tuple<const char*, float&, Callable&&>>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionType, int(const char*, float&, Callable&&)>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionPointerType,
      decltype(&TestObject::VolatileFunction)>::value));
  EXPECT_FALSE(T::kIsConst);
  EXPECT_TRUE(T::kIsVolatile);
  EXPECT_EQ(3, T::kNumArgs);
}

TEST(FunctionTraits, MemberConstVolatileFunctionPointer) {
  using T = FunctionTraits<decltype(&TestObject::ConstVolatileFunction)>;
  EXPECT_TRUE((std::is_same<T::Class, const volatile TestObject>::value));
  EXPECT_TRUE((std::is_same<T::Return, int>::value));
  EXPECT_TRUE((std::is_same<
      T::Args, std::tuple<const char*, float&, Callable&&>>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionType, int(const char*, float&, Callable&&)>::value));
  EXPECT_TRUE((std::is_same<
      T::FunctionPointerType,
      decltype(&TestObject::ConstVolatileFunction)>::value));
  EXPECT_TRUE(T::kIsConst);
  EXPECT_TRUE(T::kIsVolatile);
  EXPECT_EQ(3, T::kNumArgs);
}

TEST(FunctionTraits, CallableType) {
  auto lambda = [](int x, int y) { return x + y; };

  EXPECT_TRUE((std::is_same<
      typename FunctionTraits<CallableType<decltype(lambda)>>::FunctionType,
      int(int, int)>::value));
  EXPECT_TRUE((std::is_same<
      typename FunctionTraits<CallableType<Callable>>::FunctionType,
      const char*(const int&, int&&)>::value));
}
