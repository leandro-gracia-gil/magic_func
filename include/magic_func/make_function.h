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

#ifndef MAGIC_FUNC_MAKE_FUNCTION_H_
#define MAGIC_FUNC_MAKE_FUNCTION_H_

#include <magic_func/function.h>
#include <magic_func/function_traits.h>
#include <magic_func/member_function.h>
#include <magic_func/type_traits.h>

// Helper macro to create functions that take function addresses as template
// arguments. Avoids having to write something like
// make_function<decltype(&Foo::Bar), &Foo::Bar>(&object).
// Instead, MF_MakeFunction(&Foo::Bar, &object) can be used.
#ifndef MF_MakeFunction
#define MF_MakeFunction(x, ...) \
    mf::make_function<decltype((x)), (x)>(__VA_ARGS__)
#define MF_MakeFunctionNoArgs(x) \
    mf::make_function<decltype((x)), (x)>()
#endif

namespace mf {

// Creates a Function object deducing its type when provided a function pointer
// as a template argument. Works with MF_MakeFunction.
//
// Example:
// void Foo(int x);
// auto function = MF_MakeFunction(&Foo); // Function<void(int)>.
//
// Warning: MF_MakeFunction does not work if the function is overloaded
// because it's unable to guess which version should be used. In these cases
// Function<T>::FromFunction must be used directly.
template <typename FuncPtr, FuncPtr func_ptr>
std::enable_if_t<IsFunctionPointer<FuncPtr>::value,
                 Function<typename FunctionTraits<FuncPtr>::FunctionType>>
make_function() {
  using Result = Function<typename FunctionTraits<FuncPtr>::FunctionType>;
  return Result::template FromFunction<func_ptr>();
}

// Creates a MemberFunction object deducing its type when provided a member
// function address. Works with MF_MakeFunction.
//
// Example:
// struct Object {
//   void Foo(int x);
// };
//
// // Deduced as MemberFunction<void (Object::*)(int)>.
// auto member_function = MF_MakeFunction(&Object::Foo);
//
// WARNING: MF_MakeFunction does not work if the function is overloaded
// because it's unable to guess which version should be used. In these cases
// MemberFunction<T>::FromMemberFunction must be used directly.
template <typename FuncPtr, FuncPtr func_ptr>
std::enable_if_t<std::is_member_function_pointer<FuncPtr>::value,
                 MemberFunction<FuncPtr>>
make_function() {
  return MemberFunction<FuncPtr>::template FromMemberFunction<func_ptr>();
}

// Creates an Function object deducing its type when provided a member function
// address and an object to be bound to it. Works with MF_MakeFunction.
//
// Example:
// struct Object {
//   void Foo(int x);
// };
//
// // Deduced as Function<void(int)>.
// Object object;
// auto function_ref_object = MF_MakeFunction(&Object::Foo, &object);
//
// WARNING: MF_MakeFunction does not work if the function is overloaded
// because it's unable to guess which version should be used. In these cases
// Function<T>::FromMemberFunction must be used directly.
template <typename FuncPtr, FuncPtr func_ptr>
typename std::enable_if<
    std::is_member_function_pointer<FuncPtr>::value,
    Function<typename FunctionTraits<FuncPtr>::FunctionType>>::type
make_function(typename FunctionTraits<FuncPtr>::Class* object) {
  using Result = Function<typename FunctionTraits<FuncPtr>::FunctionType>;
  using Class = typename FunctionTraits<FuncPtr>::Class;
  return Result::template FromMemberFunction<Class, func_ptr>(object);
}

// Creates an Function object deducing its type when provided a member function
// address and an object shared pointer to be bound to it.
// Works with MF_MakeFunction.
//
// Example:
// struct Object {
//   void Foo(int x);
// };
//
// // Deduced as Function<void(int)>.
// auto shared_object = std::make_shared<Object>();
// auto function_shared_object = MF_MakeFunction(
//     &Object::Foo, shared_object);
//
// WARNING: MF_MakeFunction does not work if the function is overloaded
// because it's unable to guess which version should be used. In these cases
// Function<T>::FromMemberFunction must be used directly.
template <typename FuncPtr, FuncPtr func_ptr>
typename std::enable_if<
    std::is_member_function_pointer<FuncPtr>::value,
    Function<typename FunctionTraits<FuncPtr>::FunctionType>>::type
make_function(
    const std::shared_ptr<typename FunctionTraits<FuncPtr>::Class>& object) {
  using Result = Function<typename FunctionTraits<FuncPtr>::FunctionType>;
  using Class = typename FunctionTraits<FuncPtr>::Class;
  return Result::template FromMemberFunction<Class, func_ptr>(object);
}

// Creates an Function object deducing its type when provided a MemberFunction
// and an object to be bound to it.
//
// Example:
// struct Object {
//   void Foo(int x);
// };
//
// // Deduced as MemberFunction<void (Object::*)(int)>.
// auto member_function = MF_MakeFunction(&Object::Foo);
//
// Object object;
// auto shared_object = std::make_shared<Object>();
// auto function_ref_object = make_function(member_function, &object);
// auto function_shared_object = make_function(member_function, shared_object);
template <typename MemberFunctionType, typename ObjectType>
std::enable_if_t<
    IsMemberFunction<MemberFunctionType>::value,
    Function<typename FunctionTraits<
        typename MemberFunctionType::FunctionPointerType>::FunctionType>>
make_function(const MemberFunctionType& obj_function, const ObjectType& object) {
  using Result = decltype(make_function(obj_function, object));
  return Result(obj_function, object);
}

// Creates a Function object deducing its type when provided a callable or a
// lambda object.
//
// Since this method takes a universal reference, the callable object can be a
// lvalue reference or a rvalue reference. Similarly the Callable type can be a
// reference type.
//
// // Example 1:
// auto function = make_function([](int x) { std::cout << x << std::endl; });
//
// // Example 2:
// auto lambda = [](int x) { std::cout << x << std::endl; };
// auto function = make_function(lambda);
//
// WARNING: this method only works if Callable::operator () is not overloaded.
// If it is, this function simply cannot guess which version should be used and
// therefore what the type of the function should be. In these situations it is
// required to use the Function constructor explicitly. For example:
//
// struct Callable {
//   void operator() {}
//   void operator(int x) {}
// };
//
// Callable callable;
//
// // This will not work because it cannot guess which which operator to use:
// auto function = make_function(callable);
//
// // However, this will work because the function type can disambiguate:
// Function<void()> function_void = callable;
// Function<void(int)> function_int = callable;
template <typename Callable>
Function<typename FunctionTraits<CallableType<Callable>>::FunctionType>
make_function(Callable&& callable) {
  using Result = decltype(make_function(std::forward<Callable>(callable)));
  return Result(std::forward<Callable>(callable));
}

}  // namespace mf

#endif  // MAGIC_FUNC_MAKE_FUNCTION_H_
