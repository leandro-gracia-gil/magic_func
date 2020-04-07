#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

#include "generic_event_queue.h"

// Class defining event functions.
//
// Functions here do not need to be implemented, extended or anything else.
// Rather, they are simply a function name to refer to with its type signature.
// The fact these are part of a class is purely for organizative purposes.
class FooEvents {
 public:
  static void OnFoo(const std::string& str) {}
  static void OnBar(int x, int y) {}
  static void LvalueReferenceExample(int& value) {}

  // C++14 support is required for non-copyable and rvalue reference arguments.
  static void NonCopyableExample(std::unique_ptr<int> x) {}
  static void RvalueReferenceExample(std::unique_ptr<int>&& x) {}
};

int main() {
  GenericEventQueue event_queue;

  // Add an event listener for OnFoo events.
  event_queue.AddEventListener(
      FooEvents::OnFoo,
      [](const std::string& str) {
        std::cout << "OnFoo called: " << str << std::endl;
      });

  // Add an event listener for OnBar events.
  event_queue.AddEventListener(
      FooEvents::OnBar,
      [](int x, int y) {
        std::cout << "OnBar called: " << x << ", " << y << std::endl;
      });

  // Add an event listener that receives an lvalue reference.
  event_queue.AddEventListener(
      FooEvents::LvalueReferenceExample,
      [](int& value) {
        value = 7;
        std::cout << "Setting reference value to " << value << std::endl;
      });

  // This does nothing because there are no enqueued events.
  event_queue.Dispatch();

  // So, let's enqueue some.
  event_queue.Enqueue(FooEvents::OnFoo, "this goes to the event listener");
  event_queue.Enqueue(FooEvents::OnBar, 2, 3);

  // With C++11 features we can also pass lvalue references as events.
  // For that we must make sure to use std::ref() or the reference will decay.
  int value = 5;
  event_queue.Enqueue(FooEvents::LvalueReferenceExample, std::ref(value));
  assert(value == 5);

  // Now this synchronously calls any listeners with the provided arguments.
  event_queue.Dispatch();

  // The value we passed should have been updated by the dispatch.
  std::cout << "Value is now " << value << " after dispatch." << std::endl;
  assert(value == 7);

#if __cplusplus >= 201402L || (defined(_MSC_VER) && _MSC_VER >= 1900)
  // There are a few more things we can do if we have C++14 support.
  // These are possible because of C++14 generalized lambda captures, which
  // allow moving objects into lambdas instead of only copying them.
  std::cout << "--------------------------" << std::endl;
  std::cout << "|      C++14 extras      |" << std::endl;
  std::cout << "--------------------------" << std::endl;

  // Add an event listener that receives a non-copyable object.
  event_queue.AddEventListener(
      FooEvents::NonCopyableExample,
      [](std::unique_ptr<int> x) {
        std::cout << "NonCopyable called: " << *x << std::endl;
      });

  // Move a unique_ptr into the event queue.
  // The event queue will detect that std::unique_ptr<int> is not copyable
  // and will automatically move it when calling the first registered listener.
  std::unique_ptr<int> p1(new int(16));
  event_queue.Enqueue(FooEvents::NonCopyableExample, std::move(p1));
  assert(p1 == nullptr);

  // Add an event listener that receives an rvalue reference.
  // The object is not moved unless this listener explicitly does so.
  event_queue.AddEventListener(
      FooEvents::RvalueReferenceExample,
      [](std::unique_ptr<int>&& x) {
        // This is where moving actually happens.
        // If we don't move here the second listener will get a valid object.
        assert(x != nullptr);
        std::unique_ptr<int> value = std::move(x);
        assert(value != nullptr);
        assert(x == nullptr);

        std::cout << "RvalueReferenceExample called: " << *value << std::endl;
      });

  // Add a second event listener that receives an rvalue reference.
  // In this case, since the object was moved by the first listener we will
  // receive an empty object.
  event_queue.AddEventListener(
      FooEvents::RvalueReferenceExample,
      [](std::unique_ptr<int>&& x) {
        assert(x == nullptr);
        std::cout << "RvalueReferenceExample called: empty object" << std::endl;
      });

  // Move another unique_ptr into the event queue.
  //
  // The unique pointer will be moved only when a listener explicitly takes
  // the rvalue reference into an object. In this example, this happens inside
  // the body of the first listener, but if commented out the second listener
  // will still receive a valid object.
  std::unique_ptr<int> p2(new int(32));
  event_queue.Enqueue(FooEvents::RvalueReferenceExample, std::move(p2));
  assert(p2 == nullptr);

  // Dispatch the enqueued events.
  event_queue.Dispatch();
#endif  // __cplusplus >= 201402L || (defined(_MSC_VER) && _MSC_VER >= 1900)

  return 0;
}
