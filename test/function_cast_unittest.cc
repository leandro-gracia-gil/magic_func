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

#include <magic_func/error.h>
#include <magic_func/function.h>
#include <magic_func/function_cast.h>
#include <magic_func/make_function.h>
#include <magic_func/type_erased_function.h>
#include <magic_func/type_id.h>
#include <gtest/gtest.h>

#include "test_common.h"

using namespace mf;
using namespace mf::test;

TEST(FunctionCast, FunctionAddress) {
  // Test casting back from a type-erased Function pointing to a function
  // address.
  auto function = MF_MakeFunction(&FreeFunction);
  TypeErasedFunction type_erased = function;
  EXPECT_TRUE(type_erased);

  auto casted_function = FunctionCast<int(bool&, bool&&)>(type_erased);
  EXPECT_TRUE((std::is_same<decltype(casted_function),
              Function<int(bool&, bool&&)>>::value));

  bool called = false;
  EXPECT_EQ(42, casted_function(called, true));
  EXPECT_TRUE(called);

  // Try catching a bad cast.
  try {
    FunctionCast<int(bool, bool)>(type_erased);
    FAIL();
  } catch (Error error) {
    EXPECT_EQ(Error::kInvalidCast, error);
  }
}

TEST(FunctionCast, FunctionCallable) {
  // Test casting back from a type-erased Function that calls a lambda.
  int id = rand();
  Function<int(bool&, bool&&)> function = [=](bool& called, bool&& value) {
    called = std::move(value);
    return id;
  };

  TypeErasedFunction type_erased = function;
  EXPECT_TRUE(type_erased);

  auto casted_function = FunctionCast<int(bool&, bool&&)>(type_erased);
  EXPECT_TRUE((std::is_same<decltype(casted_function),
              Function<int(bool&, bool&&)>>::value));

  bool called = false;
  EXPECT_EQ(id, casted_function(called, true));
  EXPECT_TRUE(called);

  // Try catching a bad cast.
  try {
    FunctionCast<int(bool, bool)>(type_erased);
    FAIL();
  } catch (Error error) {
    EXPECT_EQ(Error::kInvalidCast, error);
  }
}

TEST(FunctionCast, MemberFunctionAndObject) {
  auto member_function = MF_MakeFunction(&Object::Function);

  {
    // Test casting back from a type-erased Function built by binding a
    // MemberFunction and an object pointer.
    int id = rand();
    Object object(id);
    Function<int(bool&, bool&&)> function(member_function, &object);

    TypeErasedFunction type_erased = function;
    EXPECT_TRUE(type_erased);

    auto casted_function = FunctionCast<int(bool&, bool&&)>(type_erased);
    EXPECT_TRUE((std::is_same<decltype(casted_function),
                Function<int(bool&, bool&&)>>::value));

    bool called = false;
    EXPECT_EQ(id, casted_function(called, true));
    EXPECT_TRUE(called);

    // Try catching a bad cast.
    try {
      FunctionCast<decltype(&Object::Function)>(type_erased);
      FAIL();
    } catch (Error error) {
      EXPECT_EQ(Error::kInvalidCast, error);
    }
  }

  {
    // Test casting back from a type-erased Function built by binding a
    // MemberFunction and an object shared pointer.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    Function<int(bool&, bool&&)> function(member_function, object);

    TypeErasedFunction type_erased = function;
    EXPECT_TRUE(type_erased);

    auto casted_function = FunctionCast<int(bool&, bool&&)>(type_erased);
    EXPECT_TRUE((std::is_same<decltype(casted_function),
                Function<int(bool&, bool&&)>>::value));

    bool called = false;
    EXPECT_EQ(id, casted_function(called, true));
    EXPECT_TRUE(called);

    // Try catching a bad cast.
    try {
      FunctionCast<decltype(&Object::Function)>(type_erased);
      FAIL();
    } catch (Error error) {
      EXPECT_EQ(Error::kInvalidCast, error);
    }
  }
}

TEST(FunctionCast, MemberFunctionAddressAndObject) {
  {
    // Test casting back from a type-erased Function built by binding a member
    // function address and an object pointer.
    int id = rand();
    Object object(id);
    auto function = MF_MakeFunction(&Object::Function, &object);

    TypeErasedFunction type_erased = function;
    EXPECT_TRUE(type_erased);

    auto casted_function = FunctionCast<int(bool&, bool&&)>(type_erased);
    EXPECT_TRUE((std::is_same<decltype(casted_function),
                Function<int(bool&, bool&&)>>::value));

    bool called = false;
    EXPECT_EQ(id, casted_function(called, true));
    EXPECT_TRUE(called);

    // Try catching a bad cast.
    try {
      FunctionCast<decltype(&Object::Function)>(type_erased);
      FAIL();
    } catch (Error error) {
      EXPECT_EQ(Error::kInvalidCast, error);
    }
  }

  {
    // Test casting back from a type-erased Function built by binding a member
    // function address and an object shared pointer.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    auto function = MF_MakeFunction(&Object::Function, object);

    TypeErasedFunction type_erased = function;
    EXPECT_TRUE(type_erased);

    auto casted_function = FunctionCast<int(bool&, bool&&)>(type_erased);
    EXPECT_TRUE((std::is_same<decltype(casted_function),
                Function<int(bool&, bool&&)>>::value));

    bool called = false;
    EXPECT_EQ(id, casted_function(called, true));
    EXPECT_TRUE(called);

    // Try catching a bad cast.
    try {
      FunctionCast<decltype(&Object::Function)>(type_erased);
      FAIL();
    } catch (Error error) {
      EXPECT_EQ(Error::kInvalidCast, error);
    }
  }
}

TEST(FunctionCast, MemberFunctionAddress) {
  // Test casting back from a type-erased MemberFunction built from a member
  // function address.
  auto member_function = MF_MakeFunction(&Object::Function);
  TypeErasedFunction type_erased = member_function;
  EXPECT_TRUE(type_erased);

  auto casted_function = FunctionCast<decltype(&Object::Function)>(type_erased);
  EXPECT_TRUE((std::is_same<decltype(casted_function),
              MemberFunction<decltype(&Object::Function)>>::value));

  int id = rand();
  Object object(id);

  bool called = false;
  EXPECT_EQ(id, casted_function(object, called, true));
  EXPECT_TRUE(called);

  // Try catching a bad cast.
  try {
    FunctionCast<decltype(&Object::ConstFunction)>(type_erased);
    FAIL();
  } catch (Error error) {
    EXPECT_EQ(Error::kInvalidCast, error);
  }
}
