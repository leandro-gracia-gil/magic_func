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

// This test needs C++ exceptions thrown by MagicFunc exceptions to work.
// These exceptions are turned off in release builds that define NDEBUG.
#undef NDEBUG

#include <magic_func/error.h>
#include <magic_func/make_function.h>
#include <magic_func/member_function.h>
#include <magic_func/type_id.h>
#include <gtest/gtest.h>

#include "test_common.h"

using namespace mf;
using namespace mf::test;

TEST(MemberFunction, Empty) {
  MemberFunction<decltype(&Object::Function)> member_function;
  EXPECT_FALSE(member_function);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ(GetTypeId<decltype(&Object::Function)>(),
            member_function.type_id());

  // Test calling an empty function.
  try {
    Object object(0);
    bool called;

    member_function(object, called, true);
    FAIL();

  } catch (Error error) {
    EXPECT_EQ(Error::kInvalidFunction, error);
  }
}

TEST(MemberFunction, CallMemberFunction) {
  MemberFunction<decltype(&Object::Function)>
      member_function = MF_MakeFunction(&Object::Function);

  EXPECT_TRUE(member_function);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ(GetTypeId<decltype(&Object::Function)>(),
            member_function.type_id());

  {
    // Test calling regular member functions with regular objects.
    int id = rand();
    Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling after copying and moving the member function.
    int id = rand();
    Object object(id);

    auto member_function_copy = member_function;
    EXPECT_TRUE(member_function_copy);
    EXPECT_EQ(nullptr, member_function_copy.GetObject());
    EXPECT_EQ(GetTypeId<decltype(&Object::Function)>(),
              member_function_copy.type_id());

    bool called = false;
    EXPECT_EQ(id, member_function_copy(object, called, true));
    EXPECT_TRUE(called);

    auto member_function_move = std::move(member_function_copy);
    EXPECT_TRUE(member_function_move);
    EXPECT_EQ(nullptr, member_function_move.GetObject());
    EXPECT_EQ(GetTypeId<decltype(&Object::Function)>(),
              member_function_move.type_id());

    called = false;
    EXPECT_EQ(id, member_function_move(object, called, true));
    EXPECT_TRUE(called);
  }
}

TEST(MemberFunction, CallConstMemberFunction) {
  MemberFunction<decltype(&Object::ConstFunction)>
      member_function = MF_MakeFunction(&Object::ConstFunction);

  EXPECT_TRUE(member_function);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ(GetTypeId<decltype(&Object::ConstFunction)>(),
            member_function.type_id());

  {
    // Test calling const member functions with regular objects.
    int id = rand();
    Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling const member functions with const objects.
    int id = rand();
    const Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling after copying and moving the member function.
    int id = rand();
    const Object object(id);

    auto member_function_copy = member_function;
    EXPECT_TRUE(member_function_copy);
    EXPECT_EQ(nullptr, member_function_copy.GetObject());
    EXPECT_EQ(GetTypeId<decltype(&Object::ConstFunction)>(),
              member_function_copy.type_id());

    bool called = false;
    EXPECT_EQ(id, member_function_copy(object, called, true));
    EXPECT_TRUE(called);

    auto member_function_move = std::move(member_function_copy);
    EXPECT_TRUE(member_function_move);
    EXPECT_EQ(nullptr, member_function_move.GetObject());
    EXPECT_EQ(GetTypeId<decltype(&Object::ConstFunction)>(),
              member_function_move.type_id());

    called = false;
    EXPECT_EQ(id, member_function_move(object, called, true));
    EXPECT_TRUE(called);
  }
}

TEST(MemberFunction, CallVolatileMemberFunction) {
  MemberFunction<decltype(&Object::VolatileFunction)>
      member_function = MF_MakeFunction(&Object::VolatileFunction);

  EXPECT_TRUE(member_function);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ(GetTypeId<decltype(&Object::VolatileFunction)>(),
            member_function.type_id());

  {
    // Test calling volatile member functions with regular objects.
    int id = rand();
    Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling volatile member functions with volatile objects.
    int id = rand();
    volatile Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling after copying and moving the member function.
    int id = rand();
    volatile Object object(id);

    auto member_function_copy = member_function;
    EXPECT_TRUE(member_function_copy);
    EXPECT_EQ(nullptr, member_function_copy.GetObject());
    EXPECT_EQ(GetTypeId<decltype(&Object::VolatileFunction)>(),
              member_function_copy.type_id());

    bool called = false;
    EXPECT_EQ(id, member_function_copy(object, called, true));
    EXPECT_TRUE(called);

    auto member_function_move = std::move(member_function_copy);
    EXPECT_TRUE(member_function_move);
    EXPECT_EQ(nullptr, member_function_move.GetObject());
    EXPECT_EQ(GetTypeId<decltype(&Object::VolatileFunction)>(),
              member_function_move.type_id());

    called = false;
    EXPECT_EQ(id, member_function_move(object, called, true));
    EXPECT_TRUE(called);
  }
}

TEST(MemberFunction, CallConstVolatileMemberFunction) {
  MemberFunction<decltype(&Object::ConstVolatileFunction)>
      member_function = MF_MakeFunction(&Object::ConstVolatileFunction);

  EXPECT_TRUE(member_function);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ(GetTypeId<decltype(&Object::ConstVolatileFunction)>(),
            member_function.type_id());

  {
    // Test calling const volatile member functions with regular objects.
    int id = rand();
    Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling const volatile member functions with volatile objects.
    int id = rand();
    volatile Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling const volatile member functions with const objects.
    int id = rand();
    const Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling const volatile member functions with const volatile objects.
    int id = rand();
    const volatile Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling after copying and moving the member function.
    int id = rand();
    volatile Object object(id);

    auto member_function_copy = member_function;
    EXPECT_TRUE(member_function_copy);
    EXPECT_EQ(nullptr, member_function_copy.GetObject());
    EXPECT_EQ(GetTypeId<decltype(&Object::ConstVolatileFunction)>(),
              member_function_copy.type_id());

    bool called = false;
    EXPECT_EQ(id, member_function_copy(object, called, true));
    EXPECT_TRUE(called);

    auto member_function_move = std::move(member_function_copy);
    EXPECT_TRUE(member_function_move);
    EXPECT_EQ(nullptr, member_function_move.GetObject());
    EXPECT_EQ(GetTypeId<decltype(&Object::ConstVolatileFunction)>(),
              member_function_move.type_id());

    called = false;
    EXPECT_EQ(id, member_function_move(object, called, true));
    EXPECT_TRUE(called);
  }
}

TEST(MemberFunction, CallVirtualMemberFunction) {
  MemberFunction<decltype(&Object::VirtualFunction)>
      member_function = MF_MakeFunction(&Object::VirtualFunction);

  EXPECT_TRUE(member_function);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ(GetTypeId<decltype(&Object::VirtualFunction)>(),
            member_function.type_id());

  {
    // Test calling a virtual function with a base object.
    int id = rand();
    Object object(id);

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, member_function(object, called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_FALSE(derived);
  }

  {
    // Test calling const member functions with const objects.
    int id = rand();
    DerivedObject object(id);

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, member_function(object, called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_TRUE(derived);
  }
}
