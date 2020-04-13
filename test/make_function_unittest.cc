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

#include <magic_func/make_function.h>
#include <magic_func/type_id.h>
#include <gtest/gtest.h>

#include "test_common.h"

using namespace mf;
using namespace mf::test;

TEST(MakeFunction, FunctionAddress) {
  auto function = MF_MakeFunction(&FreeFunction);
  EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
              decltype(function)>::value));

  EXPECT_TRUE(function);
  EXPECT_EQ(nullptr, function.GetObject());
  EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

  bool called = false;
  EXPECT_EQ(42, function(called, true));
  EXPECT_TRUE(called);
}

TEST(MakeFunction, Lambda) {
  {
    // Test creating a function from a lambda lvalue reference.
    int id = rand();
    auto lambda = [=](bool& called, bool&& value) {
      called = std::move(value);
      return id;
    };

    auto function = MakeFunction(lambda);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));

    EXPECT_TRUE(function);
    EXPECT_NE(nullptr, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a function from a lambda rvalue reference.
    int id = rand();
    auto function = MakeFunction([=](bool& called, bool&& value) {
      called = std::move(value);
      return id;
    });

    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));

    EXPECT_TRUE(function);
    EXPECT_NE(nullptr, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }
}

TEST(MakeFunction, MemberFunctionAndObject) {
  auto member_function = MF_MakeFunction(&Object::Function);
  auto member_function_const = MF_MakeFunction(&Object::ConstFunction);

  {
    // Test non-const functions with object pointers.
    int id = rand();
    Object object(id);
    auto function = MakeFunction(member_function, &object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(&object, function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test const functions with object pointers.
    int id = rand();
    Object object(id);
    auto function = MakeFunction(member_function_const, &object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(&object, function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test const functions with const object pointers.
    int id = rand();
    const Object const_object(id);
    auto function = MakeFunction(member_function_const, &const_object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(&const_object, function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test non-const functions with shared object pointers.
    int id = rand();
    auto shared_object = std::make_shared<Object>(id);
    auto function = MakeFunction(member_function, shared_object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(shared_object.get(), function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test const functions with shared object pointers.
    int id = rand();
    auto shared_object = std::make_shared<Object>(id);
    auto function = MakeFunction(member_function_const, shared_object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(shared_object.get(), function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test const functions with shared const object pointers.
    int id = rand();
    auto shared_const_object = std::make_shared<const Object>(id);
    auto function = MakeFunction(member_function_const, shared_const_object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(shared_const_object.get(), function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }
}

TEST(MakeFunction, MemberFunctionAddressAndObject) {
  {
    // Test non-const functions with object pointers.
    int id = rand();
    Object object(id);
    auto function = MF_MakeFunction(&Object::Function, &object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(&object, function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test const functions with object pointers.
    int id = rand();
    Object object(id);
    auto function = MF_MakeFunction(&Object::ConstFunction, &object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(&object, function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test const functions with const object pointers.
    int id = rand();
    const Object const_object(id);
    auto function = MF_MakeFunction(&Object::ConstFunction, &const_object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(&const_object, function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test non-const functions with shared object pointers.
    int id = rand();
    auto shared_object = std::make_shared<Object>(id);
    auto function = MF_MakeFunction(&Object::Function, shared_object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(shared_object.get(), function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test const functions with shared object pointers.
    int id = rand();
    auto shared_object = std::make_shared<Object>(id);
    auto function = MF_MakeFunction(&Object::ConstFunction, shared_object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(shared_object.get(), function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test const functions with shared const object pointers.
    int id = rand();
    auto shared_const_object = std::make_shared<const Object>(id);
    auto function = MF_MakeFunction(
        &Object::ConstFunction, shared_const_object);
    EXPECT_TRUE((std::is_same<Function<int(bool&, bool&&)>,
                decltype(function)>::value));
    EXPECT_EQ(shared_const_object.get(), function.GetObject());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }
}

TEST(MakeFunction, MemberFunctionAddress) {
  {
    // Test creating a MemberFunction for a member function address.
    auto member_function = MF_MakeFunction(&Object::Function);
    EXPECT_TRUE((std::is_same<MemberFunction<decltype(&Object::Function)>,
                decltype(member_function)>::value));
    EXPECT_FALSE(decltype(member_function)::kIsConst);
    EXPECT_FALSE(decltype(member_function)::kIsVolatile);

    EXPECT_TRUE(member_function);
    EXPECT_EQ(nullptr, member_function.GetObject());
    EXPECT_EQ((get_type_id<decltype(&Object::Function)>()),
              member_function.type_id());

    int id = rand();
    Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a MemberFunction to a const member function address.
    auto member_function = MF_MakeFunction(&Object::ConstFunction);
    EXPECT_TRUE((std::is_same<MemberFunction<decltype(&Object::ConstFunction)>,
                decltype(member_function)>::value));
    EXPECT_TRUE(decltype(member_function)::kIsConst);
    EXPECT_FALSE(decltype(member_function)::kIsVolatile);

    EXPECT_TRUE(member_function);
    EXPECT_EQ(nullptr, member_function.GetObject());
    EXPECT_EQ((get_type_id<decltype(&Object::ConstFunction)>()),
              member_function.type_id());

    int id = rand();
    Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);

    int const_id = rand();
    const Object const_object(const_id);

    called = false;
    EXPECT_EQ(const_id, member_function(const_object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a MemberFunction to a volatile member function address.
    auto member_function = MF_MakeFunction(&Object::VolatileFunction);
    EXPECT_TRUE(
        (std::is_same<MemberFunction<decltype(&Object::VolatileFunction)>,
        decltype(member_function)>::value));
    EXPECT_FALSE(decltype(member_function)::kIsConst);
    EXPECT_TRUE(decltype(member_function)::kIsVolatile);

    EXPECT_TRUE(member_function);
    EXPECT_EQ(nullptr, member_function.GetObject());
    EXPECT_EQ((get_type_id<decltype(&Object::VolatileFunction)>()),
              member_function.type_id());

    int id = rand();
    Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);

    int volatile_id = rand();
    volatile Object volatile_object(volatile_id);

    called = false;
    EXPECT_EQ(volatile_id, member_function(volatile_object, called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a MemberFunction to a const volatile member function
    // address.
    auto member_function = MF_MakeFunction(&Object::ConstVolatileFunction);
    EXPECT_TRUE(
        (std::is_same<MemberFunction<decltype(&Object::ConstVolatileFunction)>,
        decltype(member_function)>::value));
    EXPECT_TRUE(decltype(member_function)::kIsConst);
    EXPECT_TRUE(decltype(member_function)::kIsVolatile);

    EXPECT_TRUE(member_function);
    EXPECT_EQ(nullptr, member_function.GetObject());
    EXPECT_EQ((get_type_id<decltype(&Object::ConstVolatileFunction)>()),
              member_function.type_id());

    int id = rand();
    Object object(id);

    bool called = false;
    EXPECT_EQ(id, member_function(object, called, true));
    EXPECT_TRUE(called);

    int const_id = rand();
    const Object const_object(const_id);

    called = false;
    EXPECT_EQ(const_id, member_function(const_object, called, true));
    EXPECT_TRUE(called);

    int volatile_id = rand();
    volatile Object volatile_object(volatile_id);

    called = false;
    EXPECT_EQ(volatile_id, member_function(volatile_object, called, true));
    EXPECT_TRUE(called);

    int const_volatile_id = rand();
    const volatile Object const_volatile_object(const_volatile_id);

    called = false;
    EXPECT_EQ(const_volatile_id,
              member_function(const_volatile_object, called, true));
    EXPECT_TRUE(called);
  }
}
