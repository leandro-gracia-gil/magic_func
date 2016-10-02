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

#include <memory>

#include <magic_func/type_traits.h>
#include <gtest/gtest.h>

#include "test_common.h"

using namespace mf;
using namespace mf::test;

class Class;

TEST(TypeTraits, IsFunctionPointer) {
  EXPECT_TRUE(IsFunctionPointer<decltype(&FreeFunction)>::value);
  EXPECT_TRUE(IsFunctionPointer<decltype(&FreeFunction)&>::value);
  EXPECT_TRUE(IsFunctionPointer<decltype(&FreeFunction)&&>::value);
  EXPECT_TRUE(IsFunctionPointer<void (*)()>::value);

  EXPECT_FALSE(IsFunctionPointer<decltype(&Object::Function)>::value);
  EXPECT_FALSE(IsFunctionPointer<void (Object::*)()>::value);
  EXPECT_FALSE(IsFunctionPointer<void()>::value);
  EXPECT_FALSE(IsFunctionPointer<void*>::value);
}

TEST(TypeTraits, IsFunction) {
  EXPECT_TRUE(IsFunction<Function<void()>>::value);
  EXPECT_TRUE(IsFunction<Function<void()>&>::value);
  EXPECT_TRUE(IsFunction<Function<void()>&&>::value);
  EXPECT_TRUE(IsFunction<Function<void()> const>::value);
  EXPECT_TRUE(IsFunction<Function<void (*)()>>::value);
  EXPECT_TRUE(IsFunction<Function<Function<int>>>::value);

  EXPECT_FALSE(IsFunction<int>::value);
  EXPECT_FALSE(IsFunction<MemberFunction<void (Class::*)()>>::value);
}

TEST(TypeTraits, IsMemberFunction) {
  EXPECT_TRUE(IsMemberFunction<MemberFunction<void (Class::*)()>>::value);
  EXPECT_TRUE(IsMemberFunction<MemberFunction<void (Class::*)()&>>::value);
  EXPECT_TRUE(IsMemberFunction<MemberFunction<void (Class::*)()&&>>::value);
  EXPECT_TRUE(IsMemberFunction<MemberFunction<void (Class::*)() const>>::value);
  EXPECT_TRUE(IsMemberFunction<MemberFunction<void (*)()>>::value);
  EXPECT_TRUE(IsMemberFunction<MemberFunction<MemberFunction<int>>>::value);

  EXPECT_FALSE(IsMemberFunction<int>::value);
  EXPECT_FALSE(IsMemberFunction<Function<void(int)>>::value);
}

TEST(TypeTraits, IsSharedPtr) {
  EXPECT_TRUE(IsSharedPtr<std::shared_ptr<int>>::value);
  EXPECT_TRUE(IsSharedPtr<std::shared_ptr<int>&>::value);
  EXPECT_TRUE(IsSharedPtr<std::shared_ptr<int>&&>::value);
  EXPECT_TRUE(IsSharedPtr<std::shared_ptr<int> const>::value);
  EXPECT_TRUE(IsSharedPtr<std::shared_ptr<const int>>::value);
  EXPECT_TRUE(IsSharedPtr<std::shared_ptr<const int*>>::value);
  EXPECT_TRUE(IsSharedPtr<std::shared_ptr<std::shared_ptr<int>>>::value);

  EXPECT_FALSE(IsSharedPtr<int>::value);
}
