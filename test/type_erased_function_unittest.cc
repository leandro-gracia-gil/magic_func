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
#include <magic_func/make_function.h>
#include <magic_func/type_erased_function.h>
#include <gtest/gtest.h>

#include "test_common.h"

using namespace mf;
using namespace mf::test;

TEST(TypeErasedFunction, Empty) {
  TypeErasedFunction function;
  EXPECT_FALSE(function);
  EXPECT_EQ(function, nullptr);
  EXPECT_EQ(0, function.type_id());
  EXPECT_EQ(nullptr, function.GetObject());
}

TEST(TypeErasedFunction, Assign) {
  auto function = MF_MakeFunction(&FreeFunction);
  auto member_function = MF_MakeFunction(&Object::Function);

  try {
    TypeErasedFunction type_erased_function;

    // Assigning to another empty type-erased function should work because there
    // is no type set yet.
    type_erased_function = TypeErasedFunction();
    SUCCEED();
    EXPECT_FALSE(type_erased_function);
    EXPECT_EQ(type_erased_function, nullptr);
    EXPECT_EQ(0, type_erased_function.type_id());

    // Assigning a new type should work.
    type_erased_function = function;
    SUCCEED();
    EXPECT_TRUE(type_erased_function);
    EXPECT_NE(type_erased_function, nullptr);
    EXPECT_NE(0, type_erased_function.type_id());

    // Assigning a new Function of the same type should work.
    type_erased_function = MF_MakeFunction(&FreeFunction);
    SUCCEED();

    // This should throw an invalid cast excepton.
    type_erased_function = member_function;
    FAIL();

  } catch (Error error) {
    EXPECT_EQ(Error::kIncompatibleType, error);
  }

  try {
    TypeErasedFunction type_erased_function;
    type_erased_function = function;
    SUCCEED();

    // Trying to assign a typeless type-erased function when it already as a
    // type should throw an exception.
    type_erased_function = TypeErasedFunction();
    FAIL();

  } catch (Error error) {
    EXPECT_EQ(Error::kIncompatibleType, error);
  }

  try {
    TypeErasedFunction type_erased_function;
    type_erased_function = member_function;
    SUCCEED();

    // Member functions with same argument and return types but different
    // qualifications are considered different types. This should fail.
    type_erased_function = MF_MakeFunction(&Object::ConstFunction);
    FAIL();

  } catch (Error error) {
    EXPECT_EQ(Error::kIncompatibleType, error);
  }
}
