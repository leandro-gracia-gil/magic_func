# MagicFunc
## Introduction
MagicFunc is a faster alternative to std::function featuring type erasure.

It's main features are:
- Supports most features of std::function including lambdas and other callable objects.
- Allows storing function objects of different types together thanks to function type erasure.
- Requires only C++11 support. Works with MSVC 2015, GCC and Clang.
- Measured speed-ups up to 2x against std::function.
- Compatible with std::function and std::bind.
- All features are extensively unit tested.

Other features:
- Provides automatic type deduction mechanisms for functions.
- Full support and correctness for const and volatile qualifiers.
- Correct support for mutable lambdas and functors.
- Correctly supports  lvalue and rvalue reference argument types in functions.
- Copy, move, destruction and alignment correctness when managing type-erased objects.
- Integrated support for binding std::shared_ptr pointing to objects and retaining them.
- Supports overloaded functions and provides mechanisms to disambiguate all cases.
- No virtual functions or C++ RTTI functionalities are used. Heap memory is only used on lambdas and callables.
- Provides a flexible error system for managing invalid operations.
- Support for custom allocators.

## Basics
MagicFunc has two main classes:
- **mf::Function<*function_type*\>**: similar to std::function, takes a function type and allows calling it.
- **mf::MemberFunction<*member_function_pointer_type*\>**: allows calling a member function of a class, provided an object.

Both classes derive from **mf::TypeErasedFunction**, which holds all the required information. Type-erased functions can be safely copied and moved but not invoked.
To cast back to a mf::Function or a mf::MemberFunction, **mf::FunctionCast** must be used. This performs a fast runtime type check to ensure the cast is valid and raises an error otherwise.

 MagicFunc features automatic type deduction for functions that are not overloaded. This can be done by  calling **mf::MakeFunction**, which will return a mf::Function or a mf::MemberFunction object depending on the provided arguments.
When trying to use mf::MakeFunction with explicit function addresses (not pointers) it's simpler to use the auxiliary macro **MF_MakeFunction**. This macro simplifies the syntax involved in passing the function address as a template argument.

See the section below for examples of each use case.

## Examples
### Calling a free or static function
```c++
#include <magic_func/function.h>
#include <magic_func/make_function.h>

// Here's a free function to call.
int Foo();

// Here's a class with a static function to call.
class Object {
 public:
  static void Bar();
};

// Like std::function, but using the MF_MakeFunction helper macro.
mf::Function<int(void)> foo1 = MF_Makefunction(&Foo);
foo1();

// MagicFunc can also guess your function type for you if it's not overloaded.
auto foo2 = MF_Makefunction(&Foo);  // Returns a mf::Function<int(void)>.
foo2();

auto bar2 = MF_Makefunction(&Object::Bar);  // Returns a mf::Function<void()>.
bar2();
```

### Calling a member function
```c++
#include <magic_func/make_function.h>
#include <magic_func/member_function.h>

// Example class with member functions to call.
class Object {
 public:
  void Foo(int x);
};

// MagicFunc has a wrapper object for member functions called mf::MemberFunction<T> with T being a member function pointer type.
mf::MemberFunction<void (Object::*)(int)> foo1 = MF_MakeFunction(&Object::Foo);

// That member function pointer syntax looks a bit too terrible. We can do better.
mf::MemberFunction<decltype(&Object::Foo)> foo2 = MF_MakeFunction(&Object::Foo);

// In fact, since Object::Foo is not overloaded we can just let it guess the type for us.
auto foo3 = MF_MakeFunction(&Object::Foo);

// Then member functions can be invoked by passing an object as the first argument.
Object object;
foo1(object, 42);

```

