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

#include <atomic>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "generic_event_queue.h"

struct Events {
  static void NoArgs() {}
  static void WithArgs(int x, const std::string& str) {}
  static void LvalueRef(int& x) {}

  // C++14 support is required for non-copyable and rvalue reference arguments.
  static void NonCopyable(std::unique_ptr<int> x) {}
  static void RvalueRef(std::unique_ptr<int>&& x) {}
};

TEST(GenericEventQueue, DispatchEventNeedsEnqueue) {
  GenericEventQueue event_queue;
  bool called = false;

  event_queue.AddEventListener(
      &Events::NoArgs,
      [&called]() {
        called = true;
      });

  EXPECT_FALSE(called);
  EXPECT_TRUE(event_queue.Dispatch());

  // No event is enqueued despite being a listener.
  EXPECT_FALSE(called);
}

TEST(GenericEventQueue, DispatchEventNoArgs) {
  GenericEventQueue event_queue;
  bool called = false;

  event_queue.AddEventListener(
      &Events::NoArgs,
      [&called]() {
        called = true;
      });

  event_queue.Enqueue(&Events::NoArgs);

  EXPECT_FALSE(called);
  EXPECT_TRUE(event_queue.Dispatch());
  EXPECT_TRUE(called);
}

TEST(GenericEventQueue, DispatchEventWithArgs) {
  GenericEventQueue event_queue;
  bool called = false;

  event_queue.AddEventListener(
      &Events::WithArgs,
      [&called](int x, const std::string& str) {
        EXPECT_EQ(23, x);
        EXPECT_EQ("testing", str);
        called = true;
      });

  event_queue.Enqueue(&Events::WithArgs, 23, "testing");

  EXPECT_FALSE(called);
  EXPECT_TRUE(event_queue.Dispatch());
  EXPECT_TRUE(called);
}

TEST(GenericEventQueue, DispatchEventLvalueReference) {
  GenericEventQueue event_queue;
  bool called = false;

  event_queue.AddEventListener(
      &Events::LvalueRef,
      [&called](int& x) {
        EXPECT_EQ(23, x);
        x = 42;
        called = true;
      });

  int x = 23;
  event_queue.Enqueue(&Events::LvalueRef, std::ref(x));

  EXPECT_FALSE(called);
  EXPECT_EQ(23, x);
  EXPECT_TRUE(event_queue.Dispatch());

  EXPECT_TRUE(called);
  EXPECT_EQ(42, x);
}

#if __cplusplus >= 201402L
TEST(GenericEventQueue, DispatchEventNonCopyable) {
  GenericEventQueue event_queue;
  std::vector<int> called;

  event_queue.AddEventListener(
      &Events::NonCopyable,
      [&called](std::unique_ptr<int> x) {
        EXPECT_EQ(23, *x);
        called.push_back(0);
      });

  event_queue.AddEventListener(
      &Events::NonCopyable,
      [&called](std::unique_ptr<int> x) {
        // The object was moved in the previous event listener.
        EXPECT_EQ(nullptr, x);
        called.push_back(1);
      });

  std::unique_ptr<int> ptr(new int(23));
  event_queue.Enqueue(&Events::NonCopyable, std::move(ptr));

  EXPECT_EQ(0U, called.size());
  EXPECT_TRUE(event_queue.Dispatch());

  ASSERT_EQ(2U, called.size());
  for (size_t i = 0; i < called.size(); ++i)
    EXPECT_EQ(i, called[i]);
}

TEST(GenericEventQueue, DispatchEventRvalueReference) {
  GenericEventQueue event_queue;
  std::vector<int> called;

  event_queue.AddEventListener(
      &Events::RvalueRef,
      [&called](std::unique_ptr<int>&& x) {
        // Do not move the rvalue reference in this listener.
        EXPECT_NE(nullptr, x);
        called.push_back(0);
      });

  event_queue.AddEventListener(
      &Events::RvalueRef,
      [&called](std::unique_ptr<int>&& x) {
        // The object should not have been moved yet.
        EXPECT_NE(nullptr, x);
        std::unique_ptr<int> value = std::move(x);
        EXPECT_EQ(23, *value);
        called.push_back(1);
      });

  event_queue.AddEventListener(
      &Events::RvalueRef,
      [&called](std::unique_ptr<int>&& x) {
        // The object should have been already moved.
        EXPECT_EQ(nullptr, x);
        called.push_back(2);
      });

  std::unique_ptr<int> ptr(new int(23));
  event_queue.Enqueue(&Events::RvalueRef, std::move(ptr));

  EXPECT_EQ(0U, called.size());
  EXPECT_TRUE(event_queue.Dispatch());

  ASSERT_EQ(3U, called.size());
  for (size_t i = 0; i < called.size(); ++i)
    EXPECT_EQ(i, called[i]);
}
#endif  // __cplusplus >= 201402L

