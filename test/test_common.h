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

#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include <string>
#include <utility>

namespace mf {
namespace test {

// Different overload types by cv qualification.
enum class CVQualification {
  kUndefined = 0,
  kNonQualified,
  kConstQualified,
  kVolatileQualified,
  kConstVolatileQualified,
};

// Sample free function using lvalue and rvalue references.
int FreeFunction(bool& called, bool&& value);

// Another free function that returns the sum of two values.
int Sum(int x, int y);

// Sample object holding an integer value.
class Object {
 public:
  // Sets the integer value returned by functions.
  explicit Object(int id);

  // Explicit copy constructors to allow storing volatile objects.
  // Implicit ones do not support volatile.
  Object(const Object& object);
  Object(const volatile Object& object);

  // Returns the sum of the provided arguments and the stored object id.
  int Sum(int x, int y);

  // Example member function using lvalue and rvalue references.
  // Sets in called the provided value and returns the object id.
  int Function(bool& called, bool&& value);

  // Example const member function using lvalue and rvalue references.
  // Sets in called the provided value and returns the object id.
  int ConstFunction(bool& called, bool&& value) const;

  // Example volatile member function using lvalue and rvalue references.
  // Sets in called the provided value and returns the object id.
  int VolatileFunction(bool& called, bool&& value) volatile;

  // Example const volatile member function using lvalue and rvalue references.
  // Sets in called the provided value and returns the object id.
  int ConstVolatileFunction(bool& called, bool&& value) const volatile;

  // Example virtual function using lvalue and rvalue references.
  // Sets in called the provided value and returns the object id.
  // Sets derived to false when called.
  virtual int VirtualFunction(bool& called, bool&& value, bool* derived);

  // Function used to test convertibility of argument and return types.
  // Returns the index-th character of a string.
  char ConvertibleFunction(const std::string& str, const unsigned int index);

  // Overloaded member function by cv qualifiers.
  int Overloaded(CVQualification& cv);
  int Overloaded(CVQualification& cv) const;
  int Overloaded(CVQualification& cv) volatile;
  int Overloaded(CVQualification& cv) const volatile;

 protected:
  int id_;
};

// Override of the virtual function from the class above.
class DerivedObject : public Object {
 public:
  // Sets the integer value returned by functions.
  explicit DerivedObject(int id);

  // Example virtual function using lvalue and rvalue references.
  // Sets in called the provided value and returns the object id.
  // Sets derived to true when called.
  int VirtualFunction(bool& called, bool&& value, bool* derived) override;
};

// Callable object that overloads its operator () with cv qualifiers.
class OverloadedCallable {
 public:
  // Sets the integer value returned by functions.
  explicit OverloadedCallable(int id);

  // Explicit copy constructors to allow storing volatile objects.
  // Implicit ones do not support volatile.
  OverloadedCallable(const OverloadedCallable& callable);
  OverloadedCallable(const volatile OverloadedCallable& callable);

  // Overloads of operator ().
  int operator()(CVQualification& cv);
  int operator()(CVQualification& cv) const;
  int operator()(CVQualification& cv) volatile;
  int operator()(CVQualification& cv) const volatile;

 private:
  int id_;
};

}  // namespace test
}  // namespace mf

#endif  // TEST_COMMON_H_
