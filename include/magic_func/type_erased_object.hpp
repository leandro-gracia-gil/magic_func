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

#ifndef MAGIC_FUNC_TYPE_ERASED_OBJECT_HPP_
#define MAGIC_FUNC_TYPE_ERASED_OBJECT_HPP_

namespace mf {

TypeErasedObject::TypeErasedObject() MF_NOEXCEPT
    : object_ptr_(nullptr),
      destructor_(nullptr),
      copy_constructor_(nullptr),
      move_constructor_(nullptr) {}

TypeErasedObject::TypeErasedObject(const TypeErasedObject& object)
    : destructor_(object.destructor_),
      copy_constructor_(object.copy_constructor_),
      move_constructor_(object.move_constructor_) {

  object_ptr_ = object.HasStoredObject() ?
      (*copy_constructor_)(data_, object.data_) :
      object.object_ptr_;
}

TypeErasedObject::TypeErasedObject(TypeErasedObject&& object) MF_NOEXCEPT
    : TypeErasedObject() {
  std::swap(object_ptr_, object.object_ptr_);
  std::swap(destructor_, object.destructor_);
  std::swap(copy_constructor_, object.copy_constructor_);
  std::swap(move_constructor_, object.move_constructor_);

  if (HasStoredObject())
    object_ptr_ = (*move_constructor_)(data_, object.data_);
}

TypeErasedObject::~TypeErasedObject() {
  if (destructor_)
    (*destructor_)(data_);
}

TypeErasedObject& TypeErasedObject::operator =(
    const TypeErasedObject& object) {
  if (this == &object)
    return *this;

  if (destructor_)
    (*destructor_)(data_);

  destructor_ = object.destructor_;
  copy_constructor_ = object.copy_constructor_;
  move_constructor_ = object.move_constructor_;

  object_ptr_ = HasStoredObject() ?
      (*copy_constructor_)(data_, object.data_) :
      object.object_ptr_;

  return *this;
}

TypeErasedObject& TypeErasedObject::operator =(TypeErasedObject&& object) {
  if (this == &object)
    return *this;

  if (destructor_)
    (*destructor_)(data_);

  object_ptr_ = nullptr;
  destructor_ = nullptr;
  copy_constructor_ = nullptr;
  move_constructor_ = nullptr;

  std::swap(object_ptr_, object.object_ptr_);
  std::swap(destructor_, object.destructor_);
  std::swap(copy_constructor_, object.copy_constructor_);
  std::swap(move_constructor_, object.move_constructor_);

  if (HasStoredObject())
    object_ptr_ = (*move_constructor_)(data_, object.data_);

  return *this;
}

void TypeErasedObject::Reset() {
  if (destructor_)
    (*destructor_)(data_);

  object_ptr_ = nullptr;
  destructor_ = nullptr;
  copy_constructor_ = nullptr;
  move_constructor_ = nullptr;
}

template <typename T>
void TypeErasedObject::StorePointer(const T* object) {
  Reset();
  object_ptr_ = const_cast<std::remove_cv_t<T>*>(object);
}

template <typename T, typename>
void TypeErasedObject::StoreObject(T&& object) {
  // Delete any previously stored object.
  Reset();

  // Store a unique_ptr locally that owns the object in the heap.
  using U = std::decay_t<T>;
  static_assert(sizeof(data_) >= sizeof(CustomUniquePtr<U>),
                "Buffer is too small.");

  static_assert(alignof(DataBuffer<void>) >= alignof(CustomUniquePtr<U>),
                "Incompatible unique_ptr type alignments.");

  const auto& allocator = CustomAllocator();
  CustomUniquePtr<U> heap_obj(nullptr, CustomAllocatorDeleter<U>());

  if (allocator.first) {
    // Use the custom allocator for the object heap data if set.
    void* heap = (*allocator.first)(sizeof(U), alignof(U), allocator.second);
    MAGIC_FUNC_CHECK(heap, Error::kCustomAllocator);
    heap_obj.reset(new (heap) U(std::forward<T>(object)));
  } else {
    // Otherwise use the regular new operator.
    heap_obj.reset(new U(std::forward<T>(object)));
  }

  auto local_unique_ptr = new (data_) CustomUniquePtr<U>(std::move(heap_obj));
  object_ptr_ = const_cast<std::remove_cv_t<U>*>(local_unique_ptr->get());

  copy_constructor_ = &CopyHeapObject<U>;
  move_constructor_ = &MoveSmartPointer<CustomUniquePtr<U>>;
  destructor_ = &DestroyObject<CustomUniquePtr<U>>;
}

template <typename T>
void TypeErasedObject::StoreObject(const std::shared_ptr<T>& object) {
  // Delete any previously stored object.
  Reset();

  static_assert(sizeof(data_) >= sizeof(std::shared_ptr<T>),
                "Buffer is too small.");

  static_assert(alignof(DataBuffer<void>) >= alignof(std::shared_ptr<T>),
                "Incompatbile std::shared_ptr type alignments.");

  // Shared pointers are stored locally.
  auto ptr = new (data_) std::shared_ptr<T>(object);
  object_ptr_ = const_cast<std::remove_cv_t<T>*>(ptr->get());

  copy_constructor_ = &CopySharedPointer<std::shared_ptr<T>>;
  move_constructor_ = &MoveSmartPointer<std::shared_ptr<T>>;
  destructor_ = &DestroyObject<std::shared_ptr<T>>;
}

template <typename T>
void* TypeErasedObject::CopySharedPointer(
    void* dest, const void* src) {
  static_assert(IsSharedPtr<T>::value, "Type is not a shared_ptr.");
  auto src_obj = reinterpret_cast<const T*>(src);
  MAGIC_FUNC_DCHECK(dest, Error::kInvalidObject);
  MAGIC_FUNC_DCHECK(src_obj, Error::kInvalidObject);
  auto ptr = new (dest) T(*src_obj);
  return const_cast<std::remove_cv_t<typename T::element_type>*>(ptr->get());
}

template <typename T>
std::enable_if_t<std::is_copy_constructible<T>::value, void*>
TypeErasedObject::CopyHeapObject(void* dest, const void* src) {
  auto src_obj = reinterpret_cast<const CustomUniquePtr<T>*>(src);
  MAGIC_FUNC_DCHECK(dest, Error::kInvalidObject);
  MAGIC_FUNC_DCHECK(src_obj, Error::kInvalidObject);

  const auto& allocator = CustomAllocator();
  CustomUniquePtr<T> heap_obj(nullptr, CustomAllocatorDeleter<T>());

  if (allocator.first) {
    void* heap = (*allocator.first)(sizeof(T), alignof(T), allocator.second);
    MAGIC_FUNC_CHECK(heap, Error::kCustomAllocator);
    heap_obj.reset(new (heap) T(*src_obj->get()));
  } else {
    heap_obj.reset(new T(*src_obj->get()));
  }

  auto ptr = new (dest) CustomUniquePtr<T>(std::move(heap_obj));
  return const_cast<std::remove_cv_t<T>*>(ptr->get());
}

template <typename T>
std::enable_if_t<!std::is_copy_constructible<T>::value, void*>
TypeErasedObject::CopyHeapObject(void*, const void*) {
  // We're trying to copy a non-copyable object.
  MAGIC_FUNC_ERROR(Error::kNonCopyableObject);
  return nullptr;
}

template <typename T>
void* TypeErasedObject::MoveSmartPointer(void* dest, void* src) {
  static_assert(IsUniquePtr<T>::value || IsSharedPtr<T>::value,
                "Type is not a unique_ptr or shared_ptr.");
  auto src_obj = reinterpret_cast<T*>(src);
  MAGIC_FUNC_DCHECK(dest, Error::kInvalidObject);
  MAGIC_FUNC_DCHECK(src_obj, Error::kInvalidObject);
  auto ptr = new (dest) T(std::move(*src_obj));
  return const_cast<std::remove_cv_t<typename T::element_type>*>(ptr->get());
}

template <typename T>
void TypeErasedObject::DestroyObject(void* obj_erased) {
  auto obj = reinterpret_cast<T*>(obj_erased);
  MAGIC_FUNC_DCHECK(obj, Error::kInvalidObject);
  obj->~T();
}

template <typename T>
void TypeErasedObject::CustomAllocatorDeleter<T>::operator ()(T* ptr) {
  const auto& deallocator = CustomDeallocator();
  if (deallocator.first) {
    if (!(*deallocator.first)(ptr, sizeof(T), alignof(T), deallocator.second)) {
      MAGIC_FUNC_CHECK(false, Error::kCustomAllocator);
    }
  } else {
    delete ptr;
  }
}

}  // namespace mf

#endif  // MAGIC_FUNC_TYPE_ERASED_OBJECT_HPP_
