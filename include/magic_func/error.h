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

#ifndef MAGIC_FUNC_ERROR_H_
#define MAGIC_FUNC_ERROR_H_

namespace mf {

// Enumeration of magic func error codes.
enum class Error {
  // The function to call is invalid or not set.
  kInvalidFunction,

  // The object to call is invalid or not set.
  kInvalidObject,

  // Invalid type cast. Actual types do not match.
  kInvalidCast,

  // Actual types behind type erasure are not compatible.
  kIncompatibleType,

  // Object not copyable. Type-erased object is not copy-constructible.
  kNonCopyableObject,

  // Custom allocator failed to allocate or deallocate the memory.
  kCustomAllocator,
};

// Macro called in case of error.
//
// By default, throws an error exception if exceptions are enabled, or just
// terminates otherwise.
//
// Can be customized as needed, but continuing to run after an error is
// undefined behavior and can cause crashes or subtle issues.
#if !defined(MAGIC_FUNC_ERROR)
#if defined(__EXCEPTIONS) || defined(_HAS_EXCEPTIONS) || defined(_CPPUNWIND)
#define MAGIC_FUNC_ERROR(error) throw (error);
#else
#define MAGIC_FUNC_ERROR(error) std::terminate();
#endif
#endif

// Auxiliary macro for unconditional assertions.
//
// Used to assert conditions that are severe enough to be verified in all kinds
// of builds, not only debug ones.
//
// Unconditional assertions include runtime type checks in function casts, where
// incorrect casts can have many dangerous and hard to find consequences.
#define MAGIC_FUNC_CHECK(cond, error) \
    if (!(cond)) { MAGIC_FUNC_ERROR((error)); }

// Auxiliary macro for debug-mode assertions.
//
// Used to assert conditions in debug builds that are not critical and might
// otherwise decrease performance.
//
// Debug-mode assertions involve checking for the validity of functions and
// objects, where if not valid a crash involving nullptr is likely to happen.
//
// Debug-mode assertions can be disabled by defining the NDEBUG macro.
#if !defined(NDEBUG)
#define MAGIC_FUNC_DCHECK(cond, error) \
    if (!(cond)) { MAGIC_FUNC_ERROR((error)); }
#else
#define MAGIC_FUNC_DCHECK(cond, error)
#endif

}  // namespace mf

#endif  // MAGIC_FUNC_ERROR_H_
