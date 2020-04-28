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

#ifndef MAGIC_FUNC_TYPE_ERASED_FUNCTION_HPP_
#define MAGIC_FUNC_TYPE_ERASED_FUNCTION_HPP_

namespace mf {

TypeErasedFunction::TypeErasedFunction() noexcept
    : func_ptr_(nullptr),
      type_id_(0) {}

TypeErasedFunction::TypeErasedFunction(TypeId type_id,
                                       TypeErasedFuncPtr func_ptr) noexcept
    : func_ptr_(func_ptr),
      type_id_(type_id) {}

TypeErasedFunction::TypeErasedFunction(TypeErasedFunction&& function)
    : func_ptr_(nullptr), type_id_(0) {
  std::swap(object_, function.object_);
  std::swap(func_ptr_, function.func_ptr_);
  std::swap(type_id_, function.type_id_);
}

TypeErasedFunction& TypeErasedFunction::operator =(
    const TypeErasedFunction& function) {
  if (this == &function)
    return *this;

  MAGIC_FUNC_CHECK(type_id_ == 0 || type_id_ == function.type_id_,
                   Error::kIncompatibleType);
  object_ = function.object_;
  func_ptr_ = function.func_ptr_;
  type_id_ = function.type_id_;
  return *this;
}

TypeErasedFunction& TypeErasedFunction::operator =(
    TypeErasedFunction&& function) {
  if (this == &function)
    return *this;

  MAGIC_FUNC_CHECK(type_id_ == 0 || type_id_ == function.type_id_,
                   Error::kIncompatibleType);
  object_.Reset();
  func_ptr_ = nullptr;
  type_id_ = 0;

  std::swap(object_, function.object_);
  std::swap(func_ptr_, function.func_ptr_);
  std::swap(type_id_, function.type_id_);
  return *this;
}

TypeErasedFunction& TypeErasedFunction::operator =(std::nullptr_t) {
  func_ptr_ = nullptr;
  object_.Reset();
  return *this;
}

}  // namespace mf

#endif  // MAGIC_FUNC_TYPE_ERASED_FUNCTION_HPP_