### Binding objects to member functions
```c++
#include <magic_func/function.h>
#include <magic_func/make_function.h>
#include <magic_func/member_function.h>

// Example class with member functions to call.
class Object {
 public:
  void Foo(int x);
};

// Unlike with std::function, there's no need for std::bind or an equivalent to bind member functions to objects.
// All you need is to pass the object when setting the member function. There are different ways to do this.
// You can pass a pointer to the object, in which case you must guarantee its validity at the time of any call.
Object object;
auto foo = MF_MakeFunction(&Object::Foo, &object);
foo(42);

// You can also pass a shared pointer to the object so that the mf::Function object keeps it alive.
// This guarantees the lifetime of the called object.
std::shared_ptr<Object> object_shared = std::make_shared<Object>();
auto foo_shared = MF_MakeFunction(&Object::Foo, object);
foo_shared(42);

// If you want to ensure the lifetime of the object but don't want to keep it alive you can use a std::weak_ptr with a lambda.
// Support for std::weak_ptr is not directly integrated because it causes ambiguity with the cast from shared to weak_ptrs.
std::weak_ptr<Object> object_weak = object_shared;
mf::Function<void()> foo_weak = [object_weak](int x) {
  std::shared_ptr<Object> object = object_weak.lock();
  if (object)
    object->Foo(x);
};
foo_weak(42);

// Finally, you can also bind objects to mf::MemberFunctions to create mf::Functions.
mf::MemberFunction<decltype(&Object::Foo)> foo_member_func = MF_MakeFunction(&Object::Foo);
mf::Function<void()> foo_member(foo_member_func, &object);
foo_member(42);
```

### Calling lambdas
```c++
#include <magic_func/function.h>
#include <magic_func/make_function.h>

// Just as you would do with std::function.
mf::Function<int(int, int)> lambda1 = [](int x, int y) { return x + y; };

// In fact, you can let MagicFunc deduce the type for you.
auto lambda2 = mf::MakeFunction([](int x, int y) { return x + y; });

// Not only that, but you can actually have different lambda and function types as long as they are convertible.
// In particular, function arguments must convert to lambda arguments, and the lambda return type must convert to the function one.
auto get_string_char = [](const std::string& str, int i) { return str[i]; };
mf::Function<short(const char*, float)> lambda3 = get_string_char;
lambda3("test", 1);  // Returns a short containing 'e'.
```

### Using function type erasure
```c++
#include <magic_func/function.h>
#include <magic_func/make_function.h>
#include <vector>

// Example class with member functions to call.
class Object {
 public:
  int Foo(int x);
  static void Bar(const std::string& str);
};

// Let's create functions of different types.
mf::MemberFunction<decltype(&Object::Foo)> foo_member_func = MF_MakeFunction(&Object::Foo);
mf::Function<int(int)> foo = mf::MakeFunction(foo_member_func, std::make_shared<Object>());
mf::Function<void(const std::string&)> bar = MF_MakeFunction(&Object::Bar);

// Since all those derive from mf::TypeErasedFunction we can just store them in a vector.
std::vector<mf::TypeErasedFunction> many_functions = { foo_member_func, foo, bar };

// We can copy and move them as we like. Even if a shared_pointer is bound inside everything is still taken care of internally.
mf::TypeErasedFunction type_erased_foo = many_functions[1];
mf::TypeErasedFunction type_erased_bar = std::move(many_functions[2]);

// We can undo type erasure by using mf::FunctionCast.
auto foo_restored = mf::FunctionCast<int(int)>(type_erased_foo);  // Returns a mf::Function<int(int)>.
foo_restored(42);

Object object;
auto foo_func_restored = mf::FunctionCast<decltype(&Object::Foo)>(many_functions[0]);  // Returns a mf::MemberFunction<decltype(&Object::Foo)>.
foo_func_restored(object, 42);

// However, note that a runtime type check is performed when calling mf::FunctionCast.
// If we try something invalid we will get an error. What this error does depends on our error.h configuration and the type of build.
// By default, exceptions are disabled if the NDEBUG macro is defined. This is usual in release builds.
// If exceptions are enabled then we can catch the failure. If they are not we would terminate.
// For illustrative purposes let's assume exceptions are enabled.
try {
  auto invalid_cast = mf::FunctionCast<void()>(type_erased_foo);
} catch (mf::Error error) {
  assert(error == mf::Error::kInvalidCast);
}

// Similarly, we will get an error if we try to copy or move incompatible type-erased functions.
// Remember that despite being erased the functions do have a type.
try {
  // This works fine. The real type is the same.
  type_erased_foo = many_functions[1];

  // This works fine too. The type-erased function object has no type defined yet, but it does after this.
  mf::TypeErasedFunction some_function = type_erased_foo;

  // This fails. The type is not the same.
  some_function = type_erased_bar;

} catch (mf::Error error) {
  assert(error == mf::Error::kInvalidCast);
}
```

