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

#include <functional>
#include <type_traits>

#include <magic_func/error.h>
#include <magic_func/function.h>
#include <magic_func/function_cast.h>
#include <magic_func/make_function.h>
#include <magic_func/type_id.h>
#include <gtest/gtest.h>

#include "test_common.h"

using namespace mf;
using namespace mf::test;

// Some versions of llvm-libc++ are affected by a bug that prevents from
// creating a volatile shared pointer. This affects some tests here.
//
// https://llvm.org/bugs/show_bug.cgi?id=23647
//
// Define this macro in order to disable such tests.
//#define DISABLE_VOLATILE_SHARED_PTRS

TEST(Function, Empty) {
  Function<void()> function;
  EXPECT_FALSE(function);
  EXPECT_EQ(nullptr, function.GetObject());
  EXPECT_EQ(get_type_id<void()>(), function.type_id());

  // Test calling an empty function.
  try {
    function();
    FAIL();
  } catch (Error error) {
    EXPECT_EQ(Error::kInvalidFunction, error);
  }
}

TEST(Function, FreeFunctionCall) {
  // Try creating a function with its explicit type and calling it.
  Function<int(bool&, bool&&)> function = MF_MakeFunction(&FreeFunction);
  EXPECT_TRUE((std::is_same<decltype(&FreeFunction),
              typename decltype(function)::FunctionPointerType>::value));

  EXPECT_TRUE(function);
  EXPECT_EQ(nullptr, function.GetObject());
  EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

  bool called = false;
  EXPECT_EQ(42, function(called, true));
  EXPECT_TRUE(called);

  // Try calling a copy of the function.
  auto function_copy = function;
  called = false;
  EXPECT_EQ(42, function_copy(called, true));
  EXPECT_TRUE(called);

  // Try calling a move of the function.
  auto function_move = std::move(function_copy);
  called = false;
  EXPECT_EQ(42, function_move(called, true));
  EXPECT_TRUE(called);

  // Reset the function.
  function = nullptr;
  EXPECT_FALSE(function);
  EXPECT_EQ(nullptr, function.GetObject());
  EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());
}

TEST(Function, LambdaCall) {
  // Try creating a function directly from a lambda and calling it.
  int id = rand();
  auto lambda = [=](bool& called, bool&& value) {
    called = std::move(value);
    return id;
  };

  Function<int(bool&, bool&&)> function(lambda);
  EXPECT_TRUE(function);
  EXPECT_NE(nullptr, function.GetObject());
  EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

  bool called = false;
  EXPECT_EQ(id, function(called, true));
  EXPECT_TRUE(called);

  // Try explicitly assigning the lambda on function construction.
  Function<int(bool&, bool&&)> function_lambda_construct = lambda;

  called = false;
  EXPECT_EQ(id, function_lambda_construct(called, true));
  EXPECT_TRUE(called);

  // Try usign copy assignment with the lambda.
  Function<int(bool&, bool&&)> function_lambda_copy;
  function_lambda_copy = lambda;

  called = false;
  EXPECT_EQ(id, function_lambda_copy(called, true));
  EXPECT_TRUE(called);

  // Try usign move assignment with the lambda.
  Function<int(bool&, bool&&)> function_lambda_move;
  function_lambda_move = std::move(function_lambda_copy);

  called = false;
  EXPECT_EQ(id, function_lambda_move(called, true));
  EXPECT_TRUE(called);

  // Try assigning a const lambda.
  const auto& const_lambda = lambda;
  Function<int(bool&, bool&&)> function_const_lambda;
  function_const_lambda = const_lambda;

  called = false;
  EXPECT_EQ(id, function_const_lambda(called, true));
  EXPECT_TRUE(called);

  // Try calling a copy of the function.
  auto function_copy = function;
  called = false;
  EXPECT_EQ(id, function_copy(called, true));
  EXPECT_TRUE(called);

  // Try calling a move of the function.
  auto function_move = std::move(function_copy);
  called = false;
  EXPECT_EQ(id, function_move(called, true));
  EXPECT_TRUE(called);

  // Reset the function.
  function = nullptr;
  EXPECT_FALSE(function);
  EXPECT_EQ(nullptr, function.GetObject());
  EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());
}

TEST(Function, LambdaConvertible) {
  // Test that functions can be initialized to lambdas as long as argument and
  // return types are convertible.
  auto lambda = [](const std::string& str, int i) { return str[i]; };

  EXPECT_TRUE((std::is_same<decltype(make_function(lambda)),
              Function<char(const std::string&, int)>>::value));

  {
    // Test that functions can be initialized to lambdas as long as they
    // argument and result types are convertible.
    Function<short(const char[], float)> function = lambda;
    EXPECT_EQ((short)('e'), function("test", 1));
  }

  {
    // Test that functions can be assigned to lambdas as long as they argument
    // and result types are convertible.
    Function<short(const char[], float)> function;
    function = lambda;
    EXPECT_EQ((short)('e'), function("test", 1));
  }
}

