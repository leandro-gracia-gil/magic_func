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

#ifndef MAGIC_FUNC_MEMBER_FUNCTION_HPP_
#define MAGIC_FUNC_MEMBER_FUNCTION_HPP_

#include <magic_func/function.h>
#include <magic_func/type_id.h>

namespace mf {

// Default constructor.
template <typename MemberFuncPtr>
MemberFunction<MemberFuncPtr>::MemberFunction() noexcept
    : TypeErasedFunction(GetTypeId<MemberFuncPtr>()) {}

// Constructor used by factory methods taking member functions addresses.
template <typename MemberFuncPtr>
MemberFunction<MemberFuncPtr>::MemberFunction(
    TypeErasedFunction::TypeErasedFuncPtr member_func_ptr) noexcept
    : TypeErasedFunction(GetTypeId<MemberFuncPtr>(), member_func_ptr) {}

template <typename MemberFuncPtr>
template <MemberFuncPtr member_func_ptr>
std::enable_if_t<std::is_member_function_pointer<MemberFuncPtr>::value,
                 MemberFunction<MemberFuncPtr>>
MemberFunction<MemberFuncPtr>::FromMemberFunction() noexcept {
  // We use the function type here to avoid having to deduce the argument pack
  // types from MemberFuncPtr. If we do, we also have to specialize for all
  // possible combinations of const and volatile qualifications of the function.
  return MemberFunction(reinterpret_cast<TypeErasedFunction::TypeErasedFuncPtr>(
      &Function<FunctionType>::template CallMemberFuncAddress<
          MemberFuncPtr, member_func_ptr>));
}

template <typename MemberFuncPtr>
template <typename... CallArgs>
typename MemberFunction<MemberFuncPtr>::ReturnType
MemberFunction<MemberFuncPtr>::operator ()(
    ClassType& object, CallArgs&&... args) const {
  MAGIC_FUNC_DCHECK(func_ptr_, Error::kInvalidFunction);
  using FuncCallPtr = typename Traits::TypeErasedCallType;
  return (*reinterpret_cast<FuncCallPtr>(this->func_ptr_))(
      &object, std::forward<CallArgs>(args)...);
}

}  // namespace mf

#endif  // MAGIC_FUNC_MEMBER_FUNCTION_HPP_