### Disambiguating overloaded functions
```c++
#include <magic_func/function.h>
#include <magic_func/make_function.h>

// Example class with overloaded methods.
class Object {
 public:
  int Foo(int x) { return 1; }
  int Foo(const std::string& str) { return 2; }

  int Bar() { return 1; }
  int Bar() const { return 2; }
};

Object object;

// This will fail because it won't know which version to call.
auto function = MF_MakeFunction(&Object::Foo, &object);

// Disambiguating by type signature.
// In this case the function type in mf::Function is enough for it to know which version of Object::Foo to use.
auto foo_1 = mf::Function<int(int)>::FromMemberFunction<Object, &Object::Foo>(&object);
auto foo_2 = mf::Function<int(const std::string&)>::FromMemberFunction<Object, &Object::Foo>(&object);

// Disambiguating by const volatile qualifiers.
// In this case the function signature is not enough. We need to specify th const-volatile qualifiers of the object type.
// The object type argument must have the same exact qualifications as the function we're targeting or it will fail to find it.
auto bar_1 = mf::Function<void(int)>::FromMemberFunction<Object, &Object::Bar>(&object);  // Returns 1 when called.
auto bar_2 = mf::Function<void(int)>::FromMemberFunction<const Object, &Object::Bar>(&object);  // Returns 2 when called.
```

## Frequently Asked Questions
### How do I use MagicFunc in my project? Does it have any dependencies?

MagicFunc is a header-only library, so you only need to copy the contents of the include folder somewhere in your include path.
No dependencies are required to use MagicFunc. The googletest library is used for unit testing, but it's not required otherwise.

### Is there anything that std::function supports but mf::Function or mf::MemberFunction do not?

Yes, mf::Function and mf::MemberFunction cannot take function or member function pointers, but only explicit function addresses that have linkage. This is because the addresses are passed as template arguments.
Supporting function and member function pointers is possible, but it would require practices that are non-compliant with the standard (member function pointers cannot be type-erased) and would affect overall performance.

However, it is still possible to use a lambda to achieve the same:
```c++
class Object {
 public:
  static int Foo(int arg);
  int Bar(int arg);
};

// Just to make things more readable.
using FooFuncPtr = decltype(&Object::Foo);  // This is a regular function pointer type: int (*)(int).
using BarFuncPtr = decltype(&Object::Bar);  // This is a member function pointer type: int (Object::*)(int).

// This works.
auto foo1 = MF_MakeFunction(&Object::Foo);  // Returns a mf::Function<int(int)> object.
auto bar1 = MF_MakeFunction(&Object::Bar);  // Returns a mf::MemberFunction<int (Object::*)(int)> object.

// This does not.
FooFuncPtr foo_ptr = &Object::Foo;
auto foo2 = MF_MakeFunction(foo_ptr);

BarFuncPtr bar_ptr = &Object::Bar;
auto bar2 = MF_MakeFunction(bar_ptr);

// But you can do this instead.
auto foo3 = mf::MakeFunction([foo_ptr](int arg) { return (*foo_ptr)(arg); });
foo3(42);

Object object;
auto bar3 = mf::MakeFunction([&object, bar_ptr](int arg) { return (object.*bar_ptr)(arg); });
bar3(42);
```

### Is there a mf::Bind or some alternative to std::bind?

There's no mf::Bind because we consider it's not worth the complexity it brings to the code when a simple lambda can be used instead. Using std::bind can be more error-prone than it seems.
```c++
int Foo(int arg);

class Object {
 public:
  int Bar(int arg1, int arg2);
};

// This is what you would do with std::bind.
auto foo1 = std::bind(&Foo, 32, std::placeholders::_1);
foo1(64);

Object object;
auto bar1 = std::bind(&Object::Bar, &object, 32, std::placeholders::_1);
bar1(64);

// With MagicFunc you can do this instead.
auto foo2 = mf::MakeFunction([](int arg2) { return Foo(32, arg2); });
foo2(64);

auto bar2 = mf::MakeFunction([&object](int arg2) { return object.Bar(32, arg2); });
bar2(64);
```

