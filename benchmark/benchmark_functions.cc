#include <cstdlib>

void FreeFunction(size_t& value) { ++value; }

struct Object {
  Object() : value(0) {}

  void Function(int delta);
  size_t value;
};

void Object::Function(int delta) {
  value += delta;
}
