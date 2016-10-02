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

#include <array>
#include <bitset>
#include <memory>

#include <magic_func/allocator.h>
#include <magic_func/function.h>
#include <magic_func/make_function.h>
#include <gtest/gtest.h>

using namespace mf;

// Allocates memory within a local buffer based on blocks of a provided size.
//
// @tparam BlockSize Size of each allocation block.
// @tparam NumBlocks Number of blocks in the allocator buffer.
template <size_t BlockSize, size_t NumBlocks>
class SampleCustomAllocator {
 public:
  static constexpr size_t kBufferSize = BlockSize * NumBlocks;

  SampleCustomAllocator() : buffer_(new uint8_t[kBufferSize]) {}

  void* Allocate(size_t size, size_t alignment) {
    if (size == 0)
      return nullptr;

    // Count how many blocks this request needs.
    size_t required_blocks = (size + BlockSize - 1) / BlockSize;

    for (size_t block = 0; block < NumBlocks; ++block) {
      // Find the first available set of contiguous blocks long enough for
      // our needs. Might not be enough after taking alignment into account.
      bool has_enough_contiguous_blocks = true;
      for (size_t i = 0; i < required_blocks; ++i) {
        if (blocks_used_.test(block + i)) {
          has_enough_contiguous_blocks = false;
          break;
        }
      }

      if (!has_enough_contiguous_blocks)
        continue;

      // Take alignment into account, which might require additional blocks.
      uint8_t* start_ptr = buffer_.get() + block * BlockSize;
      size_t alignment_shift = (alignment -
          (reinterpret_cast<uintptr_t>(start_ptr) % alignment)) % alignment;
      start_ptr += alignment_shift;

      // Check if the required size puts us past the buffer size.
      uint8_t* end_ptr = start_ptr + size;
      size_t end_in_buffer = end_ptr - buffer_.get();
      if (end_in_buffer >= kBufferSize)
        return nullptr;

      // Check if any extra alignment requires more blocks, or leaves some empty
      // at the beginning.
      size_t start_block = (start_ptr - buffer_.get()) / BlockSize;
      size_t end_block = (end_in_buffer + BlockSize - 1) / BlockSize;
      for (size_t i = block + required_blocks; i < end_block; ++i) {
        if (blocks_used_.test(i)) {
          has_enough_contiguous_blocks = false;
          break;
        }
      }

      // More blocks required by alignment, but none available.
      if (!has_enough_contiguous_blocks)
        continue;

      // Everything looks good, so mark blocks as used and return the pointer.
      for (size_t i = start_block; i < end_block; ++i)
        blocks_used_.set(i);

      return start_ptr;
    }

    // No continuous set of blocks large enough available.
    return nullptr;
  }

  bool Deallocate(void* address, size_t size, size_t alignment) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(address);
    if (ptr == nullptr)
      return true;

    // Verify the block is within the buffer bounds.
    if (ptr < buffer_.get() || ptr + size >= buffer_.get() + kBufferSize)
      return false;

    // Verify the address and alignment actually point to a block.
    size_t pos_in_buffer = ptr - buffer_.get();
    size_t block_start_pos = pos_in_buffer - (pos_in_buffer % alignment);
    if (block_start_pos % BlockSize != 0)
      return false;

    // Find the range of blocks to deallocate.
    size_t first_block = block_start_pos / BlockSize;
    size_t last_block = (pos_in_buffer + size + BlockSize - 1) / BlockSize;

    // Verify the blocks are set as in use.
    for (size_t i = first_block; i < last_block; ++i) {
      if (!blocks_used_.test(i))
        return false;
    }

    // Set the blocks as free.
    for (size_t i = first_block; i < last_block; ++i)
      blocks_used_.reset(i);

    return true;
  }

  // Tells if an address is contained within the buffer.
  bool IsInAllocatorBuffer(const void* address) const {
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(address);
    return ptr >= buffer_.get() && ptr < buffer_.get() + kBufferSize;
  }

  size_t UsedMemory() const {
    return blocks_used_.count() * BlockSize;
  }

 private:
  std::unique_ptr<uint8_t[]> buffer_;
  std::bitset<NumBlocks> blocks_used_;
};

using TestAllocator = SampleCustomAllocator<256, 1024>;

TEST(Allocator, CallStoredLambda) {
  TestAllocator allocator;

  mf::SetCustomAllocator(
      [](size_t size, size_t alignment, void* context) {
        auto allocator = reinterpret_cast<TestAllocator*>(context);
        return allocator->Allocate(size, alignment);
      }, &allocator,

      [](void* address, size_t size, size_t alignment, void* context) {
        auto allocator = reinterpret_cast<TestAllocator*>(context);
        return allocator->Deallocate(address, size, alignment);
      }, &allocator);

  std::array<uint8_t, 2048> dummy;
  for (size_t i = 0; i < 10; ++i) {
    auto check_allocation = [dummy, &allocator](size_t min_size) {
      EXPECT_TRUE(allocator.IsInAllocatorBuffer(&dummy));
      EXPECT_GE(allocator.UsedMemory(), min_size);
    };

    std::vector<mf::Function<void(size_t)>> functions;
    for (size_t j = 0; j < 10; ++j) {
      functions.push_back(mf::MakeFunction(check_allocation));
      functions.back()(sizeof(dummy) * (j+1));
    }
  }

  // Reset the custom allocator so it does not affect other unit tests.
  mf::SetCustomAllocator(nullptr, nullptr, nullptr, nullptr);
}
