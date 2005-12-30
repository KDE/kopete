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

#ifndef CRICKET_EXAMPLES_CALL_CONSOLE_H__
#define CRICKET_EXAMPLES_CALL_CONSOLE_H__

#include <string>
#include <vector>

#include "talk/base/sigslot.h"
#include "talk/base/thread.h"
#include "talk/base/physicalsocketserver.h"

class CConsole;

class ConsoleTask {
public:
  ConsoleTask() : console_(NULL) {}
  virtual ~ConsoleTask() {}

  CConsole* console() const { return console_; } 
  void set_console(CConsole* console) { console_ = console; }

  virtual void Start() {}
  virtual std::string GetPrompt() = 0;
  virtual void ProcessLine(const std::string& line) = 0; // includes newline

  sigslot::signal1<ConsoleTask*> SignalDone;

protected:
  void ParseLine(const std::string& line, std::vector<std::string>* words);

private:
  CConsole* console_;
};

class CConsole: public cricket::MessageHandler, public sigslot::has_slots<> {
public:
  CConsole(cricket::PhysicalSocketServer* ss);
  ~CConsole();

  void Push(ConsoleTask* task);
  void Remove(ConsoleTask* task);

  // final newline should not be included
  void Print(const char* str);
  void Print(const std::string& str);
  void Printf(const char* format, ...);

private:
  cricket::AsyncFile* stdin_;
  std::vector<ConsoleTask*>* tasks_;
  std::string input_;
  bool prompting_;
  bool prompt_dirty_;

  void OnTaskDone(ConsoleTask* task);
  void OnReadInput(cricket::AsyncFile* file);
  void OnMessage(cricket::Message* pmsg);
  void UpdatePrompt();
};

void InitConsole(cricket::PhysicalSocketServer* ss);
CConsole* Console();

#endif // CRICKET_EXAMPLES_CALL_CONSOLE_H__
