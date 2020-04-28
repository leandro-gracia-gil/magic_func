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

#ifndef MAGIC_FUNC_TYPE_ERASED_OBJECT_H_
#define MAGIC_FUNC_TYPE_ERASED_OBJECT_H_

#include <type_traits>

#include <magic_func/allocator.h>
#include <magic_func/error.h>
#include <magic_func/type_traits.h>

namespace mf {

// Encapsulates a type-erased object.
//
// The object can be handled in three different ways:
// 1. The class simply contains a type-erased pointer to an external object.
//    This can be achieved with the StorePointer function.
//
// 2. The class contains a provided shared pointer in its data buffer.
//    Happens when calling StoreObject explicitly with a shared pointer.
//
// 3. The class contains a unique pointer in its data buffer that points to the
//    real object in heap memory. Happens when calling StoreObject otherwise.
//
// Stored objects get their copy constructors and destructors called when
// appropriate despite type erasure. See StoreObject for more details.
class TypeErasedObject {
 public:
  inline TypeErasedObject() noexcept;
  inline TypeErasedObject(const TypeErasedObject& object);
  inline TypeErasedObject(TypeErasedObject&& object) noexcept;
  inline ~TypeErasedObject();

  inline TypeErasedObject& operator =(const TypeErasedObject& object);
  inline TypeErasedObject& operator =(TypeErasedObject&& object);

  // Tells if an object is being encapsulatd or referenced.
  explicit operator bool() const noexcept { return object_ptr_ != nullptr; }

  // Tells if an object is currently being stored.
  // This can either mean in the current instance or in the heap.
  bool HasStoredObject() const noexcept { return destructor_ != nullptr; }

  // Returns the referenced or stored object, if any.
  void* GetObject() const noexcept { return object_ptr_; }

  // Deletes any stored object and cleans any object references.
  inline void Reset();

  // Stores a reference to an object. Any previously stored object is destroyed.
  // Constness and volatility are casted away and must be added back if desired
  // by callers.
  template <typename T>
  void StorePointer(const T* object);

  // Stores an object type-erasing it.
  //
  // Instances of this class will still call the appropriate destructors and
  // copy constructors of the stored object despite the type erasure.
  // In particular:
  //
  // 1. If the object is a std::shared_ptr, the shared pointer will be copied
  //    within the TypeErasedObject. Copying and moving the TypeErasedObject
  //    will also copy and move the shared pointer.
  //
  // 2. For any other case, the object will be copied or moved depending on the
  //    argument into a std::unique_ptr stored within the TypeErasedObject.
  //    Copying the TypeErasedObject will create new copies of the stored object
  //    using its copy constructor. Moving it will just move the std::unique_ptr
  //    containing it.
  //
  // To ensure correct copyability and moveability of TypeErasedObjects, objects
  // stored within them must be copy constructible. Trying to make a copy of a
  // TypeErasedObject encapsulating an object that is not copy constructible
  // will raise a kNonCopyable fatal error at runtime. No move constructors are
  // needed, as only the smart pointers containing them will be moved.
  //
  // Note that copy-assignment is not used. Assigning two TypeErasedObjects will
  // will make use of the object destructor and copy constructor instead of its
  // assignment operator. This is because some callable objects such as lambdas
  // define a copy constructor but not a copy assignment operator.
  //
  // Note: this function uses universal references. As such, object can both
  // refer to lvalue and rvalue references and T can be a reference type.
  template <typename T, typename = std::enable_if_t<!IsSharedPtr<T>::value>>
  void StoreObject(T&& object);

  // Special version of StoreObject for shared pointers to objects.
  // Stores a shared_ptr object locally.
  //
  // Note: T can have const and volatile qualifications depending on the
  // provided shared_ptr.
  template <typename T>
  void StoreObject(const std::shared_ptr<T>& object);

 private:
  // Copies a type-erased locally stored shared pointer.
  template <typename T>
  static void* CopySharedPointer(void* dest, const void* src);

  // Copies a type-erased stored object in the heap.
  template <typename T>
  static std::enable_if_t<std::is_copy_constructible<T>::value, void*>
  CopyHeapObject(void* dest, const void* src);

  // Raises an error if trying to copy a non-copyable object.
  template <typename T>
  static std::enable_if_t<!std::is_copy_constructible<T>::value, void*>
  CopyHeapObject(void* dest, const void* src);

  // Moves a type-erased smart pointer.
  // T is either a std::unique_ptr or a std::shared_ptr.
  template <typename T>
  static void* MoveSmartPointer(void* dest, void* src);

  // Destroys a type-erased stored object.
  template <typename T>
  static void DestroyObject(void* obj_erased);

  using TypeErasedDestructor = void (*)(void*);
  using TypeErasedCopyConstructor = void* (*)(void*, const void*);
  using TypeErasedMoveConstructor = void* (*)(void*, void*);

  // Deleter for std::unique_ptr that uses the current custom deallocation
  // function if any is set.
  template <typename T>
  struct CustomAllocatorDeleter {
    void operator ()(T* ptr);
  };

  // Version of std::unique_ptr that uses a deleter compatible with custom
  // allocators.
  template <typename T>
  using CustomUniquePtr = std::unique_ptr<T, CustomAllocatorDeleter<T>>;

  // Possible contents of the data buffer.
  template <typename T>
  union DataBuffer {
    ~DataBuffer() = delete;
    CustomUniquePtr<T> unique_ptr;
    std::shared_ptr<T> shared_ptr;
  };

  // Type-erased data buffer. Used to store a DataBuffer union of an erased type
  // within the local object. Size and alignment compatibility with the actual
  // stored type is statically asserted.
  //
  // This buffer allows to store smart pointers of different types directly.
  // Otherwise, trying to store typed pointers like a shared_ptr to an object
  // would involve an extra indirection in heap memory.
  alignas(DataBuffer<void>) uint8_t data_[sizeof(DataBuffer<void>)];

  // The object this object refers to. Can point to:
  // 1. An external object.
  // 2. An address in the heap, owned by objects encoded in the data array.
  void* object_ptr_;

  // When not null, points to a function that triggers the appropriate
  // destructor of the object stored in data.
  TypeErasedDestructor destructor_;

  // When not null, points to a functions that copies the object represented
  // in a data buffer into another.
  TypeErasedCopyConstructor copy_constructor_;

  // When not null, points to a functions that moves the object represented
  // in a data buffer into another.
  TypeErasedMoveConstructor move_constructor_;
};

}  // namespace mf

#include <magic_func/type_erased_object.hpp>

#endif  // MAGIC_FUNC_TYPE_ERASED_OBJECT_H_
