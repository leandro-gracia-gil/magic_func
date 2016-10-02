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

#include "test_common.h"

namespace mf {
namespace test {

int FreeFunction(bool& called, bool&& value) {
  called = std::move(value);
  return 42;
}

int Sum(int x, int y) {
  return x + y;
}

Object::Object(int id) : id_(id) {}

Object::Object(const Object& object) : id_(object.id_) {}

Object::Object(const volatile Object& object) : id_(object.id_) {}

int Object::Sum(int x, int y) {
  return id_ + x + y;
}

int Object::Function(bool& called, bool&& value) {
  called = std::move(value);
  return id_;
}

int Object::ConstFunction(bool& called, bool&& value) const {
  called = std::move(value);
  return id_;
}

int Object::VolatileFunction(bool& called, bool&& value) volatile {
  called = std::move(value);
  return id_;
}

int Object::ConstVolatileFunction(bool& called, bool&& value) const volatile {
  called = std::move(value);
  return id_;
}

int Object::VirtualFunction(bool& called, bool&& value, bool* derived) {
  called = std::move(value);
  *derived = false;
  return id_;
}

char Object::ConvertibleFunction(const std::string& str,
                                 const unsigned int index) {
  return str[index];
}

int Object::Overloaded(CVQualification& cv) {
  cv = CVQualification::kNonQualified;
  return id_;
}

int Object::Overloaded(CVQualification& cv) const {
  cv = CVQualification::kConstQualified;
  return id_;
}

int Object::Overloaded(CVQualification& cv) volatile {
  cv = CVQualification::kVolatileQualified;
  return id_;
}

int Object::Overloaded(CVQualification& cv) const volatile {
  cv = CVQualification::kConstVolatileQualified;
  return id_;
}

DerivedObject::DerivedObject(int id) : Object(id) {}

int DerivedObject::VirtualFunction(bool& called, bool&& value, bool* derived) {
  called = std::move(value);
  *derived = true;
  return id_;
}

OverloadedCallable::OverloadedCallable(int id) : id_(id) {}

OverloadedCallable::OverloadedCallable(const OverloadedCallable& callable)
    : id_(callable.id_) {}

OverloadedCallable::OverloadedCallable(
    const volatile OverloadedCallable& callable)
    : id_(callable.id_) {}

int OverloadedCallable::operator ()(CVQualification& cv) {
  cv = CVQualification::kNonQualified;
  return id_;
}

int OverloadedCallable::operator ()(CVQualification& cv) const {
  cv = CVQualification::kConstQualified;
  return id_;
}

int OverloadedCallable::operator ()(CVQualification& cv) volatile {
  cv = CVQualification::kVolatileQualified;
  return id_;
}

int OverloadedCallable::operator ()(CVQualification& cv) const volatile {
  cv = CVQualification::kConstVolatileQualified;
  return id_;
}

}  // namespace test
}  // namespace mf
