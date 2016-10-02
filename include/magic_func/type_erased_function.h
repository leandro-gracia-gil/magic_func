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

#ifndef MAGIC_FUNC_TYPE_ERASED_FUNCTION_H_
#define MAGIC_FUNC_TYPE_ERASED_FUNCTION_H_

#include <magic_func/type_erased_object.h>
#include <magic_func/type_id.h>

namespace mf {

// Type-erased base function type.
class TypeErasedFunction {
 public:
  // Creates an empty, uninitialized type-erased function.
  // The object has no type yet and can be assigned to any other.
  // Trying to FunctionCast it to another object will fail.
  inline TypeErasedFunction() noexcept;

  // Default copy and move constructors.
  inline TypeErasedFunction(const TypeErasedFunction&) = default;
  inline TypeErasedFunction(TypeErasedFunction&&) = default;

  // Copies another function into the current object.
  //
  // If this object has a type set, the other function must have the same type
  // or this call will raise an assertion failure.
  inline TypeErasedFunction& operator =(const TypeErasedFunction& function);

  // Moves another function into the current object.
  //
  // If this object has a type set, the other function must have the same type
  // or this call will raise an assertion failure.
  inline TypeErasedFunction& operator =(TypeErasedFunction&& function);

  // Tells if the object point to a valid function or not.
  explicit operator bool() const noexcept { return func_ptr_ != nullptr; }

  bool operator ==(std::nullptr_t) const noexcept {
	  return func_ptr_ == nullptr;
  }

  bool operator !=(std::nullptr_t) const noexcept {
	  return func_ptr_ != nullptr;
  }

  // Provides a unique integer representation of the type this object is
  // encapsulating. Type ids can be uninitialized (with a value of 0), but once
  // initialized they cannot change. This prevents from copying or moving two
  // incompatible objects at the type-erased function level.
  TypeId type_id() const noexcept { return type_id_; }

  // Returns a pointer to the object associated to this function if any.
  void* GetObject() const noexcept { return object_.GetObject(); }

  // Resets the function.
  inline TypeErasedFunction& operator =(std::nullptr_t);

 protected:
 public:
  // Type-erased versions of functions.
  using TypeErasedFuncPtr = void (*)();

  // Constructor used by derived types.
  inline TypeErasedFunction(TypeId type_id,
                            TypeErasedFuncPtr func_ptr = nullptr) noexcept;

  // Type-erased version of the object associated with the function, if any.
  // Goes intentionally first because it can have alignment requirements.
  TypeErasedObject object_;

  // Type-erased function pointer.
  // Points to an auxiliary function that restores the actual type and performs
  // the appropriate call.
  TypeErasedFuncPtr func_ptr_;

  // Runtime representation of the type managed by this object.
  TypeId type_id_;
};

}  // namespace mf

#include <magic_func/type_erased_function.hpp>

#endif  // MAGIC_FUNC_TYPE_ERASED_FUNCTION_H_
