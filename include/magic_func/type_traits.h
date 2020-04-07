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

#ifndef MAGIC_FUNC_TYPE_TRAITS_H_
#define MAGIC_FUNC_TYPE_TRAITS_H_

#include <memory>
#include <tuple>
#include <type_traits>

// Define shortchut template aliases provided by C++14 if not already defined.
#if __cplusplus < 201402L && (!defined(_MSC_VER) || _MSC_VER < 1900)
namespace std {

template <typename T>
using add_const_t = typename add_const<T>::type;

template <typename T>
using add_volatile_t = typename add_volatile<T>::type;

template <bool B, class T, class F>
using conditional_t = typename conditional<B, T, F>::type;

template <typename T>
using decay_t = typename decay<T>::type;

template <bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

template <typename T>
using remove_pointer_t = typename remove_pointer<T>::type;

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

}  // namespace std
#endif  // __cplusplus < 201402L && (!defined(_MSC_VER) || _MSC_VER < 1900)

namespace mf {

// Forward-declaration of functions.
template <typename FuncPtr>
class Function;

// Forward-declaration of member functions.
template <typename FuncPtr>
class MemberFunction;

namespace internal {

template <typename T>
struct IsFunctionImpl : public std::false_type {};

template <typename T>
struct IsFunctionImpl<Function<T>> : public std::true_type {};

template <typename T>
struct IsMemberFunctionImpl : public std::false_type {};

template <typename T>
struct IsMemberFunctionImpl<MemberFunction<T>> : public std::true_type {};

template <typename T>
struct IsUniquePtrImpl : public std::false_type {};

template <typename T, typename Deleter>
struct IsUniquePtrImpl<std::unique_ptr<T, Deleter>> : public std::true_type {};

template <typename T>
struct IsSharedPtrImpl : public std::false_type {};

template <typename T>
struct IsSharedPtrImpl<std::shared_ptr<T>> : public std::true_type {};

} // namespace internal

// Tells if a provided type is a mf::Function.
template <typename T>
using IsFunction = internal::IsFunctionImpl<
    std::remove_cv_t<std::decay_t<T>>>;

// Tells if a provided type is a mf::MemberFunction.
template <typename T>
using IsMemberFunction = internal::IsMemberFunctionImpl<
    std::remove_cv_t<std::decay_t<T>>>;

// Tells if a provided type is a std::unique_ptr.
template <typename T>
using IsUniquePtr = internal::IsUniquePtrImpl<
    std::remove_cv_t<std::decay_t<T>>>;

// Tells if a provided type is a std::shared_ptr.
template <typename T>
using IsSharedPtr = internal::IsSharedPtrImpl<
    std::remove_cv_t<std::decay_t<T>>>;

// Tells if a provided type is a free function pointer.
template <typename T>
using IsFunctionPointer =
    std::integral_constant<bool,
        // It's a function pointer if it's a pointer and...
        std::is_pointer<std::remove_reference_t<T>>::value &&
        // After removing cv cualifications and a pointer it becomes a function.
        std::is_function<std::remove_pointer_t<
            std::remove_cv_t<std::remove_reference_t<T>>>>::value>;

}  // namespace mf

#endif  // MAGIC_FUNC_TYPE_TRAITS_H_
