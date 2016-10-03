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

// This test needs C++ exceptions thrown by MagicFunc exceptions to work.
// These exceptions are turned off in release builds that define NDEBUG.
#undef NDEBUG

#include <magic_func/type_erased_object.h>
#include <gtest/gtest.h>

using namespace mf;

namespace {

// Cannot use gmock here for various reasons.
// 1. Mocked objects are not copyable, and copying them as undefined behavior.
// 2. We want to keep track of the number of calls across the object including
//    copies, moves and destructions. The latter is not possible with gmock.
class Object {
 public:
  Object(size_t* copy_constructor_calls = nullptr,
         size_t* move_constructor_calls = nullptr,
         size_t* destructor_calls = nullptr)
      : copy_constructor_calls_(copy_constructor_calls),
        move_constructor_calls_(move_constructor_calls),
        destructor_calls_(destructor_calls) {
    if (copy_constructor_calls_)
      *copy_constructor_calls_ = 0;

    if (move_constructor_calls_)
      *move_constructor_calls_ = 0;

    if (destructor_calls_)
      *destructor_calls = 0;
  }

  Object(const Object& dummy)
      : copy_constructor_calls_(dummy.copy_constructor_calls_),
        move_constructor_calls_(dummy.move_constructor_calls_),
        destructor_calls_(dummy.destructor_calls_) {
    if (copy_constructor_calls_)
      ++(*copy_constructor_calls_);
  }

  Object(Object&& dummy)
      : copy_constructor_calls_(dummy.copy_constructor_calls_),
        move_constructor_calls_(dummy.move_constructor_calls_),
        destructor_calls_(dummy.destructor_calls_) {
    if (move_constructor_calls_)
      ++(*move_constructor_calls_);
  }

  ~Object() {
    if (destructor_calls_)
      ++(*destructor_calls_);
  }

 private:
  size_t* copy_constructor_calls_;
  size_t* move_constructor_calls_;
  size_t* destructor_calls_;
};

// Class that is not copy constructible.
class NonCopyable {
 public:
  NonCopyable() {}
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable(NonCopyable&&) = default;
};

// Double-check that copy constructability detection works.
static_assert(!std::is_copy_constructible<NonCopyable>::value,
              "The NonCopyable class must not be copy constructible.");

}  // anonymous namespace

TEST(TypeErasedObject, TestEmpty) {
  TypeErasedObject test;
  EXPECT_FALSE(test.HasStoredObject());
  EXPECT_FALSE(test);
  EXPECT_EQ(nullptr, test.GetObject());
}

TEST(TypeErasedObject, StorePointer) {
  TypeErasedObject test;

  size_t copied, moved, destroyed;
  Object object(&copied, &moved, &destroyed);

  test.StorePointer(&object);
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  EXPECT_FALSE(test.HasStoredObject());
  EXPECT_TRUE(test);
  EXPECT_EQ(&object, test.GetObject());
}

