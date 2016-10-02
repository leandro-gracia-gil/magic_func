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

#ifndef MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_EVENT_TUPLE_EXTRACTOR_H_
#define MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_EVENT_TUPLE_EXTRACTOR_H_

#include "cpp14_helpers.h"

// Utility functions to extract elements from a tuple for use in callbacks.
//
// The following function proceed as follows:
// 1. If the type is an lvalue reference then it's passed like that.
// 2. If the type is an rvalue reference then it's moved.
// 3. If the type is not a reference and it's copy-constructible, it's copied.
// 4. If the type is not a reference nor not copy-constructible, it's moved.
//
// This is designed to support both broadcasting and single observers taking
// moveable-only or object references. To force an object to be moved even if
// copy-constructible, use an rvalue reference type.

// Shortcut for getting the I-th type of a tuple of types.
template <size_t I, typename... Types>
using IthType = std::tuple_element_t<I, std::tuple<Types...>>;

// Return a lvalue reference for lvalue reference elements.
template <size_t I, typename... Types>
std::enable_if_t<std::is_lvalue_reference<IthType<I, Types...>>::value,
                 IthType<I, Types...>&>
ExpandEventArgs(std::tuple<Types...>& tuple) {
  return std::get<I>(tuple);
}

// Return a rvalue reference for rvalue reference elements.
template <size_t I, typename... Types>
std::enable_if_t<std::is_rvalue_reference<IthType<I, Types...>>::value,
                 IthType<I, Types...>&&>
ExpandEventArgs(std::tuple<Types...>& tuple) {
  return std::move(std::get<I>(tuple));
}

// Copy non-reference types if they are copy constructible.
template <size_t I, typename... Types>
std::enable_if_t<!std::is_reference<IthType<I, Types...>>::value &&
                 std::is_copy_constructible<IthType<I, Types...>>::value,
                 IthType<I, Types...>>
ExpandEventArgs(const std::tuple<Types...>& tuple) {
  return std::get<I>(tuple);
}

// Move non-reference types if they are not copy constructible.
template <size_t I, typename... Types>
std::enable_if_t<!std::is_reference<IthType<I, Types...>>::value &&
                 !std::is_copy_constructible<IthType<I, Types...>>::value,
                 IthType<I, Types...>&&>
ExpandEventArgs(std::tuple<Types...>& tuple) {
  static_assert(std::is_move_constructible<IthType<I, Types...>>::value,
                "Type is not copy or move constructible.");
  return std::move(std::get<I>(tuple));
}

#endif  // MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_EVENT_TUPLE_EXTRACTOR_H_