### When should MF_MakeFunction and mf::MakeFunction be used?

In reality one uses the other. MF_MakeFunction is just a macro that saves us some syntax ugliness that we would otherwise have trying to pass function pointers as template arguments.
In general the rule of thumb is: if you are passing a function or member function address directly use the **MF_MakeFunction** macro. Otherwise (like passing lambdas and mf::MemberFunction objects) use **mf::MakeFunction**.
```c++
class Object {
 public:
  void Foo(int x);
};

Object object;

// These two are equivalent. This is what MF_MakeFunction is saving you to write.
auto foo1 = MF_MakeFunction(&Object::Foo, &object);  // Returns a mf::Function<void(int)>.
auto foo2 = mf::MakeFunction<decltype(&Object::Foo), &Object::Foo>(&object);

// Then, this is what mf::MakeFunction is avoiding you to do manually.
auto foo3 = mf::Function<void(int)>::FromMemberFunction<Object, &Object::Foo>(&object);

// When not passing a function address, like when using lambdas, just use mf::MakeFunction directly instead.
auto bar = mf::MakeFunction([](int x, int y) { return x + y; });

// Also the same when using mf::MemberFunction.
mf::MemberFunction<decltype(&Object::Foo)> member_function = MF_MakeFunction(&Object::Foo);
auto foo4 = mf::MakeFunction(member_function, &object);  // Returns a mf::Function<void(int)>.
```

### How does MagicFunc perform against other std::function alternatives like impossibly fast delegates?

Benchmarking against a [C++11 version of fast delegates](http://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11) suggests that both have a very similar performance when using Clang, and that MagicFunc performs about 10~15% better in some cases when using modern versions of GCC. It is not possible to compare directly with that fast delegate implementation using MSVC 2015 as its code does not build. In all measured cases MagicFunc is at least as good as std::function, most times notably faster.

### Can I mix std::function and std::bind with mf::Function?

Yes, you can without any additional effort in both directions. This is because all std::function, std::bind and mf::Function act as callables, so this goes back to the callable / lambda case.
However, keep in mind that if you make a mf::Function call a std::function you will hit the performance costs of both.

### Is heap memory used? Can I use my custom allocators if so?

Heap memory is only used when callables and lambdas are involved. This is because the size of a lambda depends on its capture, so it's not possible to allocate anything ahead of time.
Also, experiments showed that trying to store objects within a mf::TypeErasedFunction performed worse than allocating small amounts of memory in the heap, and had potential alignment issues.

If desired, it is possible to use custom allocators for any heap allocations performed by MagicFunc. To do so, use the SetCustomAllocator function.
```c++
// In this example we just map to an allocator object, but you can do your own.
std::allocator<uint8_t> some_allocator;

mf::SetCustomAllocator(
    // Allocation function.
    [](size_t size, size_t alignment, void* context) {
      auto allocator = reinterpret_cast<std::allocator<uint8_t>>(context);
      return allocator->allocate(size);
    }, &some_allocator,

    // Deallocation function.
    [](void* ptr, size_t size, size_t alignment, void* context) {
      auto allocator = reinterpret_cast<std::allocator<uint8_t>>(context);
      return allocator->deallocate(ptr, size);
    }, &some_allocator);

// This now uses the provided allocator.
int x;
auto func = mf::MakeFunction([=](int y) { return x + y; });
```

However, note that allocators are set globally (statically) for all MagicFunc. Any required allocators should be set once before any other MagicFunc use and not changed again. This is because mf::Function objects do not save which allocators they use, as it would imply a noticeable increase in the size of all mf::Function objects. Changing allocators while MagicFunc objects exist can lead to issues like deallocations on incorrect functions.

Alternatively, overloading the operator new works as usual without the need of defining custom allocators.
