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

#ifndef MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_GENERIC_EVENT_QUEUE_H_
#define MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_GENERIC_EVENT_QUEUE_H_

#include <algorithm>
#include <deque>
#include <list>
#include <mutex>
#include <unordered_map>
#include <utility>

#include <magic_func/function.h>
#include <magic_func/function_cast.h>
#include <magic_func/function_traits.h>

#include "cpp14_helpers.h"
#include "event_tuple_extractor.h"
#include "selective_decay.h"

// A versatile, simple to use, thread-safe general purpose event queue with
// support for broadcast and observer patterns.
//
// This event queue allows adding and removing listeners for event functions
// where events can be any regular or static function with linkage.
//
// @code
// // Let's define some event functions we will listen to and dispatch.
// // We can use any regular or static member function. For example, here we
// // organize two individual keyboard events into a KeyboardEvent class.
// class KeyboardEvent {
//  public:
//   // These functions are never actually called, so they can be empty.
//   // We just use their address and type signature to refer to them.
//   // We cannot use non-static member functions because they cannot be
//   // casted to a unique address that identifies them.
//   static void OnKeyDown(int code) {}
//   static void OnKeyUp(int code) {}
// };
//
// int main() {
//   GenericEventQueue event_queue;
//
//   // We enqueue events for each particular function.
//   event_queue.Enqueue(KeyboardEvent::OnKeyDown, 0x20);
//   event_queue.Enqueue(KeyboardEvent::OnKeyUp, 0x20);
//
//   // We add listeners for the event functions we care about.
//   // Anything convertible to a mf::Function can be used as a listener.
//   // For example, we could also use a member function bound to an object.
//   event_queue.AddEventListener(
//     KeyboardEvent::OnKeyDown,
//     [](int code) {
//       if (code == 0x20)
//         DoSomething();
//     });
//
//   // We dispatch any enqueued events to any listeners registered for them at
//   // the time of the call. We could call this, for example, each frame in
//   // a main loop. This synchronously calls our lambda listener above.
//   event_queue.Dispatch();
//   return 0;
// }
// @endcode
class GenericEventQueue {
 public:
  // Type used to identify registered event listener.
  using ListenerId = int;

  // Auxiliary alias to get the underlying function type corresponding to a
  // function pointer or member function pointer.
  template <typename FuncPtr>
  using FunctionType = typename mf::FunctionTraits<FuncPtr>::FunctionType;

  GenericEventQueue();
  GenericEventQueue(const GenericEventQueue&) = delete;
  GenericEventQueue(GenericEventQueue&&) = delete;

  GenericEventQueue& operator =(const GenericEventQueue&) = delete;
  GenericEventQueue& operator =(GenericEventQueue&&) = delete;

  // Adds a listener for an event function.
  //
  // @param event The event function to listener to.
  // @param listener The function to call when an event for the provided
  //                 function is dispatched.
  // @return A unique id for the listener that can be used to remove it,
  //         or zero in case of invalid arguments.
  template <typename FuncPtr>
  ListenerId AddEventListener(FuncPtr event,
                              mf::Function<FunctionType<FuncPtr>> listener) {
    return AddEventListener(reinterpret_cast<void*>(event),
                            std::move(listener));
  }

  // Removes a previously added listener for an event function.
  //
  // @param event The event function to remove the listener from.
  // @param id The id returned by AddEventListener.
  // @return true if removed, false if not found or invalid arguments.
  template <typename FuncPtr>
  bool RemoveEventListener(FuncPtr event, ListenerId id) {
    return RemoveEventListener(reinterpret_cast<void*>(event), id);
  }

  // Returns the number of listeners registered for an event function.
  //
  // @param event The event function to count its listeners.
  // @return The number of listeners currently registered for the function.
  template <typename FuncPtr>
  size_t CountListeners(FuncPtr event) {
    return CountListeners(reinterpret_cast<void*>(event));
  };

