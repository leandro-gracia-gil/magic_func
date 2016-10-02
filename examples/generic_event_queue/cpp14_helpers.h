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

#ifndef MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_CPP14_HELPERS_H_
#define MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_CPP14_HELPERS_H_

// Implementation of C++14 STL helper functions for C++11.
#if __cplusplus < 201402L
namespace std {

template <size_t I, typename T>
using tuple_element_t = typename tuple_element<I, T>::type;

template <typename T>
using result_of_t = typename result_of<T>::type;

// Visual Studio 2015 already has a definition of std::integer_sequence and
// other related classes such as std::index_sequence.
#if defined(_MSC_VER) && _MSC_VER >= 1900
#include <utility>
#else
template <typename T, T... Ints>
class integer_sequence {
 public:
  static constexpr size_t size() { return sizeof...(Ints); }
  using value_type = T;
};

template <size_t... Ints>
using index_sequence = integer_sequence<size_t, Ints...>;

namespace impl {

template <typename T, typename Seq1, typename Seq2>
struct concat_integer_sequence_impl;

template <typename T, T... Ints1, T... Ints2>
struct concat_integer_sequence_impl<T, integer_sequence<T, Ints1...>,
                                       integer_sequence<T, Ints2...>> {
  using type = integer_sequence<T, Ints1..., Ints2...>;
};

template <typename T, typename Seq1, typename Seq2>
using concat_integer_sequence =
    typename concat_integer_sequence_impl<T, Seq1, Seq2>::type;

template <typename T, typename U>
struct make_integer_sequence_impl;

template <typename T, T N>
struct make_integer_sequence_impl<T, std::integral_constant<T, N>> {
  using type = concat_integer_sequence<
      T, typename make_integer_sequence_impl<
          T, std::integral_constant<T, N-1>>::type,
          integer_sequence<T, N-1>>;
};

template <typename T>
struct make_integer_sequence_impl<
    T, std::integral_constant<T, static_cast<T>(0)>> {
  using type = integer_sequence<T>;
};

}  // namespace impl

template <typename T, T N>
using make_integer_sequence = typename impl::make_integer_sequence_impl<
    T, std::integral_constant<T, N>>::type;

template <size_t N>
using make_index_sequence = make_integer_sequence<size_t, N>;

template <typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;
#endif  // !defined(_MSC_VER) || _MSC_VER < 1900

}  // namespace std
#endif  // __cplusplus < 201402L

#endif  // MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_CPP14_HELPERS_H_
