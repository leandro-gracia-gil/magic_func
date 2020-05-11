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

#ifndef MAGIC_FUNC_MEMBER_FUNCTION_H_
#define MAGIC_FUNC_MEMBER_FUNCTION_H_

#include <type_traits>

#include <magic_func/port.h>
#include <magic_func/type_erased_function.h>

namespace mf {

// Class encapsulating a member function of an object.
// MemberFuncPtr must be a member function pointer type.
template <typename MemberFuncPtr>
class MemberFunction : public TypeErasedFunction {
 public:
  static_assert(std::is_member_function_pointer<MemberFuncPtr>::value,
                "Template argument must be a member function pointer.");

  // Auxiliary type for traits of the function pointer.
  using Traits = FunctionTraits<MemberFuncPtr>;

  // The function type of this member function pointer type.
  // For example, for void (Class::*)(int) is void(int).
  using FunctionType = typename Traits::FunctionType;

  // The member function pointer type of this MemberFunction.
  using FunctionPointerType = typename Traits::FunctionPointerType;

  // The return type of the member function.
  using ReturnType = typename Traits::Return;

  // A tuple with the argument types of the member function.
  using ArgTypes = typename Traits::Args;

  // The class the member function belongs to.
  using ClassType = typename Traits::Class;

  // Flag indicating the qualifications of the member function.
  enum : bool {
    kIsConst = std::is_const<ClassType>::value,
    kIsVolatile = std::is_volatile<ClassType>::value,
  };

  // Creates an empty typed MemberFunction.
  MemberFunction() MF_NOEXCEPT;

  // Factory method for member function addresses.
  template <MemberFuncPtr member_func_ptr,
            typename = std::enable_if_t<
                std::is_member_function_pointer<MemberFuncPtr>::value>>
  static MemberFunction<MemberFuncPtr> FromMemberFunction() MF_NOEXCEPT;

  // Invokes the function returning its result.
  //
  // Provided arguments must be convertible to the arguments of the
  // MemberFunction. Similarly, the object must have compatible qualifications
  // (const and volatile) to the targeted member function.
  //
  // This method uses universal references, so provided arguments do not need
  // to be rvalue references. These can be lvalue or rvalue references depending
  // on what is provided for each argument.
  //
  // Note that ClassType has the same qualifications as the member function.
  // This makes the object become a const reference if the function is const.
  template <typename... CallArgs>
  ReturnType operator ()(ClassType& object, CallArgs&&... args) const;

 private:
  // For accessing func_ptr_.
  template <typename FuncType>
  friend class Function;

  explicit MemberFunction(
      TypeErasedFunction::TypeErasedFuncPtr member_func_ptr) MF_NOEXCEPT;
};

}  // namespace mf

#include <magic_func/member_function.hpp>

#endif  // MAGIC_FUNC_MEMBER_FUNCTION_H_
