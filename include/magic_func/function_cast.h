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

#ifndef MAGIC_FUNC_FUNCTION_CAST_H_
#define MAGIC_FUNC_FUNCTION_CAST_H_

#include <type_traits>

#include <magic_func/function.h>
#include <magic_func/function_traits.h>
#include <magic_func/member_function.h>
#include <magic_func/type_erased_function.h>
#include <magic_func/type_id.h>
#include <magic_func/type_traits.h>

namespace mf {

// Casts from a type-erased function to a function when a function type is
// provided.
template <typename T>
std::enable_if_t<std::is_function<T>::value, Function<T>&>
function_cast(TypeErasedFunction& function) {
  MAGIC_FUNC_CHECK(function.type_id() == GetTypeId<T>(), Error::kInvalidCast);
  return static_cast<Function<T>&>(function);
}

// Const version of the above.
template <typename T>
std::enable_if_t<std::is_function<T>::value, const Function<T>&>
function_cast(const TypeErasedFunction& function) {
  MAGIC_FUNC_CHECK(function.type_id() == GetTypeId<T>(), Error::kInvalidCast);
  return static_cast<const Function<T>&>(function);
}

// Casts from a type-erased function to a function when a function pointer type
// is provided. Allows using the function_cast<decltype(func)> syntax.
template <typename T>
std::enable_if_t<
    IsFunctionPointer<T>::value,
    Function<typename FunctionTraits<T>::FunctionType>&>
function_cast(TypeErasedFunction& function) {
  using FunctionType = typename FunctionTraits<T>::FunctionType;
  MAGIC_FUNC_CHECK(function.type_id() == GetTypeId<FunctionType>(),
                   Error::kInvalidCast);
  return static_cast<Function<FunctionType>&>(function);
}

// Const version of the above.
template <typename T>
std::enable_if_t<
    IsFunctionPointer<T>::value,
    const Function<typename FunctionTraits<T>::FunctionType>&>
function_cast(const TypeErasedFunction& function) {
  using FunctionType = typename FunctionTraits<T>::FunctionType;
  MAGIC_FUNC_CHECK(function.type_id() == GetTypeId<FunctionType>(),
                   Error::kInvalidCast);
  return static_cast<const Function<FunctionType>&>(function);
}

// Casts from a type-erased function to a member function when a member function
// pointer type is provided.
template <typename T>
std::enable_if_t<std::is_member_function_pointer<T>::value, MemberFunction<T>&>
function_cast(TypeErasedFunction& function) {
  MAGIC_FUNC_CHECK(function.type_id() == GetTypeId<T>(), Error::kInvalidCast);
  return static_cast<MemberFunction<T>&>(function);
}

// Const version of the above.
template <typename T>
std::enable_if_t<std::is_member_function_pointer<T>::value,
                 const MemberFunction<T>&>
function_cast(const TypeErasedFunction& function) {
  MAGIC_FUNC_CHECK(function.type_id() == GetTypeId<T>(), Error::kInvalidCast);
  return static_cast<const MemberFunction<T>&>(function);
}

}  // namespace mf

#endif  // MAGIC_FUNC_FUNCTION_CAST_H_
