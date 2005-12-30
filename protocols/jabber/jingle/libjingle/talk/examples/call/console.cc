/*
 * Jingle call example
 * Copyright 2004--2005, Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

extern "C" {
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
}
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "talk/examples/call/console.h"

namespace {

void PError(const char* desc) {
  perror(desc);
  exit(1);
}

CConsole* gConsole = NULL;

const uint32 MSG_UPDATE = 1;

} // namespace

void InitConsole(cricket::PhysicalSocketServer* ss) {
  assert(gConsole == NULL);
  assert(ss);
  gConsole = new CConsole(ss);
}

CConsole* Console() {
  assert(gConsole);
  return gConsole;
}

CConsole::CConsole(cricket::PhysicalSocketServer* ss)
    : prompting_(false), prompt_dirty_(false) {
  stdin_ = ss->CreateFile(0);
  stdin_->SignalReadEvent.connect(this, &CConsole::OnReadInput);

  tasks_ = new std::vector<ConsoleTask*>;
}

CConsole::~CConsole() {
  delete stdin_;
  delete tasks_;
}

void CConsole::Push(ConsoleTask* task) {
  task->set_console(this);
  task->SignalDone.connect(this, &CConsole::OnTaskDone);
  tasks_->push_back(task);
  task->Start();
  UpdatePrompt();
}

void CConsole::Remove(ConsoleTask* task) {
  int index = -1;
  for (size_t i = 0; i < tasks_->size(); ++i) {
    if ((*tasks_)[i] == task)
      index = i;
  }

  assert(index >= 0);
  tasks_->erase(tasks_->begin() + index);
  if (static_cast<int>(tasks_->size()) == index)
    UpdatePrompt();

  delete task;

  if (tasks_->size() == 0)
    exit(0);
}

void CConsole::Print(const char* str) {
  if (prompting_)
    printf("\r");
  printf("%s\n", str);
  prompting_ = false;
  UpdatePrompt();
}

void CConsole::Print(const std::string& str) {
  Print(str.c_str());
}

void CConsole::Printf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);

  char buf[4096];
  int size = vsnprintf(buf, sizeof(buf), format, ap);
  assert(size >= 0);
  assert(size < static_cast<int>(sizeof(buf)));
  buf[size] = '\0';
  Print(buf);

  va_end(ap);
}

void CConsole::OnTaskDone(ConsoleTask* task) {
  Remove(task);
}

void CConsole::OnReadInput(cricket::AsyncFile* file) {
  assert(file == stdin_);

  char buf[4096];
  int size = read(0, buf, sizeof(buf));
  if (size < 0)
    PError("read");

  prompting_ = (buf[size-1] != '\n');

  int start = 0;
  for (int i = 0; i < size; ++i) {
    if (buf[i] == '\n') {
      std::string line = input_;
      line.append(buf + start, i + 1 - start);
      input_.clear();

      assert(tasks_->size() > 0);
      tasks_->back()->ProcessLine(line);

      start = i + 1;
    }
  }

  input_.append(buf + start, size - start);
}

void CConsole::OnMessage(cricket::Message* pmsg) {
  assert(pmsg->message_id == MSG_UPDATE);
  assert(tasks_->size() > 0);
  if (prompting_)
    printf("\n");
  printf("%s: %s", tasks_->back()->GetPrompt().c_str(), input_.c_str());
  fflush(stdout);
  prompting_ = true;
  prompt_dirty_ = false;
}

void CConsole::UpdatePrompt() {
  if (!prompt_dirty_) {
    prompt_dirty_ = true;
    cricket::Thread::Current()->Post(this, MSG_UPDATE);
  }
}

void ConsoleTask::ParseLine(const std::string& line,
                            std::vector<std::string>* words) {
  assert(line.size() > 0);
  assert(line[line.size() - 1] == '\n');

  int start = -1;
  int state = 0;
  for (int index = 0; index <= static_cast<int>(line.size()); ++index) {
    if (state == 0) {
      if (!isspace(line[index])) {
        start = index;
        state = 1;
      }
    } else {
      assert(state == 1);
      assert(start >= 0);
      if (isspace(line[index])) {
        std::string word(line, start, index - start);
        words->push_back(word);
        start = -1;
        state = 0;
      }
    }
  }
}
