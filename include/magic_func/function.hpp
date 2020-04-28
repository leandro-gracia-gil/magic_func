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

#ifndef MAGIC_FUNC_FUNCTION_HPP_
#define MAGIC_FUNC_FUNCTION_HPP_

#include <magic_func/error.h>
#include <magic_func/type_id.h>

namespace mf {

// Default constructor.
template <typename Return, typename... Args>
Function<Return(Args...)>::Function() noexcept
    : TypeErasedFunction(get_type_id<FunctionType>()) {}

// Factory method for function addresses.
template <typename Return, typename... Args>
template <Return (*func_ptr)(Args...)>
Function<Return(Args...)> Function<Return(Args...)>::FromFunction() noexcept {
  return Function(reinterpret_cast<TypeErasedFuncPtr>(
      &CallFunctionAddress<func_ptr>));
}

// Factory function for member function addresses bound to an object pointer.
template <typename Return, typename... Args>
template <typename Object, CopyCV<Return(Args...), Object> Object::*func_ptr>
Function<Return(Args...)> Function<Return(Args...)>::FromMemberFunction(
    Object* object) {
  MAGIC_FUNC_DCHECK(object, Error::kInvalidObject);
  auto function = Function(reinterpret_cast<TypeErasedFuncPtr>(
      &CallMemberFuncAddress<decltype(func_ptr), func_ptr>));
  function.object_.StorePointer(object);
  return function;
}

// Factory function for member function addresses bound to an object shared_ptr.
template <typename Return, typename... Args>
template <typename Object, CopyCV<Return(Args...), Object> Object::*func_ptr>
Function<Return(Args...)> Function<Return(Args...)>::FromMemberFunction(
    const std::shared_ptr<Object>& object) {
  MAGIC_FUNC_DCHECK(object, Error::kInvalidObject);
  auto function = Function(reinterpret_cast<TypeErasedFuncPtr>(
      &CallMemberFuncAddress<decltype(func_ptr), func_ptr>));
  function.object_.StoreObject(object);
  return function;
}

// Constructor for MemberFunction objects bound to an object pointer.
template <typename Return, typename... Args>
template <typename MemberFuncPtr, typename Object, typename>
Function<Return(Args...)>::Function(
    const MemberFunction<MemberFuncPtr>& member_function, Object* object)
    : TypeErasedFunction(get_type_id<FunctionType>()) {
  // Class is qualified as the member function and Object as the object.
  // This enforces const compatibility and produces more useful build errors.
  typename FunctionTraits<MemberFuncPtr>::Class* class_ptr = object;
  MAGIC_FUNC_DCHECK(class_ptr, Error::kInvalidObject);
  func_ptr_ = member_function.func_ptr_;
  object_.StorePointer(class_ptr);
}

// Constructor for MemberFunction objects bound to an object shared pointer.
template <typename Return, typename... Args>
template <typename MemberFuncPtr, typename Object, typename>
Function<Return(Args...)>::Function(
    const MemberFunction<MemberFuncPtr>& member_function,
    const std::shared_ptr<Object>& object)
    : TypeErasedFunction(get_type_id<FunctionType>()) {
  // Class is qualified as the member function and Object as the object.
  // This enforces const compatibility and produces more useful build errors.
  using Class = typename FunctionTraits<MemberFuncPtr>::Class;
  const std::shared_ptr<Class>& class_object = object;
  MAGIC_FUNC_DCHECK(class_object, Error::kInvalidObject);
  func_ptr_ = member_function.func_ptr_;
  object_.StoreObject(class_object);
}

// Constructor for compatible callable objects.
template <typename Return, typename... Args>
template <typename Callable, typename>
Function<Return(Args...)>::Function(Callable&& callable)
    : TypeErasedFunction(
        get_type_id<FunctionType>(),
        reinterpret_cast<TypeErasedFuncPtr>(&CallCallable<Callable>)) {
  // Store the callable object within the function or owned by it in the heap.
  object_.StoreObject(std::forward<Callable>(callable));
}

// Auxiliary constructor for factory methods based on function addresses and
// member function addresses bound to objects.
template <typename Return, typename... Args>
Function<Return(Args...)>::Function(TypeErasedFuncPtr func_ptr) noexcept
    : TypeErasedFunction(get_type_id<FunctionType>(), func_ptr) {}

// Assignment operator for compatible callable objects.
template <typename Return, typename... Args>
template <typename Callable, typename>
Function<Return(Args...)>& Function<Return(Args...)>::operator =(
    Callable&& callable) {
  using T = std::remove_reference_t<Callable>;
  func_ptr_ = reinterpret_cast<TypeErasedFuncPtr>(&CallCallable<Callable>);
  object_.StoreObject(std::forward<Callable>(callable));
  return *this;
}

// Assignment operator to nullptr.
template <typename Return, typename... Args>
Function<Return(Args...)>& Function<Return(Args...)>::operator =(
    std::nullptr_t null) {
  TypeErasedFunction::operator =(null);
  return *this;
}

// Parenthesis operator for calling functions.
template <typename Return, typename... Args>
Return Function<Return(Args...)>::operator ()(Args... args) const {
  MAGIC_FUNC_DCHECK(func_ptr_, Error::kInvalidFunction);

  // Invoke whatever helper function is set.
  // Each one will take care of undoing type erasure and calling.
  return (*reinterpret_cast<Return (*)(void*, Args...)>
      (func_ptr_))(object_.GetObject(), std::forward<Args>(args)...);
}

// Auxiliary function to forward calls to function addresses provided as
// template arguments.
template <typename Return, typename... Args>
template <Return (*func_ptr)(Args...)>
Return Function<Return(Args...)>::CallFunctionAddress(void*, Args... args) {
  return func_ptr(std::forward<Args>(args)...);
}

// Auxiliary function to recover from type erasure and call a member function
// address with the provided object.
template <typename Return, typename... Args>
template <typename MemberFuncPtr, MemberFuncPtr func_ptr, typename>
Return Function<Return(Args...)>::CallMemberFuncAddress(
    void* object, Args... args) {
  MAGIC_FUNC_DCHECK(object, Error::kInvalidObject);
  using Class = typename FunctionTraits<MemberFuncPtr>::Class;
  return (reinterpret_cast<Class*>(object)->*func_ptr)(
      std::forward<Args>(args)...);
}

template <typename Return, typename... Args>
template <typename Callable>
Return Function<Return(Args...)>::CallCallable(void* object, Args... args) {
  MAGIC_FUNC_DCHECK(object, Error::kInvalidObject);
  using Object = std::remove_reference_t<Callable>;
  return reinterpret_cast<Object*>(object)->operator()(
      std::forward<Args>(args)...);
}

}  // namespace mf

#endif  // MAGIC_FUNC_FUNCTION_HPP_
