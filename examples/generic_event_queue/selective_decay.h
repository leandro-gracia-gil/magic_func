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

#ifndef MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_SELECTIVE_DECAY_H_
#define MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_SELECTIVE_DECAY_H_

namespace impl {

// Tells if the provided type is a std::reference_wrapper.
template <typename T>
struct IsReferenceWrapper {
  enum : bool { value = false };
};

template <typename T>
struct IsReferenceWrapper<std::reference_wrapper<T>> {
  enum : bool { value = true };
};

// Decays a tuple of arguments T1 into the types defined by the tuple T2
// for all types except reference wrappers.
//
// This is used with T1 as the tuple of arguments provided for enqueuing a
// function call and T2 being the tuple of arguments expected by the function.
template <typename T1, typename T2>
struct SelectiveDecay;

template <typename Arg1, typename... Args1, typename Arg2, typename... Args2>
struct SelectiveDecay<std::tuple<Arg1, Args1...>, std::tuple<Arg2, Args2...>> {
  // If the type is a reference wrapper keep it as it is. Otherwise decay it.
  using Element = std::conditional_t<
      IsReferenceWrapper<Arg1>::value,
      Arg2, std::decay_t<Arg2>>;

  // Recursively concatenate the result into a tuple.
  using type = decltype(std::tuple_cat(
      std::declval<std::tuple<Element>>(),
      std::declval<typename SelectiveDecay<
          std::tuple<Args1...>, std::tuple<Args2...>>::type>()));
};

template <>
struct SelectiveDecay<std::tuple<>, std::tuple<>> {
  using type = std::tuple<>;
};

}  // namespace impl

// Decays a tuple of arguments T1 into the types defined by the tuple T2
// for all types except reference wrappers.
//
// The result is a selective decay of T2 depending on the types in T1.
// This ensures explicit references using std::ref can be kept as such.
template <typename T1, typename T2>
using SelectiveDecay = typename impl::SelectiveDecay<T1, T2>::type;

#endif  // MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_SELECTIVE_DECAY_H_
