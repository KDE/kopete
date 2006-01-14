/*
 * libjingle
 * Copyright 2004--2005, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products 
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "task.h"
#include "taskrunner.h"

#include <algorithm>

namespace buzz {

Task::Task(Task * parent) :
    state_(STATE_INIT),
    parent_(parent),
    blocked_(false),
    done_(false),
    aborted_(false),
    busy_(false),
    error_(false),
    child_error_(false),
    start_time_(0) {
  runner_ = ((parent == NULL) ? (TaskRunner *)this : parent->GetRunner());
  if (parent_ != NULL) {
    parent_->AddChild(this);
  }
}

unsigned long long
Task::CurrentTime() {
  return runner_->CurrentTime();
}

unsigned long long
Task::ElapsedTime() {
  return CurrentTime() - start_time_;
}

void
Task::Start() {
  if (state_ != STATE_INIT)
    return;
  GetRunner()->StartTask(this);
  start_time_ = CurrentTime();
}

void
Task::Step() {
  if (done_) {
#ifdef DEBUG
    // we do not know how !blocked_ happens when done_ - should be impossible.
    // But it causes problems, so in retail build, we force blocked_, and
    // under debug we assert.
    assert(blocked_);
#else
    blocked_ = true;
#endif
    return;
  }

  // Async Error() was called
  if (error_) {
    done_ = true;
    state_ = STATE_ERROR;
    blocked_ = true;
//   obsolete - an errored task is not considered done now
//   SignalDone();
    Stop();
    return;
  }

  busy_ = true;
  int new_state = Process(state_);
  busy_ = false;

  if (aborted_) {
    Abort(true); // no need to wake because we're awake
    return;
  }
  
  if (new_state == STATE_BLOCKED) {
    blocked_ = true;
  }
  else {
    state_ = new_state;
    blocked_ = false;
  }
  
  if (new_state == STATE_DONE) {
    done_ = true;
  }
  else if (new_state == STATE_ERROR) {
    done_ = true;
    error_ = true;
  }

  if (done_) {
//  obsolete - call this yourself
//    SignalDone();
    Stop();
    blocked_ = true;
  }
}

void
Task::Abort(bool nowake) {
  if (aborted_ || done_)
    return;
  aborted_ = true;
  if (!busy_) {
    done_ = true;
    blocked_ = true;
    error_ = true;
    Stop();
    if (!nowake)
      Wake(); // to self-delete
  }
}

void
Task::Wake() {
  if (done_)
    return;
  if (blocked_) {
    blocked_ = false;
    GetRunner()->WakeTasks();
  }
}

void
Task::Error() {
  if (error_ || done_)
    return;
  error_ = true;
  Wake();
}

std::string
Task::GetStateName(int state) const {
  static const std::string STR_BLOCKED("BLOCKED");
  static const std::string STR_INIT("INIT");
  static const std::string STR_START("START");
  static const std::string STR_DONE("DONE");
  static const std::string STR_ERROR("ERROR");
  static const std::string STR_RESPONSE("RESPONSE");
  static const std::string STR_HUH("??");
  switch (state) {
    case STATE_BLOCKED: return STR_BLOCKED;
    case STATE_INIT: return STR_INIT;
    case STATE_START: return STR_START;
    case STATE_DONE: return STR_DONE;
    case STATE_ERROR: return STR_ERROR;
    case STATE_RESPONSE: return STR_RESPONSE;
  }
  return STR_HUH;
}

int Task::Process(int state) {
  switch (state) {
    case STATE_INIT:
      return STATE_START;
    case STATE_START:
      return ProcessStart();
    case STATE_RESPONSE:
      return ProcessResponse();
    case STATE_DONE:
    case STATE_ERROR:
      return STATE_BLOCKED;
  }
  return STATE_ERROR;
}

void
Task::AddChild(Task * child) {
  children_.insert(child);
}

bool
Task::AllChildrenDone() {
  for (ChildSet::iterator it = children_.begin(); it != children_.end(); ++it) {
    if (!(*it)->IsDone())
      return false;
  }
  return true;
}

bool
Task::AnyChildError() {
  return child_error_;
}

void
Task::AbortAllChildren() {
  if (children_.size() > 0) {
    ChildSet copy = children_;
    for (ChildSet::iterator it = copy.begin(); it != copy.end(); ++it) {
      (*it)->Abort(true); // Note we do not wake
    }
  }
}

void
Task::Stop() {
  AbortAllChildren(); // No need to wake because we're either awake or in abort
  parent_->OnChildStopped(this);
}

void
Task::OnChildStopped(Task * child) {
  if (child->HasError())
    child_error_ = true;
  children_.erase(child);
}


}
