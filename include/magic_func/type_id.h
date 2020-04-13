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

#ifndef MAGIC_FUNC_TYPE_ID_H_
#define MAGIC_FUNC_TYPE_ID_H_

#include <cstdint>

namespace mf {

using TypeId = intptr_t;

// Visual Studio applies the linker flags /OPT:ICF by default in Release builds,
// which merges identical functions into the same address. This makes the usual
// get_type_id approach to fail because all type ids collapse to the same value.
//
// Instead, for Visual Studio Release builds, or if the macro MSC_OPT_NOICF is
// manually defined by the user, we use an alternative approach where the id
// comes from the address of a static variable in a template function.
//
// Note that neither of these approaches produce ids that should:
// 1. Be serialized.
// 2. Be shared across processes.
// 3. Be used across Windows DLL boundaries.
//
#if defined(_MSC_VER) && !defined(_DEBUG) && !defined(MSC_OPT_NOICF)
template <typename... T>
TypeId get_type_id() noexcept {
  static uint8_t id;
  return reinterpret_cast<TypeId>(&id);
}
#else
template <typename... T>
constexpr TypeId get_type_id() noexcept {
  // The double reinterpret_cast is to workaround a MSVC compiler error.
  return reinterpret_cast<TypeId>(reinterpret_cast<void*>(&get_type_id<T...>));
}
#endif

}  // namespace mf

#endif  // MAGIC_FUNC_TYPE_ID_H_