TEST(GenericEventQueue, DispatchEventMultipleListeners) {
  GenericEventQueue event_queue;
  std::vector<int> called;

  // This will be the first listener to be called on NoArgs events.
  event_queue.AddEventListener(
      &Events::NoArgs,
      [&called]() {
        called.push_back(2);
      });

  // This will be the first listener to be called on WithArgs events.
  event_queue.AddEventListener(
      &Events::WithArgs,
      [&called](int x, const std::string& str) {
        EXPECT_EQ(123, x);
        EXPECT_EQ("foo", str);
        called.push_back(0);
      });

  // This will be the second listener to be called on NoArgs events.
  event_queue.AddEventListener(
      &Events::NoArgs,
      [&called]() {
        called.push_back(3);
      });

  // This will be the second listener to be called on WithArgs events.
  event_queue.AddEventListener(
      &Events::WithArgs,
      [&called](int x, const std::string& str) {
        EXPECT_EQ(123, x);
        EXPECT_EQ("foo", str);
        called.push_back(1);
      });

  event_queue.Enqueue(&Events::WithArgs, 123, "foo");
  event_queue.Enqueue(&Events::NoArgs);

  EXPECT_EQ(0U, called.size());
  EXPECT_TRUE(event_queue.Dispatch());

  ASSERT_EQ(4U, called.size());
  for (size_t i = 0; i < called.size(); ++i)
    EXPECT_EQ(i, called[i]);
}

TEST(GenericEventQueue, RemoveListeners) {
  GenericEventQueue event_queue;
  std::vector<int> called;
  int id[3];

  id[0] = event_queue.AddEventListener(
      &Events::NoArgs,
      [&called]() {
        called.push_back(0);
      });

  id[1] = event_queue.AddEventListener(
      &Events::NoArgs,
      [&called]() {
        called.push_back(-1);
      });

  id[2] = event_queue.AddEventListener(
      &Events::NoArgs,
      [&called]() {
        called.push_back(1);
      });

  event_queue.Enqueue(&Events::NoArgs);
  event_queue.RemoveEventListener(&Events::NoArgs, id[1]);

  EXPECT_EQ(0U, called.size());
  EXPECT_TRUE(event_queue.Dispatch());

  ASSERT_EQ(2U, called.size());
  for (size_t i = 0; i < called.size(); ++i)
    EXPECT_EQ(i, called[i]);

  event_queue.RemoveEventListener(&Events::NoArgs, id[0]);
  event_queue.RemoveEventListener(&Events::NoArgs, id[2]);
  called.clear();

  event_queue.Enqueue(&Events::NoArgs);
  EXPECT_TRUE(event_queue.Dispatch());
  EXPECT_EQ(0U, called.size());
}

TEST(GenericEventQueue, EnqueueEventsDuringDispatch) {
  GenericEventQueue event_queue;
  std::vector<int> called;

  event_queue.AddEventListener(
      &Events::NoArgs,
      [&called, &event_queue]() {
        called.push_back(0);
        event_queue.Enqueue(&Events::WithArgs, 123, "foo");
      });

  event_queue.AddEventListener(
      &Events::WithArgs,
      [&called, &event_queue](int x, const std::string& str) {
        EXPECT_EQ(123, x);
        EXPECT_EQ(str, "foo");
        called.push_back(1);
      });

  event_queue.Enqueue(&Events::NoArgs);

  EXPECT_EQ(0U, called.size());
  EXPECT_TRUE(event_queue.Dispatch());

  // Only the first event listener should be called in the dispatch.
  ASSERT_EQ(1U, called.size());
  EXPECT_EQ(0, called[0]);

  // The second event listener should be called after this dispatch.
  EXPECT_TRUE(event_queue.Dispatch());
  ASSERT_EQ(2U, called.size());
  for (size_t i = 0; i < called.size(); ++i)
    EXPECT_EQ(i, called[i]);
}

