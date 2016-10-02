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

#include "generic_event_queue.h"

GenericEventQueue::GenericEventQueue()
    : last_id_(0),
      current_dispatch_event_(nullptr),
      listeners_removed_during_dispatch_(false) {}

GenericEventQueue::ListenerId GenericEventQueue::AddEventListener(
    void* event,
    mf::TypeErasedFunction&& listener) {
  if (!event || !listener)
    return 0;

  // Add a listener list for this event if there isn't one already.
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  ListenerId id = ++last_id_;
  ListenerList& listener_list = listener_map_[event];
  listener_list.emplace_back(std::make_pair(id, std::move(listener)));
  return id;
}

bool GenericEventQueue::RemoveEventListener(void* event, ListenerId id) {
  if (!event || id == 0)
    return false;

  std::lock_guard<std::recursive_mutex> lock(mutex_);
  auto event_it = listener_map_.find(event);
  if (event_it == listener_map_.end())
    return false;

  ListenerList& listener_list = event_it->second;
  auto it = std::find_if(
      listener_list.begin(), listener_list.end(),
      [id](const Listener& listener) {
        return listener.first == id;
      });

  if (it == listener_list.end())
    return false;

  // Check if this is being removed from a dispatch for the same event.
  if (current_dispatch_event_ == event) {
    // If so, just mark the id as null. It will be deleted later.
    it->first = 0;
    listeners_removed_during_dispatch_ = true;
  } else {
    // Otherwise just delete the listener.
    listener_list.erase(it);
    if (listener_list.empty())
      listener_map_.erase(event_it);
  }

  return true;
}

size_t GenericEventQueue::CountListeners(void* event) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  auto it = listener_map_.find(event);
  return it == listener_map_.end() ? 0 : it->second.size();
}

bool GenericEventQueue::Dispatch() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  // Abort dispatch calls if we already are within a dispatch.
  if (current_dispatch_event_ != nullptr)
    return false;

  // Process any enqueued events. Note that events added during a dispatch are
  // stored in a separate list and added back at the end.
  for (auto event_it = event_queue_.begin(); event_it != event_queue_.end();) {
    auto listener_list_it = listener_map_.find(event_it->first);
    if (listener_list_it == listener_map_.end()) {
      ++event_it;
      continue;
    }

    // Reset the information for the current event dispatch.
    // Used to handle listeners removed during dispatched events.
    current_dispatch_event_ = event_it->first;
    listeners_removed_during_dispatch_ = false;

    // Process only to the last listener available when starting to dispatch
    // the current event.
    auto& listener_list = listener_list_it->second;
    auto last_listener = listener_list.end();
    for (auto listener_it = listener_list.begin();
         listener_it != last_listener; ++listener_it) {
      event_it->second(listener_it->second);
    }

    // Clean up any listeners with null ids.
    // These were removed during dispatch of events of the current type.
    if (listeners_removed_during_dispatch_) {
      listeners_removed_during_dispatch_ = false;
      listener_list.remove_if([](const Listener& listener) {
        return listener.first == 0;
      });
    }

    // Advance to the next event.
    ++event_it;
    event_queue_.pop_front();

    // The end iterator of a deque might be invalidated after removing the last
    // element on it. This is required to fix a debug error in MSVC.
	if (event_queue_.empty())
      break;
  }

  // Move any events enqueued during dispatch to the event queue.
  for (auto& event : events_enqueued_during_dispatch_)
    event_queue_.emplace_back(std::move(event));
  events_enqueued_during_dispatch_.clear();

  current_dispatch_event_ = nullptr;
  return true;
}