TEST(TypeErasedObject, StoreObject) {
  TypeErasedObject test;
  size_t copied, moved, destroyed;
  Object object(&copied, &moved, &destroyed);

  // Copy the object.
  test.StoreObject(object);
  EXPECT_EQ(1, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  EXPECT_TRUE(test.HasStoredObject());
  EXPECT_TRUE(test);
  EXPECT_NE(&object, test.GetObject());

  // Then we move the object. It should destroy the previous copy first.
  test.StoreObject(std::move(object));
  EXPECT_EQ(1, copied);
  EXPECT_EQ(1, moved);
  EXPECT_EQ(1, destroyed);

  EXPECT_TRUE(test.HasStoredObject());
  EXPECT_TRUE(test);

  // Make the type-erased object release anything it has.
  test.Reset();
  EXPECT_EQ(1, copied);
  EXPECT_EQ(1, moved);
  EXPECT_EQ(2, destroyed);

  EXPECT_FALSE(test.HasStoredObject());
  EXPECT_FALSE(test);
}

TEST(TypeErasedObject, StoreObjectSharedPtr) {
  TypeErasedObject test;
  size_t copied, moved, destroyed;
  auto object = std::make_shared<Object>(&copied, &moved, &destroyed);

  // Copy the object shared pointer. The object itself is not copied.
  test.StoreObject(object);
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  EXPECT_TRUE(test.HasStoredObject());
  EXPECT_TRUE(test);
  EXPECT_EQ(test.GetObject(), object.get());

  // Reset the type-erased object. Since we still have a reference to the
  // stored object it should not have been destroyed.
  test.Reset();
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  EXPECT_FALSE(test.HasStoredObject());
  EXPECT_FALSE(test);

  // Reset the last reference. The object should now be destroyed.
  object.reset();
  EXPECT_EQ(1, destroyed);
}

TEST(TypeErasedObject, StoreObjectSharedPtrConst) {
  TypeErasedObject test;

  size_t copied, moved, destroyed;
  auto object = std::make_shared<const Object>(&copied, &moved, &destroyed);

  // Copy the object shared pointer. The object itself is not copied.
  test.StoreObject(object);
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  EXPECT_TRUE(test.HasStoredObject());
  EXPECT_TRUE(test);
  EXPECT_EQ(test.GetObject(), object.get());

  // Reset the type-erased object. Since we still have a reference to the
  // stored object it should not have been destroyed.
  test.Reset();
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  EXPECT_FALSE(test.HasStoredObject());
  EXPECT_FALSE(test);

  // Reset the last reference. The object should now be destroyed.
  object.reset();
  EXPECT_EQ(1, destroyed);
}

TEST(TypeErasedObject, StoreObjectsOfDifferentTypes) {
  TypeErasedObject test;
  size_t object_copied, object_moved, object_destroyed;
  Object object(&object_copied, &object_moved, &object_destroyed);

  size_t shared_copied, shared_moved, shared_destroyed;
  std::shared_ptr<Object> shared_object = std::make_shared<Object>(
      &shared_copied, &shared_moved, &shared_destroyed);

  // Store an object.
  test.StoreObject(object);
  EXPECT_EQ(1, object_copied);
  EXPECT_EQ(0, object_moved);
  EXPECT_EQ(0, object_destroyed);
  EXPECT_EQ(0, shared_copied);
  EXPECT_EQ(0, shared_moved);
  EXPECT_EQ(0, shared_destroyed);

  // Store a shared pointer to an object.
  test.StoreObject(shared_object);
  EXPECT_EQ(1, object_copied);
  EXPECT_EQ(0, object_moved);
  EXPECT_EQ(1, object_destroyed);
  EXPECT_EQ(0, shared_copied);
  EXPECT_EQ(0, shared_moved);
  EXPECT_EQ(0, shared_destroyed);

  // Reset the local reference to the shared object.
  shared_object.reset();

  // Store an object pointer. Should destroy the stored object.
  test.StorePointer(&object);
  EXPECT_EQ(1, object_copied);
  EXPECT_EQ(0, object_moved);
  EXPECT_EQ(1, object_destroyed);
  EXPECT_EQ(0, shared_copied);
  EXPECT_EQ(0, shared_moved);
  EXPECT_EQ(1, shared_destroyed);

  // Move the first object back.
  test.StoreObject(std::move(object));
  EXPECT_EQ(1, object_copied);
  EXPECT_EQ(1, object_moved);
  EXPECT_EQ(1, object_destroyed);
  EXPECT_EQ(0, shared_copied);
  EXPECT_EQ(0, shared_moved);
  EXPECT_EQ(1, shared_destroyed);
}

TEST(TypeErasedObject, CopyTypeErasedObjects) {
  TypeErasedObject test;
  size_t copied, moved, destroyed;
  Object object(&copied, &moved, &destroyed);

  // Object is copied inside the type-erased object.
  test.StoreObject(object);
  EXPECT_EQ(1, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Nothing should happen either if we try to self-assign.
  test = test;
  EXPECT_EQ(1, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Same for move-assignment.
  test = std::move(test);
  EXPECT_EQ(1, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Type-erased object is copied. The object inside it is copied too.
  TypeErasedObject test_copy = test;
  EXPECT_EQ(2, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Same should happen with if the assignment operator is used.
  // The previous object should be destroyed.
  test_copy = test;
  EXPECT_EQ(3, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(1, destroyed);

  // No object is destroyed if nothing was contained.
  TypeErasedObject test_copy_empty;
  test_copy_empty = test;
  EXPECT_EQ(4, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(1, destroyed);

  // The type-erased object is moved, but only the unique_ptr owning the object
  // in the heap is moved. The object itself is not.
  TypeErasedObject test_move = std::move(test_copy);
  EXPECT_EQ(4, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(1, destroyed);

  // Same should happen with if the move-assignment operator is used.
  // The previous stored object is destroyed.
  test = std::move(test_move);
  EXPECT_EQ(4, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(2, destroyed);

  // No object is destroyed if moving into an empty object.
  TypeErasedObject test_move_empty;
  test_move_empty = std::move(test_copy_empty);
  EXPECT_EQ(4, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(2, destroyed);

  // Move the object into the type-erased object.
  // The previously stored object is destroyed.
  test.StoreObject(std::move(object));
  EXPECT_EQ(4, copied);
  EXPECT_EQ(1, moved);
  EXPECT_EQ(3, destroyed);

  // Reset the copies we have of the original type-erased object.
  test.Reset();
  test_move_empty.Reset();
  EXPECT_EQ(4, copied);
  EXPECT_EQ(1, moved);
  EXPECT_EQ(5, destroyed);
}

TEST(TypeErasedObject, CopyTypeErasedObjectsShared) {
  TypeErasedObject test;
  size_t copied, moved, destroyed;
  auto object = std::make_shared<Object>(&copied, &moved, &destroyed);

  // Store the shared pointer to the object. The object itself is not copied.
  test.StoreObject(object);
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Nothing should happen either if we try to self-assign.
  // This is tested when there's only one reference, as if the self-assign check
  // failed it would destroy the object.
  test = test;
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Same for move-assignment.
  test = std::move(test);
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Type-erased object is copied. The object inside is not copied, but its
  // shared pointer is.
  TypeErasedObject test_copy = test;
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Same should happen with if the assignment operator is used.
  // The previous object should not be destroyed because it's still referenced.
  test_copy = test;
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // No object is destroyed either if nothing was contained.
  TypeErasedObject test_copy_empty;
  test_copy_empty = test;
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Type-erased object is moved. The object inside it's not moved, but again
  // only its shared pointer is.
  TypeErasedObject test_move = std::move(test_copy);
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Same should happen with if the move-assignment operator is used.
  // The previous stored object is destroyed.
  test = std::move(test_move);
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // No object is destroyed if moving into an empty object.
  TypeErasedObject test_move_empty;
  test_move_empty = std::move(test_copy_empty);
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Release the copy of the object we have.
  object.reset();

  // Destroy the original type-erased object. Since test_move_empty is still
  // around the object is not destroyed yet.
  test.Reset();
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(0, destroyed);

  // Destroy test_move, which should be the last reference to the object.
  // It should now be destroyed.
  test_move_empty.Reset();
  EXPECT_EQ(0, copied);
  EXPECT_EQ(0, moved);
  EXPECT_EQ(1, destroyed);
}

TEST(TypeErasedObject, NonCopyableObject) {
  TypeErasedObject test;
  test.StoreObject(NonCopyable());

  try {
    // Moving should work.
    TypeErasedObject f1 = std::move(test);
    SUCCEED();

    // Copying should not.
    TypeErasedObject f2 = f1;
    FAIL();

  } catch (Error error) {
    EXPECT_EQ(Error::kNonCopyableObject, error);
  }
}