TEST(Function, LambdaMutable) {
  // Create a function for a mutable lambda.
  size_t call_count = 0;
  auto function = make_function([=]() mutable { return call_count++; });

  // Call it 50 times.
  for (size_t i = 0; i < 50; ++i)
    EXPECT_EQ(i, function());

  // Copy the function. This should make a copy of the stored lambda object.
  auto function_copy = function;

  // Call the original 50 more times.
  for (size_t i = 0; i < 50; ++i)
    EXPECT_EQ(50 + i, function());

  // If we call the copy, its state should be separate from the original.
  EXPECT_EQ(100, function());
  EXPECT_EQ(50, function_copy());

  // If we move the copy, we replace the state of the original.
  function = std::move(function_copy);
  EXPECT_EQ(51, function());
}

TEST(Function, CallableOverload) {
  // Test calling the non-qualified operator ().
  {
    int id = rand();
    OverloadedCallable callable(id);
    Function<int(CVQualification&)> function = callable;

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kNonQualified, cv);
  }

  // Test calling the const operator ().
  {
    int id = rand();
    const OverloadedCallable callable(id);
    Function<int(CVQualification&)> function = callable;

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kConstQualified, cv);
  }

  // Test calling the volatile operator ().
  {
    int id = rand();
    volatile OverloadedCallable callable(id);
    Function<int(CVQualification&)> function = callable;

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kVolatileQualified, cv);
  }

  // Test calling the const volatile operator ().
  {
    int id = rand();
    const volatile OverloadedCallable callable(id);
    Function<int(CVQualification&)> function = callable;

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kConstVolatileQualified, cv);
  }
}