TEST(GenericEventQueue, RemoveListenersDuringDispatch) {
  GenericEventQueue event_queue;
  std::vector<int> called;
  int id;

  event_queue.AddEventListener(
      &Events::NoArgs,
      [&called, &event_queue, &id]() {
        called.push_back(0);
        event_queue.RemoveEventListener(&Events::NoArgs, id);
      });

  id = event_queue.AddEventListener(
      &Events::NoArgs,
      [&called, &event_queue]() {
        called.push_back(1);
      });

  // Enqueue two consecutive NoArgs events.
  event_queue.Enqueue(&Events::NoArgs);
  event_queue.Enqueue(&Events::NoArgs);

  EXPECT_EQ(0U, called.size());
  EXPECT_TRUE(event_queue.Dispatch());

  // During the first event both listeners should have been called.
  // This is because the listener removal is not effective until all listeners
  // for that event have been called. However, for the second event only
  // the first listener should be called.
  ASSERT_EQ(3U, called.size());
  EXPECT_EQ(0, called[0]);
  EXPECT_EQ(1, called[1]);
  EXPECT_EQ(0, called[2]);
}

TEST(GenericEventQueue, DispatchWithinDispatch) {
  GenericEventQueue event_queue;
  std::vector<int> called;

  event_queue.AddEventListener(
      &Events::NoArgs,
      [&called, &event_queue]() {
        called.push_back(0);
        EXPECT_FALSE(event_queue.Dispatch());
      });

  event_queue.AddEventListener(
      &Events::NoArgs,
      [&called]() {
        called.push_back(1);
      });

  event_queue.Enqueue(&Events::NoArgs);

  EXPECT_EQ(0U, called.size());
  EXPECT_TRUE(event_queue.Dispatch());

  ASSERT_EQ(2U, called.size());
  for (size_t i = 0; i < called.size(); ++i)
    EXPECT_EQ(i, called[i]);
}

TEST(GenericEventQueue, MultithreadedUse) {
  GenericEventQueue event_queue;

  static constexpr uint64_t N = 5;    // Number of threads to spawn.
  static constexpr uint64_t E = 100;  // Number of events per thread.
  std::thread t[N];

  size_t called[N];
  std::atomic<uint64_t> sum(0);
  std::fill(called, called + N, 0);

  srand(static_cast<unsigned int>(time(nullptr)));

  // Spawn N threads. Each thread will:
  // 1. Spawn a listener that takes a ref and a value and adds it.
  // 2. Enqueue E events that pass a range of odd numbers to add.
  //
  // The result should be that sum will contain N times the sum of the first
  // N * E odd numbers, which is equivalent to N * (N * E)^2.
  for (size_t num_thread = 0; num_thread < N; ++num_thread) {
    t[num_thread] = std::thread([num_thread, &event_queue, &called, &sum]() {
      event_queue.AddEventListener(
          &Events::WithArgs,
          [num_thread, &called, &sum](size_t value, const std::string& str) {
            sum += value;
            ++called[num_thread];
          });

      // Yield until all threads have registered their event listeners.
      // This ensures dispatches from other threads are received too.
      while (event_queue.CountListeners(&Events::WithArgs) < N)
        std::this_thread::yield();

      size_t start = E * num_thread;
      size_t end = E * (num_thread + 1);
      for (size_t i = start; i < end; ++i)
        event_queue.Enqueue(&Events::WithArgs, 2 * i + 1, "");

      // Sleep a random amount of time to make the multithreading behavior
      // less predictable.
      std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 100));

      // Dispatch the events. There might be events already enqueued by other
      // threads, which will be dispatched too.
      EXPECT_TRUE(event_queue.Dispatch());
    });
  }

  // Wait for all 3 threads to finish.
  for (size_t num_thread = 0; num_thread < N; ++num_thread)
    t[num_thread].join();

  // Each thread should have received E events from each of the N threads.
  for (size_t i = 0; i < N; ++i)
    EXPECT_EQ(N * E, called[i]);

  // The sum of the first x numbers is equal to x^2.
  // Similarly, the different threads have added the N * E first odd numbers,
  // which is equal to (N * E)^2. They have done so by making each thread
  // enqueue sums for different parts of the range. However, since each thread
  // has its own listener adding numbers this operation has been repeated N
  // times, leading to N * (N * E)^2.
  EXPECT_EQ(N * (N * E) * (N * E), sum);
}