  // Enqueues an event to be dispatched later.
  //
  // Events are dispatched in the same order they were enqueued. Any provided
  // arguments will be converted to the appropriate function argument types and
  // stored within the event queue until dispatch.
  //
  // Unless otherwise specified arguments will be copied by default. In order
  // to pass lvalue references (e.g. int&) std::ref() must be explicitly used.
  //
  // In C++11 all provided arguments must either be copy constructible or
  // reference wrappers (use of std::ref()). If C++14 support is enabled, it is
  // also possible to move objects that are not copy-constructible and to pass
  // rvalue reference types (e.g., int&&). These will be moved instead of copied
  // when dispatching the event.
  //
  // This method is thread-safe, but it can block if another thread is calling
  // Dispatch(). It should be possible to implement lock-free event queues that
  // allow this method to not block during Dispatch calls, but that is out of
  // the scope of this example.
  //
  // @param event The event function to enqueue for.
  // @param args Any arguments to pass to the event function.
  template <typename FuncPtr, typename... Args_>
  void Enqueue(FuncPtr event, Args_&&... args) {
    // Apply the selective decay operation described above depending on whether
    // std::ref was used or not and store the result in a tuple. In particular,
    // this does two things:
    //
    // 1. Enforces the conversion of any arguments into the types expected
    //    by the function, applying decay to store them.
    //
    //    For example, passing a char* to a function expecting a const
    //    std::string& will create and store a std::string object when enqueuing
    //    rather than copying the pointer and creating the string when the
    //    event is dispatched. This ensures the received string contents will
    //    match the ones at enqueue time.
    //
    // 2. Any argument passed using std::ref() will be also converted but not
    //    decayed. This is done to allow explicit references to behave as such,
    //    where the caller is responsible to ensure their validity between
    //    enqueuing and dispatching.
    //
    //    For example, in order to pass a Foo object named foo to a function
    //    expecting a Foo& argument, std::ref(foo) must be used or foo will be
    //    copied instead. If foo is destroyed before the event is dispatched
    //    behavior is undefined.
    //
    using ArgsTuple = typename mf::FunctionTraits<FuncPtr>::Args;
    static_assert(sizeof...(Args_) == std::tuple_size<ArgsTuple>::value,
                  "Invalid number of arguments for function");

    using DecayedTuple = SelectiveDecay<std::tuple<Args_...>, ArgsTuple>;
    DecayedTuple args_tuple(std::forward<Args_>(args)...);

    // Enqueue a lambda that undoes type erasure and invokes the function.
    // In C++11 the arguments tuple is copied. In C++14 is moved, so it is
    // also possible to store move-only arguments and rvalue references.
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto event_pair = std::make_pair(
        reinterpret_cast<void*>(event),
#if __cplusplus < 201402L && (!defined(_MSC_VER) || _MSC_VER < 1900)
        // In C++11 we copy the argument tuple after conversion.
        [args_tuple]
#else
        // In C++14 we use generalize lambda captures to move the tuple.
        // This allows passing non-copyable objects and rvalue references.
        [args_tuple = std::move(args_tuple)]
#endif
        (mf::TypeErasedFunction& type_erased) mutable {
          // Undo the type erasure and invoke the function with our tuple.
          // This will raise a MagicFunc fatal runtime error if the function
          // type does not match, which should never be the case.
          auto& f = mf::function_cast<FunctionType<FuncPtr>>(type_erased);
          Invoke(f, args_tuple, std::index_sequence_for<Args_...>());
        });

    // Inserting the event into the queue during a dispatch invalidates
    // all iterators. In that case we add them when dispatch finishes.
    if (current_dispatch_event_)
      events_enqueued_during_dispatch_.emplace_back(std::move(event_pair));
    else
      event_queue_.emplace_back(std::move(event_pair));
  }

  // Dispatches any enqueued events to their corresponding listeners.
  //
  // Events are dispatched synchronously from the thread performing the call,
  // and in the same relative order they were enqueued. For each event,
  // listeners are called in the same order they were registered.
  //
  // By default arguments will be copied when invoking listeners unless one
  // of the following happens:
  // - The argument is a lvalue reference (e.g. int&) and std::ref() was used
  //   when enqueing. In this case the lvalue reference is passed.
  //
  // - The argument is an rvalue reference (e.g. int&&) and was moved during
  //   enqueue. In this case an rvalue reference to the object stored during
  //   enqueue is passed. This means that a listener can move the object if
  //   desired, which will cause subsequent listeners to receive an already
  //   moved and potentially invalid object. Use with care.
  //
  // - The argument is not copy-constructible and was moved during enqueue.
  //   In this case the object will be moved to the listener, meaning that only
  //   the first listener for the event will receive the original object.
  //   Should not be used for broadcasting events. You might want to assert
  //   that the number of listeners for these events is not greater than 1.
  //
  // Dispatching is also reentrant-safe. Any events enqueued during a dispatch
  // will run the next time Dispatch() is called. Adding and removing listeners 
  // during an event dispatch becomes effective after all listeners have been
  // called for that event (not the entire dispatch).
  //
  // @return false if a dispatch is already going on on the calling thread,
  //         in which case the call will be ignored. Returns true otherwise.
  bool Dispatch();

 private:
  using Event = std::pair<void*, mf::Function<void(mf::TypeErasedFunction&)>>;
  using Listener = std::pair<ListenerId, mf::TypeErasedFunction>;
  using ListenerList = std::list<Listener>;

  // Invokes a functor with the arguments contained in a provided tuple.
  // For details on how the arguments are passed to the functor, see Dispatch.
  template <typename F, typename... Args, size_t... Indices>
  static std::result_of_t<F(Args...)>
  Invoke(F& f, std::tuple<Args...>& args, std::index_sequence<Indices...>) {
    return f(ExpandEventArgs<Indices>(args)...);
  }

  // Non-template functions for listener management.
  ListenerId AddEventListener(void* event, mf::TypeErasedFunction&& listener);
  bool RemoveEventListener(void* event, ListenerId id);
  size_t CountListeners(void* event);

  // This intentionally avoids using an unordered multimap because we want
  // an order relation between the multiple entries of a same key.
  std::unordered_map<void*, ListenerList> listener_map_;
  std::deque<Event> event_queue_;
  std::recursive_mutex mutex_;
  ListenerId last_id_;

  // Used to avoid reentrant code issues during dispatch.
  void* current_dispatch_event_;
  bool listeners_removed_during_dispatch_;
  std::list<Event> events_enqueued_during_dispatch_;
};

#endif  // MAGIC_FUNC_EXAMPLES_GENERIC_EVENT_QUEUE_GENERIC_EVENT_QUEUE_H_
