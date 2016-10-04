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

// Visual Studio applies some optimizations in Release mode that fold functions
// performing the same operations into the same address in memory. This violates
// the assumption of function address uniqueness and screws up the behavior of
// GetTypeId(). This should not happen since these functions have their
// addresses taken, so this could be a linker bug.
//
// The following pragma disables this function folding feature. This is the
// same as setting Linker -> Optimization -> Disable COMDAT folding.
// See https://msdn.microsoft.com/en-us/library/bxwfs976(v=vs.140).aspx).
#ifdef _MSC_VER
#pragma comment(linker, "/OPT:NOICF")
#endif

namespace mf {

using TypeId = intptr_t;

template <typename... T>
constexpr TypeId GetTypeId() noexcept {
  return reinterpret_cast<TypeId>(reinterpret_cast<void*>(&GetTypeId<T...>));
}

}  // namespace mf

#endif  // MAGIC_FUNC_TYPE_ID_H_
