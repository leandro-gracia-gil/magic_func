# Examples
## Basic
For a description of MagicFunc basic usage, check the README file in the parent folder.

## Advanced
### Generic event queue
Taking advantage of MagicFunc's function type erasure features, this advanced example shows how to create an event queue object that works for arbitrary functions of different types.

For example, you define events like this.
```c++
struct KeyboardEvent {
  static void OnKeyDown(int key) {}
  static void OnKeyUp(int key) {}
};
```
These are not actual functions you call, but rather represent the interface of an event you can listen to.

Then, if you want to listen for an event you do:
```c++
GenericEventQueue event_queue;

event_queue.AddEventListener(
    KeyboardEvent::OnKeyDown,
    [](int key) {
      // Do something.
    });
```

Any valid mf::Function can be used as an event listener, including for example member functions of specific objects.

With the code above the listener will be called when KeyboardEvent::OnKeyDown events are dispatched, but for that we need actual events to happen. This is done in two steps: enqueing and, when appropriate, dispatching.

Enqueuing is very simple. We just pass which event we are enqueuing followed by the arguments we would pass when calling the event normally.
```c++
event_queue.Enqueue(KeyboardEvent::OnKeyDown, 0x20);
```

By default all arguments are copied into the event queue until the event is later dispatched. You can also pass lvalue references (like int&) by using std::ref() when passing the argument to Enqueue.

If C++14 is supported (you might need to use -std=c++14 in clang and gcc) you can also move non-copyable objects and pass rvalue references (like std::unique_ptr<T>&&). For more details check the generic event queue header.

Finally, once you are ready to dispatch all enqueued events just run:
```c++
event_queue.Dispatch();
```

This will synchronously call all registered listeners for any events that were enqueued in the order they were.

This example class is thread-safe and handles reentrant events to avoid dispatch calls that could cause infinite loops. All these features are unit tested.

#### &#x1F534; **IMPORTANT NOTE** &#x1F534;
When using MagicFunc in a Release build in MSVC, make sure to disable COMDAT folding (Linker -> Optimization) or pass the [/OPT:NOICF](https://msdn.microsoft.com/en-us/library/bxwfs976(v=vs.140).aspx) linker argument. Not doing so will lead to different events having the same function address, which can cause assertion failures in the generic event queue.
