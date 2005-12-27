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

#include "taskrunner.h"
#include "task.h"
#include <algorithm>


namespace buzz {

TaskRunner::~TaskRunner() {
  // this kills and deletes children silently!
  AbortAllChildren();
  RunTasks();
}

void
TaskRunner::StartTask(Task * task) {
  tasks_.push_back(task);
  WakeTasks();
}

void
TaskRunner::RunTasks() {
  // Running continues until all tasks are Blocked (ok for a small # of tasks)
  if (tasks_running_) {
    return; // don't reenter
  }

  tasks_running_ = true;

  int did_run = true;
  while (did_run) {
    did_run = false;
    // use indexing instead of iterators because tasks_ may grow
    for (size_t i = 0; i < tasks_.size(); ++i) {
      while (!tasks_[i]->Blocked()) {
        tasks_[i]->Step();
        did_run = true;
      }
    }
  }
  // Tasks are deleted when running has paused
  for (size_t i = 0; i < tasks_.size(); ++i) {
    if (tasks_[i]->IsDone()) {
      Task* task = tasks_[i];
      delete task;
      tasks_[i] = NULL;
    }
  }
  // Finally, remove nulls
  tasks_.erase(std::remove(tasks_.begin(), tasks_.end(), (Task *)NULL), tasks_.end());

  tasks_running_ = false;
}

void
TaskRunner::PollTasks() {
  // every task gets hit once with a poll - they wake if needed
  for (size_t i = 0; i < tasks_.size(); ++i) {
    if (!tasks_[i]->IsDone()) {
      tasks_[i]->Poll();
    }
  }
}


}
