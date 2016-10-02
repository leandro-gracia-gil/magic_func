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

#ifndef MAGIC_FUNC_ALLOCATOR_H_
#define MAGIC_FUNC_ALLOCATOR_H_

#include <utility>

namespace mf {

// Type for custom memory allocation functions.
// Allocates the requested number of bytes with the provided alignment.
//
// Arguments:
// - size: the number of bytes to allocate.
// - alignment: the required alignment for the allocation in bytes.
// - context: a value provided when setting up the function.
//
// Returns: a pointer to the allocated memory. Returning a null pointer will
// cause a fatal kCustomAllocator error.
using AllocationFunc = void* (*)(size_t size, size_t alignment, void* context);

// Type for custom memory deallocation functions.
// Deallocates memory previously allocated in a provided address.
//
// Arguments:
// - address: the address of the memory to deallocate.
// - size: the size in bytes requested when allocating the memory.
// - alignment: the alignment in bytes requested when allocating the memory.
// - context: a value provided when setting up the function.
//
// Returns: true on success, false on failure. Returning false will cause a
// fatal kCustomAllocator error.
using DeallocationFunc = bool (*)(void* address, size_t size,
                                  size_t alignment, void* context);

// Allows access to the custom allocator, if any.
inline std::pair<AllocationFunc, void*>& CustomAllocator() {
  static std::pair<AllocationFunc, void*> allocator(nullptr, nullptr);
  return allocator;
}

// Allows access to the custom deallocator, if any.
inline std::pair<DeallocationFunc, void*>& CustomDeallocator() {
  static std::pair<DeallocationFunc, void*> deallocator(nullptr, nullptr);
  return deallocator;
}

// Sets custom allocator functions to use.
//
// Should be set only once before starting to use MagicFunc and not changed, as
// it might lead to deallocations called on mismatching allocator functions.
// Functions always use the current custom allocation functions if any.
//
// @param allocation_func Function used to allocate memory.
// @param allocation_context Argument provided when invoking the allocator.
// @param deallocation_func Function used to deallocate memory.
// @param deallocation_context Argument provided when invoking the deallocator.
inline void SetCustomAllocator(
    AllocationFunc allocation_func, void* allocation_context,
    DeallocationFunc deallocation_func, void* deallocation_context) {
  CustomAllocator() = std::make_pair(allocation_func, allocation_context);
  CustomDeallocator() = std::make_pair(deallocation_func, deallocation_context);
}

}  // namespace mf

#endif  // MAGIC_FUNC_ALLOCATOR_H_