TEST(Function, FromMemberFunctionAndObjectPointer) {
  // Create a MemberFunction pointing to a member function.
  auto member_function = MF_MakeFunction(&Object::Function);
  EXPECT_TRUE(IsMemberFunction<decltype(member_function)>::value);
  EXPECT_TRUE(member_function);
  EXPECT_FALSE(decltype(member_function)::kIsConst);
  EXPECT_FALSE(decltype(member_function)::kIsVolatile);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::Function)>()),
            member_function.type_id());

  // Create a MemberFunction pointing to a const member function.
  auto const_member_function = MF_MakeFunction(&Object::ConstFunction);
  EXPECT_TRUE(IsMemberFunction<decltype(const_member_function)>::value);
  EXPECT_TRUE(const_member_function);
  EXPECT_TRUE(decltype(const_member_function)::kIsConst);
  EXPECT_FALSE(decltype(const_member_function)::kIsVolatile);
  EXPECT_EQ(nullptr, const_member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::ConstFunction)>()),
            const_member_function.type_id());

  // Create a MemberFunction pointing to a volatile member function.
  auto volatile_member_function = MF_MakeFunction(&Object::VolatileFunction);
  EXPECT_TRUE(IsMemberFunction<decltype(volatile_member_function)>::value);
  EXPECT_TRUE(volatile_member_function);
  EXPECT_FALSE(decltype(volatile_member_function)::kIsConst);
  EXPECT_TRUE(decltype(volatile_member_function)::kIsVolatile);
  EXPECT_EQ(nullptr, volatile_member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::VolatileFunction)>()),
            volatile_member_function.type_id());

  // Create a MemberFunction pointing to a const_volatile member function.
  auto const_volatile_member_function =
      MF_MakeFunction(&Object::ConstVolatileFunction);
  EXPECT_TRUE(IsMemberFunction<
      decltype(const_volatile_member_function)>::value);
  EXPECT_TRUE(const_volatile_member_function);
  EXPECT_TRUE(decltype(const_volatile_member_function)::kIsConst);
  EXPECT_TRUE(decltype(const_volatile_member_function)::kIsVolatile);
  EXPECT_EQ(nullptr, const_volatile_member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::ConstVolatileFunction)>()),
            const_volatile_member_function.type_id());

  {
    // Test creating a Function from a MemberFunction pointing to a member
    // function and binding an object pointer to it.
    int id = rand();
    Object object(id);
    Function<int(bool&, bool&&)> function(member_function, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member const
    // function and binding an object pointer to it.
    int id = rand();
    Object object(id);
    Function<int(bool&, bool&&)> function(const_member_function, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member
    // volatile function and binding an object pointer to it.
    int id = rand();
    Object object(id);
    Function<int(bool&, bool&&)> function(volatile_member_function, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member const
    // volatile function and binding an object pointer to it.
    int id = rand();
    Object object(id);
    Function<int(bool&, bool&&)> function(
        const_volatile_member_function, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member const
    // function and binding a const object pointer to it.
    int id = rand();
    const Object object(id);
    Function<int(bool&, bool&&)> function(const_member_function, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member const
    // volatile function and binding a const object pointer to it.
    int id = rand();
    const Object object(id);
    Function<int(bool&, bool&&)> function(
        const_volatile_member_function, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member
    // volatile function and binding a volatile object pointer to it.
    int id = rand();
    volatile Object object(id);
    Function<int(bool&, bool&&)> function(volatile_member_function, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member
    // const volatile function and binding a const volatile object pointer.
    int id = rand();
    const volatile Object object(id);
    Function<int(bool&, bool&&)> function(
        const_volatile_member_function, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling after copying and moving the function.
    int id = rand();
    Object object(id);
    Function<int(bool&, bool&&)> function(member_function, &object);

    auto function_copy = function;
    EXPECT_TRUE(function_copy);
    EXPECT_EQ(&object, function_copy.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function_copy.type_id());

    bool called = false;
    EXPECT_EQ(id, function_copy(called, true));
    EXPECT_TRUE(called);

    auto function_move = std::move(function_copy);
    EXPECT_TRUE(function_move);
    EXPECT_EQ(&object, function_move.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function_move.type_id());

    called = false;
    EXPECT_EQ(id, function_move(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a function with an invalid object.
    try {
      Object* object = nullptr;
      Function<int(bool&, bool&&)> function(member_function, object);
      FAIL();
    } catch (Error error) {
      EXPECT_EQ(Error::kInvalidObject, error);
    }
  }
}

TEST(Function, FromMemberFunctionAndObjectPointerVirtual) {
  auto member_function = MF_MakeFunction(&Object::VirtualFunction);
  EXPECT_TRUE(IsMemberFunction<decltype(member_function)>::value);

  EXPECT_TRUE(member_function);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::VirtualFunction)>()),
            member_function.type_id());

  {
    // Test calling a virtual function using a base object.
    int id = rand();
    Object object(id);
    Function<int(bool&, bool&&, bool*)> function(member_function, &object);
    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&, bool*)>()), function.type_id());

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, function(called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_FALSE(derived);
  }

  {
    // Test calling a virtual function using a derived object.
    int id = rand();
    DerivedObject derived_object(id);
    Function<int(bool&, bool&&, bool*)> function(
        member_function, &derived_object);
    EXPECT_TRUE(function);
    EXPECT_EQ(&derived_object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&, bool*)>()), function.type_id());

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, function(called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_TRUE(derived);
  }
}

TEST(Function, FromMemberFunctionAndSharedPointer) {
  // Create a MemberFunction pointing to a member function.
  auto member_function = MF_MakeFunction(&Object::Function);
  EXPECT_TRUE(IsMemberFunction<decltype(member_function)>::value);
  EXPECT_TRUE(member_function);
  EXPECT_FALSE(decltype(member_function)::kIsConst);
  EXPECT_FALSE(decltype(member_function)::kIsVolatile);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::Function)>()),
            member_function.type_id());

  // Create a MemberFunction pointing to a const member function.
  auto const_member_function = MF_MakeFunction(&Object::ConstFunction);
  EXPECT_TRUE(IsMemberFunction<decltype(const_member_function)>::value);
  EXPECT_TRUE(const_member_function);
  EXPECT_TRUE(decltype(const_member_function)::kIsConst);
  EXPECT_FALSE(decltype(const_member_function)::kIsVolatile);
  EXPECT_EQ(nullptr, const_member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::ConstFunction)>()),
            const_member_function.type_id());

  // Create a MemberFunction pointing to a volatile member function.
  auto volatile_member_function = MF_MakeFunction(&Object::VolatileFunction);
  EXPECT_TRUE(IsMemberFunction<decltype(volatile_member_function)>::value);
  EXPECT_TRUE(volatile_member_function);
  EXPECT_FALSE(decltype(volatile_member_function)::kIsConst);
  EXPECT_TRUE(decltype(volatile_member_function)::kIsVolatile);
  EXPECT_EQ(nullptr, volatile_member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::VolatileFunction)>()),
            volatile_member_function.type_id());

  // Create a MemberFunction pointing to a const_volatile member function.
  auto const_volatile_member_function =
      MF_MakeFunction(&Object::ConstVolatileFunction);
  EXPECT_TRUE(IsMemberFunction<
      decltype(const_volatile_member_function)>::value);
  EXPECT_TRUE(const_volatile_member_function);
  EXPECT_TRUE(decltype(const_volatile_member_function)::kIsConst);
  EXPECT_TRUE(decltype(const_volatile_member_function)::kIsVolatile);
  EXPECT_EQ(nullptr, const_volatile_member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::ConstVolatileFunction)>()),
            const_volatile_member_function.type_id());

  {
    // Test creating a Function from a MemberFunction pointing to a member
    // function and binding an object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    Function<int(bool&, bool&&)> function(member_function, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member const
    // function and binding an object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    Function<int(bool&, bool&&)> function(const_member_function, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member
    // volatile function and binding an object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    Function<int(bool&, bool&&)> function(volatile_member_function, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member const
    // volatile function and binding an object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    Function<int(bool&, bool&&)> function(
        const_volatile_member_function, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member const
    // function and binding a const object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<const Object>(id);
    Function<int(bool&, bool&&)> function(const_member_function, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member const
    // volatile function and binding a const object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<const Object>(id);
    Function<int(bool&, bool&&)> function(
        const_volatile_member_function, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  #ifndef DISABLE_VOLATILE_SHARED_PTRS
  {
    // Test creating a Function from a MemberFunction pointing to a member
    // volatile function and binding a volatile object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<volatile Object>(id);
    Function<int(bool&, bool&&)> function(volatile_member_function, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a MemberFunction pointing to a member const
    // volatile function and binding a const volatile object shared pointer.
    int id = rand();
    auto object = std::make_shared<const volatile Object>(id);
    Function<int(bool&, bool&&)> function(
        const_volatile_member_function, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }
  #endif  // DISABLE_VOLATILE_SHARED_PTRS

  {
    // Test calling after copying and moving the function.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    auto member_function = MF_MakeFunction(&Object::Function);
    Function<int(bool&, bool&&)> function(member_function, object);

    auto function_copy = function;
    EXPECT_TRUE(function_copy);
    EXPECT_EQ(object.get(), function_copy.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function_copy.type_id());

    bool called = false;
    EXPECT_EQ(id, function_copy(called, true));
    EXPECT_TRUE(called);

    auto function_move = std::move(function_copy);
    EXPECT_TRUE(function_move);
    EXPECT_EQ(object.get(), function_move.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function_move.type_id());

    called = false;
    EXPECT_EQ(id, function_move(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a function with an invalid object.
    try {
      std::shared_ptr<Object> object;
      Function<int(bool&, bool&&)> function(member_function, object);
      FAIL();
    } catch (Error error) {
      EXPECT_EQ(Error::kInvalidObject, error);
    }
  }

  {
    // Check that the functions keep the object alive.
    auto object = std::make_shared<Object>(0);
    Function<int(bool&, bool&&)> function(member_function, object);

    std::weak_ptr<Object> weak_ptr = object;
    object.reset();
    EXPECT_FALSE(weak_ptr.expired());
    function = nullptr;
    EXPECT_TRUE(weak_ptr.expired());
  }
}

TEST(Function, FromMemberFunctionAndSharedPointerVirtual) {
  auto member_function = MF_MakeFunction(&Object::VirtualFunction);
  EXPECT_TRUE(IsMemberFunction<decltype(member_function)>::value);

  EXPECT_TRUE(member_function);
  EXPECT_EQ(nullptr, member_function.GetObject());
  EXPECT_EQ((get_type_id<decltype(&Object::VirtualFunction)>()),
            member_function.type_id());

  {
    // Test calling a virtual function using a base object.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    Function<int(bool&, bool&&, bool*)> function(member_function, object);
    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&, bool*)>()), function.type_id());

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, function(called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_FALSE(derived);
  }

  {
    // Test calling a virtual function using a derived object.
    int id = rand();
    auto derived_object = std::make_shared<DerivedObject>(id);
    Function<int(bool&, bool&&, bool*)> function(
        member_function, derived_object);
    EXPECT_TRUE(function);
    EXPECT_EQ(derived_object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&, bool*)>()), function.type_id());

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, function(called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_TRUE(derived);
  }
}

TEST(Function, FromFunctionAddressAndObjectPointer) {
  {
    // Test creating a Function from a member function address binding and
    // object pointer to it.
    int id = rand();
    Object object(id);
    auto function = MF_MakeFunction(&Object::Function, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member const function address and binding
    // an object pointer to it.
    int id = rand();
    Object object(id);
    auto function = MF_MakeFunction(&Object::ConstFunction, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member volatile function address and
    // binding an object pointer to it.
    int id = rand();
    Object object(id);
    auto function = MF_MakeFunction(&Object::VolatileFunction, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member const volatile function address
    // and binding an object pointer to it.
    int id = rand();
    Object object(id);
    auto function = MF_MakeFunction(&Object::ConstVolatileFunction, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member const function address and binding
    // a const object pointer to it.
    int id = rand();
    const Object object(id);
    auto function = MF_MakeFunction(&Object::ConstFunction, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member volatile function address and
    // binding a volatile object pointer to it.
    int id = rand();
    volatile Object object(id);
    auto function = MF_MakeFunction(&Object::VolatileFunction, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member const volatile function address
    // and binding a const volatile object pointer to it.
    int id = rand();
    const volatile Object object(id);
    auto function = MF_MakeFunction(&Object::ConstVolatileFunction, &object);

    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test calling after copying and moving the function.
    int id = rand();
    Object object(id);
    auto function = MF_MakeFunction(&Object::Function, &object);

    auto function_copy = function;
    EXPECT_TRUE(function_copy);
    EXPECT_EQ(&object, function_copy.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function_copy.type_id());

    bool called = false;
    EXPECT_EQ(id, function_copy(called, true));
    EXPECT_TRUE(called);

    auto function_move = std::move(function_copy);
    EXPECT_TRUE(function_move);
    EXPECT_EQ(&object, function_move.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function_move.type_id());

    called = false;
    EXPECT_EQ(id, function_move(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a function with an invalid object.
    try {
      Object* object = nullptr;
      auto function = MF_MakeFunction(&Object::Function, object);
      FAIL();
    } catch (Error error) {
      EXPECT_EQ(Error::kInvalidObject, error);
    }
  }
}

TEST(Function, FromFunctionAddressAndObjectPointerVirtual) {
  {
    // Test calling a virtual function using a base object.
    int id = rand();
    Object object(id);
    auto function = MF_MakeFunction(&Object::VirtualFunction, &object);
    EXPECT_TRUE(function);
    EXPECT_EQ(&object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&, bool*)>()), function.type_id());

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, function(called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_FALSE(derived);
  }

  {
    // Test calling a virtual function using a derived object.
    int id = rand();
    DerivedObject derived_object(id);
    auto function = MF_MakeFunction(&Object::VirtualFunction, &derived_object);
    EXPECT_TRUE(function);
    EXPECT_EQ(&derived_object, function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&, bool*)>()), function.type_id());

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, function(called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_TRUE(derived);
  }
}

TEST(Function, MemberFunctionAddressAndObjectPointerOverload) {
  // Test calling the non-qualified operator ().
  {
    int id = rand();
    Object object(id);
    auto function = Function<int(CVQualification&)>::FromMemberFunction<
        Object, &Object::Overloaded>(&object);

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kNonQualified, cv);

    auto function_const = Function<int(CVQualification&)>::FromMemberFunction<
        const Object, &Object::Overloaded>(&object);
    EXPECT_EQ(id, function_const(cv));
    EXPECT_EQ(CVQualification::kConstQualified, cv);

    auto function_volatile =
        Function<int(CVQualification&)>::FromMemberFunction<
            volatile Object, &Object::Overloaded>(&object);
    EXPECT_EQ(id, function_volatile(cv));
    EXPECT_EQ(CVQualification::kVolatileQualified, cv);

    auto function_const_volatile =
        Function<int(CVQualification&)>::FromMemberFunction<
            const volatile Object, &Object::Overloaded>(&object);
    EXPECT_EQ(id, function_const_volatile(cv));
    EXPECT_EQ(CVQualification::kConstVolatileQualified, cv);
  }

  // Test calling the const operator ().
  {
    int id = rand();
    const Object object(id);
    auto function = Function<int(CVQualification&)>::FromMemberFunction<
        const Object, &Object::Overloaded>(&object);

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kConstQualified, cv);

    auto function_const_volatile =
        Function<int(CVQualification&)>::FromMemberFunction<
            const volatile Object, &Object::Overloaded>(&object);
    EXPECT_EQ(id, function_const_volatile(cv));
    EXPECT_EQ(CVQualification::kConstVolatileQualified, cv);
  }

  // Test calling the volatile operator ().
  {
    int id = rand();
    volatile Object object(id);
    auto function = Function<int(CVQualification&)>::FromMemberFunction<
        decltype(object), &Object::Overloaded>(&object);

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kVolatileQualified, cv);

    auto function_const_volatile =
        Function<int(CVQualification&)>::FromMemberFunction<
            const volatile Object, &Object::Overloaded>(&object);
    EXPECT_EQ(id, function_const_volatile(cv));
    EXPECT_EQ(CVQualification::kConstVolatileQualified, cv);
  }

  // Test calling the const volatile operator ().
  {
    int id = rand();
    const volatile Object object(id);
    auto function = Function<int(CVQualification&)>::FromMemberFunction<
        decltype(object), &Object::Overloaded>(&object);

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kConstVolatileQualified, cv);
  }
}

TEST(Function, FromFunctionAddressAndSharedPointer) {
  {
    // Test creating a Function from a member function address and binding an
    // object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    auto function = MF_MakeFunction(&Object::Function, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member const function address and binding
    // an object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    auto function = MF_MakeFunction(&Object::ConstFunction, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member volatile function address and
    // binding an object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    auto function = MF_MakeFunction(&Object::VolatileFunction, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member const volatile function address
    // and binding an object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    auto function = MF_MakeFunction(&Object::ConstVolatileFunction, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member const function address and binding
    // a const object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<const Object>(id);
    auto function = MF_MakeFunction(&Object::ConstFunction, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  #ifndef DISABLE_VOLATILE_SHARED_PTRS
  {
    // Test creating a Function from a member volatile function address and
    // binding a volatile object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<volatile Object>(id);
    auto function = MF_MakeFunction(&Object::VolatileFunction, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a Function from a member const volatile function address
    // and binding a const volatile object shared pointer to it.
    int id = rand();
    auto object = std::make_shared<const volatile Object>(id);
    auto function = MF_MakeFunction(&Object::ConstVolatileFunction, object);

    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function.type_id());

    bool called = false;
    EXPECT_EQ(id, function(called, true));
    EXPECT_TRUE(called);
  }
  #endif  // DISABLE_VOLATILE_SHARED_PTRS

  {
    // Test calling after copying and moving the function.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    auto function = MF_MakeFunction(&Object::Function, object);

    auto function_copy = function;
    EXPECT_TRUE(function_copy);
    EXPECT_EQ(object.get(), function_copy.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function_copy.type_id());

    bool called = false;
    EXPECT_EQ(id, function_copy(called, true));
    EXPECT_TRUE(called);

    auto function_move = std::move(function_copy);
    EXPECT_TRUE(function_move);
    EXPECT_EQ(object.get(), function_move.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&)>()), function_move.type_id());

    called = false;
    EXPECT_EQ(id, function_move(called, true));
    EXPECT_TRUE(called);
  }

  {
    // Test creating a function with an invalid object.
    try {
      std::shared_ptr<Object> object;
      auto function = MF_MakeFunction(&Object::Function, object);
      FAIL();
    } catch (Error error) {
      EXPECT_EQ(Error::kInvalidObject, error);
    }
  }

  {
    // Check that the functions keep the object alive.
    auto object = std::make_shared<Object>(0);
    auto function = MF_MakeFunction(&Object::Function, object);

    std::weak_ptr<Object> weak_ptr = object;
    object.reset();
    EXPECT_FALSE(weak_ptr.expired());
    function = nullptr;
    EXPECT_TRUE(weak_ptr.expired());
  }
}

TEST(Function, FromFunctionAddressAndSharedPointerVirtual) {
  {
    // Test calling a virtual function using a base object.
    int id = rand();
    auto object = std::make_shared<Object>(id);
    auto function = MF_MakeFunction(&Object::VirtualFunction, object);
    EXPECT_TRUE(function);
    EXPECT_EQ(object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&, bool*)>()), function.type_id());

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, function(called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_FALSE(derived);
  }

  {
    // Test calling a virtual function using a derived object.
    int id = rand();
    auto derived_object = std::make_shared<DerivedObject>(id);
    auto function = MF_MakeFunction(&Object::VirtualFunction, derived_object);
    EXPECT_TRUE(function);
    EXPECT_EQ(derived_object.get(), function.GetObject());
    EXPECT_EQ((get_type_id<int(bool&, bool&&, bool*)>()), function.type_id());

    bool called = false;
    bool derived = false;
    EXPECT_EQ(id, function(called, true, &derived));
    EXPECT_TRUE(called);
    EXPECT_TRUE(derived);
  }
}

TEST(Function, MemberFunctionAddressAndSharedPointerOverload) {
  // Test calling the non-qualified operator ().
  {
    int id = rand();
    auto object = std::make_shared<Object>(id);
    auto function = Function<int(CVQualification&)>::FromMemberFunction<
        Object, &Object::Overloaded>(object);

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kNonQualified, cv);

    auto function_const = Function<int(CVQualification&)>::FromMemberFunction<
        const Object, &Object::Overloaded>(object);
    EXPECT_EQ(id, function_const(cv));
    EXPECT_EQ(CVQualification::kConstQualified, cv);

    auto function_volatile =
        Function<int(CVQualification&)>::FromMemberFunction<
            volatile Object, &Object::Overloaded>(object);
    EXPECT_EQ(id, function_volatile(cv));
    EXPECT_EQ(CVQualification::kVolatileQualified, cv);

    auto function_const_volatile =
        Function<int(CVQualification&)>::FromMemberFunction<
            const volatile Object, &Object::Overloaded>(object);
    EXPECT_EQ(id, function_const_volatile(cv));
    EXPECT_EQ(CVQualification::kConstVolatileQualified, cv);
  }

  // Test calling the const operator ().
  {
    int id = rand();
    auto object = std::make_shared<const Object>(id);
    auto function = Function<int(CVQualification&)>::FromMemberFunction<
        const Object, &Object::Overloaded>(object);

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kConstQualified, cv);

    auto function_const_volatile =
        Function<int(CVQualification&)>::FromMemberFunction<
            const volatile Object, &Object::Overloaded>(object);
    EXPECT_EQ(id, function_const_volatile(cv));
    EXPECT_EQ(CVQualification::kConstVolatileQualified, cv);
  }

  #ifndef DISABLE_VOLATILE_SHARED_PTRS
  // Test calling the volatile operator ().
  {
    int id = rand();
    auto object = std::make_shared<volatile Object>(id);
    auto function = Function<int(CVQualification&)>::FromMemberFunction<
        volatile Object, &Object::Overloaded>(object);

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kVolatileQualified, cv);

    auto function_const_volatile =
        Function<int(CVQualification&)>::FromMemberFunction<
            const volatile Object, &Object::Overloaded>(object);
    EXPECT_EQ(id, function_const_volatile(cv));
    EXPECT_EQ(CVQualification::kConstVolatileQualified, cv);
  }

  // Test calling the const volatile operator ().
  {
    int id = rand();
    auto object = std::make_shared<const volatile Object>(id);
    auto function = Function<int(CVQualification&)>::FromMemberFunction<
        const volatile Object, &Object::Overloaded>(object);

    CVQualification cv = CVQualification::kUndefined;
    EXPECT_EQ(id, function(cv));
    EXPECT_EQ(CVQualification::kConstVolatileQualified, cv);
  }
  #endif  // DISABLE_VOLATILE_SHARED_PTRS
}

TEST(Function, StdFunction) {
  // Test convertibility between mf::Function and std::function / std::bind
  // when pointing to a free function.
  {
    int a = rand();
    Function<int(int, int)> func1 = MF_MakeFunction(&Sum);
    Function<int(int, int)> func2 = std::function<int(int, int)>(func1);
    Function<int(int)> func3 = std::bind(func2, a, std::placeholders::_1);

    EXPECT_TRUE(func1);
    EXPECT_TRUE(func2);
    EXPECT_TRUE(func3);

    int b = rand();
    EXPECT_EQ(a + b, func1(a, b));
    EXPECT_EQ(a + b, func2(a, b));
    EXPECT_EQ(a + b, func3(b));
  }

  // Test convertibility between mf::Function and std::function / std::bind
  // when pointing to a lambda.
  {
    int a = rand();
    Function<int(int, int)> func1 = [](int x, int y) { return x + y; };
    Function<int(int, int)> func2 = std::function<int(int, int)>(func1);
    Function<int(int)> func3 = std::bind(func2, a, std::placeholders::_1);

    EXPECT_TRUE(func1);
    EXPECT_TRUE(func2);
    EXPECT_TRUE(func3);

    int b = rand();
    EXPECT_EQ(a + b, func1(a, b));
    EXPECT_EQ(a + b, func2(a, b));
    EXPECT_EQ(a + b, func3(b));
  }

  // Test convertibility between mf::Function and std::function / std::bind
  // when binding to a MemberFunction and an object pointer.
  {
    int id = rand();
    Object object(id);

    int a = rand();
    auto member_func = MF_MakeFunction(&Object::Sum);
    Function<int(int, int)> func1(member_func, &object);
    Function<int(int, int)> func2 = std::function<int(int, int)>(func1);
    Function<int(int)> func3 = std::bind(func2, a, std::placeholders::_1);

    EXPECT_TRUE(member_func);
    EXPECT_TRUE(func1);
    EXPECT_TRUE(func2);
    EXPECT_TRUE(func3);

    int b = rand();
    EXPECT_EQ(id + a + b, member_func(object, a, b));
    EXPECT_EQ(id + a + b, func1(a, b));
    EXPECT_EQ(id + a + b, func2(a, b));
    EXPECT_EQ(id + a + b, func3(b));
  }

  // Test convertibility between mf::Function and std::function / std::bind
  // when binding to a MemberFunction and a shared pointer.
  {
    int id = rand();
    auto object = std::make_shared<Object>(id);

    int a = rand();
    auto member_func = MF_MakeFunction(&Object::Sum);
    Function<int(int, int)> func1(member_func, object);
    Function<int(int, int)> func2 = std::function<int(int, int)>(func1);
    Function<int(int)> func3 = std::bind(func2, a, std::placeholders::_1);

    EXPECT_TRUE(member_func);
    EXPECT_TRUE(func1);
    EXPECT_TRUE(func2);
    EXPECT_TRUE(func3);

    int b = rand();
    EXPECT_EQ(id + a + b, member_func(*object, a, b));
    EXPECT_EQ(id + a + b, func1(a, b));
    EXPECT_EQ(id + a + b, func2(a, b));
    EXPECT_EQ(id + a + b, func3(b));
  }

  // Test convertibility between mf::Function and std::function / std::bind
  // when binding to a member function address and an object pointer.
  {
    int id = rand();
    Object obj(id);

    int a = rand();
    Function<int(int, int)> func1 = MF_MakeFunction(&Object::Sum, &obj);
    Function<int(int, int)> func2 = std::function<int(int, int)>(func1);
    Function<int(int)> func3 = std::bind(func2, a, std::placeholders::_1);

    EXPECT_TRUE(func1);
    EXPECT_TRUE(func2);
    EXPECT_TRUE(func3);

    int b = rand();
    EXPECT_EQ(id + a + b, func1(a, b));
    EXPECT_EQ(id + a + b, func2(a, b));
    EXPECT_EQ(id + a + b, func3(b));
  }

  // Test convertibility between mf::Function and std::function / std::bind
  // when binding to a member function address and a shared pointer.
  {
    int id = rand();
    auto obj = std::make_shared<Object>(id);

    int a = rand();
    Function<int(int, int)> func1 = MF_MakeFunction(&Object::Sum, obj);
    Function<int(int, int)> func2 = std::function<int(int, int)>(func1);
    Function<int(int)> func3 = std::bind(func2, a, std::placeholders::_1);

    EXPECT_TRUE(func1);
    EXPECT_TRUE(func2);
    EXPECT_TRUE(func3);

    int b = rand();
    EXPECT_EQ(id + a + b, func1(a, b));
    EXPECT_EQ(id + a + b, func2(a, b));
    EXPECT_EQ(id + a + b, func3(b));
  }
}

TEST(Function, CopyConstructFunction) {
  Function<int(int)> func1 = [](int x) { return x + 1; };
  Function<int(int)> func2(func1);
  EXPECT_TRUE(func1);
  EXPECT_TRUE(func2);
  EXPECT_EQ(func2(3), 4);

  Function<int(int)> func3;
  Function<int(int)> func4 = func3;
  EXPECT_FALSE(func3);
  EXPECT_FALSE(func4);
}

TEST(Function, MoveConstructFunction) {
  Function<int(int)> func1 = [](int x) { return x + 1; };
  Function<int(int)> func2(std::move(func1));
  EXPECT_FALSE(func1);
  EXPECT_TRUE(func2);
  EXPECT_EQ(func2(3), 4);

  Function<int(int)> func3;
  Function<int(int)> func4 = std::move(func3);
  EXPECT_FALSE(func3);
  EXPECT_FALSE(func4);
}

TEST(Function, CopyAssignFunction) {
  Function<int(int)> func1 = [](int x) { return x + 1; };
  Function<int(int)> func2;
  func2 = func1;
  EXPECT_TRUE(func1);
  EXPECT_TRUE(func2);
  EXPECT_EQ(func2(3), 4);

  Function<int(int)> func3;
  Function<int(int)> func4;
  func4 = func3;
  EXPECT_FALSE(func3);
  EXPECT_FALSE(func4);
}

TEST(Function, MoveAssignFunction) {
  Function<int(int)> func1 = [](int x) { return x + 1; };
  Function<int(int)> func2;
  func2 = std::move(func1);
  EXPECT_FALSE(func1);
  EXPECT_TRUE(func2);
  EXPECT_EQ(func2(3), 4);

  Function<int(int)> func3;
  Function<int(int)> func4;
  func4 = std::move(func3);
  EXPECT_FALSE(func3);
  EXPECT_FALSE(func4);
}
