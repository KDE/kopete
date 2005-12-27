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

#include <string>
#include <vector>

#include "talk/xmpp/constants.h"
#include "talk/base/thread.h"
#include "talk/base/network.h"
#include "talk/base/socketaddress.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/p2p/base/helpers.h"
#include "talk/p2p/client/basicportallocator.h"
#include "talk/session/receiver.h"
#include "talk/session/sessionsendtask.h"
#include "talk/session/phone/phonesessionclient.h"
#include "talk/examples/call/callclient.h"
#include "talk/examples/call/console.h"
#include "talk/examples/call/presencepushtask.h"
#include "talk/examples/call/presenceouttask.h"

namespace {

const char* CALL_COMMANDS =
"Available commands:\n"
"\n"
"  hangup  Ends the call.\n"
"  mute    Stops sending voice.\n"
"  unmute  Re-starts sending voice.\n"
"";

class CallTask: public ConsoleTask, public sigslot::has_slots<> {
public:
  CallTask(CallClient* call_client, const buzz::Jid& jid, cricket::Call* call)
      : call_client_(call_client), jid_(jid), call_(call) {
  }

  virtual ~CallTask() {}

  virtual void Start() {
    call_client_->phone_client()->SignalCallDestroy.connect(
        this, &CallTask::OnCallDestroy);
    if (!call_) {
      call_ = call_client_->phone_client()->CreateCall();
      call_->SignalSessionState.connect(this, &CallTask::OnSessionState);
      session_ = call_->InitiateSession(jid_);
    }
    call_client_->phone_client()->SetFocus(call_);
  }

  virtual std::string GetPrompt() { return jid_.node(); }

  virtual void ProcessLine(const std::string& line) {
    std::vector<std::string> words;
    ParseLine(line, &words);

    if ((words.size() == 1) && (words[0] == "hangup")) {
      call_->Terminate();
      SignalDone(this);
    } else if ((words.size() == 1) && (words[0] == "mute")) {
      call_->Mute(true);
    } else if ((words.size() == 1) && (words[0] == "unmute")) {
      call_->Mute(false);
    } else {
      console()->Print(CALL_COMMANDS);
    }
  }

private:
  CallClient* call_client_;
  buzz::Jid jid_;
  cricket::Call* call_;
  cricket::Session* session_;

  void OnCallDestroy(cricket::Call* call) {
    if (call == call_) {
      console()->Print("call destroyed");
      SignalDone(this);
    }
  }

  void OnSessionState(cricket::Call* call,
                      cricket::Session* session,
                      cricket::Session::State state) {
    if (state == cricket::Session::STATE_SENTINITIATE) {
      console()->Print("calling...");
    } else if (state == cricket::Session::STATE_RECEIVEDACCEPT) {
      console()->Print("call answered");
    } else if (state == cricket::Session::STATE_RECEIVEDREJECT) {
      console()->Print("call not answered");
      SignalDone(this);
    } else if (state == cricket::Session::STATE_INPROGRESS) {
      console()->Print("call in progress");
    } else if (state == cricket::Session::STATE_RECEIVEDTERMINATE) {
      console()->Print("other side hung up");
      SignalDone(this);
    }
  }
};

const char* RECEIVE_COMMANDS =
"Available commands:\n"
"\n"
"  accept  Accepts the incoming call and switches to it.\n"
"  reject  Rejects the incoming call and stays with the current call.\n"
"";

class ReceiveTask: public ConsoleTask {
public:
  ReceiveTask(CallClient* call_client,
              const buzz::Jid& jid,
              cricket::Call* call)
      : call_client_(call_client), jid_(jid), call_(call) {
  }

  virtual std::string GetPrompt() { return jid_.node(); }

  virtual void ProcessLine(const std::string& line) {
    std::vector<std::string> words;
    ParseLine(line, &words);

    if ((words.size() == 1) && (words[0] == "accept")) {
      assert(call_->sessions().size() == 1);
      call_->AcceptSession(call_->sessions()[0]);
      Console()->Push(new CallTask(call_client_, jid_, call_));
      SignalDone(this);
    } else if ((words.size() == 1) && (words[0] == "reject")) {
      call_->RejectSession(call_->sessions()[0]);
      SignalDone(this);
    } else {
      console()->Print(RECEIVE_COMMANDS);
    }
  }

private:
  CallClient* call_client_;
  buzz::Jid jid_;
  cricket::Call* call_;
};

const char* CONSOLE_COMMANDS =
"Available commands:\n"
"\n"
"  roster       Prints the online friends from your roster.\n"
"  call <name>  Initiates a call to the friend with the given name.\n"
"  quit         Quits the application.\n"
"";

class CallConsoleTask: public ConsoleTask {
public:
  CallConsoleTask(CallClient* call_client) : call_client_(call_client) {}
  virtual ~CallConsoleTask() {}

  virtual std::string GetPrompt() { return "console"; }

  virtual void ProcessLine(const std::string& line) {
    std::vector<std::string> words;
    ParseLine(line, &words);

    if ((words.size() == 1) && (words[0] == "quit")) {
      SignalDone(this);
    } else if ((words.size() == 1) && (words[0] == "roster")) {
      call_client_->PrintRoster();
    } else if ((words.size() == 2) && (words[0] == "call")) {
      call_client_->MakeCallTo(words[1]);
    } else {
      console()->Print(CONSOLE_COMMANDS);
    }
  }

private:
  CallClient* call_client_;
};

const char* DescribeStatus(buzz::Status::Show show, const std::string& desc) {
  switch (show) {
  case buzz::Status::SHOW_XA:      return desc.c_str();
  case buzz::Status::SHOW_ONLINE:  return "online";
  case buzz::Status::SHOW_AWAY:    return "away";
  case buzz::Status::SHOW_DND:     return "do not disturb";
  case buzz::Status::SHOW_CHAT:    return "ready to chat";
  delault:                         return "offline";
  }
}

} // namespace

CallClient::CallClient(buzz::XmppClient* xmpp_client)
    : xmpp_client_(xmpp_client), roster_(new RosterMap) {
  xmpp_client_->SignalStateChange.connect(this, &CallClient::OnStateChange);
  Console()->Push(new CallConsoleTask(this));
}

CallClient::~CallClient() {
  delete roster_;
}

const std::string CallClient::strerror(buzz::XmppEngine::Error err) {
  switch (err) {
   case  buzz::XmppEngine::ERROR_NONE: 
     return "";
   case  buzz::XmppEngine::ERROR_XML:  
     return "Malformed XML or encoding error";
   case  buzz::XmppEngine::ERROR_STREAM: 
     return "XMPP stream error";
   case  buzz::XmppEngine::ERROR_VERSION:
     return "XMPP version error";
   case  buzz::XmppEngine::ERROR_UNAUTHORIZED:
     return "User is not authorized (Confirm your GX cookie at mail.google.com)";
   case  buzz::XmppEngine::ERROR_TLS:
     return "TLS could not be negotiated";
   case	 buzz::XmppEngine::ERROR_AUTH:
     return "Authentication could not be negotiated";
   case  buzz::XmppEngine::ERROR_BIND:
     return "Resource or session binding could not be negotiated";
   case  buzz::XmppEngine::ERROR_CONNECTION_CLOSED:
     return "Connection closed by output handler.";
   case  buzz::XmppEngine::ERROR_DOCUMENT_CLOSED:
     return "Closed by </stream:stream>";
   case  buzz::XmppEngine::ERROR_SOCKET:
     return "Socket error";
  }
}

void CallClient::OnStateChange(buzz::XmppEngine::State state) {
  switch (state) {
  case buzz::XmppEngine::STATE_START:
    Console()->Print("connecting...");
    break;

  case buzz::XmppEngine::STATE_OPENING:
    Console()->Print("logging in...");
    break;

  case buzz::XmppEngine::STATE_OPEN:
    Console()->Print("logged in...");
    InitPhone();
    InitPresence();
    break;

  case buzz::XmppEngine::STATE_CLOSED:
    buzz::XmppEngine::Error error = xmpp_client_->GetError();
    Console()->Print("logged out..." + strerror(error));
    exit(0);
  }
}

void CallClient::InitPhone() {
  std::string client_unique = xmpp_client_->jid().Str();
  cricket::InitRandom(client_unique.c_str(), client_unique.size());

  worker_thread_ = new cricket::Thread();

  network_manager_ = new cricket::NetworkManager();
  
  cricket::SocketAddress *stun_addr = new cricket::SocketAddress("64.233.167.126", 19302);
  port_allocator_ = new cricket::BasicPortAllocator(network_manager_, stun_addr, NULL);

  session_manager_ = new cricket::SessionManager(
      port_allocator_, worker_thread_);
  session_manager_->SignalRequestSignaling.connect(
      this, &CallClient::OnRequestSignaling);
  session_manager_->OnSignalingReady();

  phone_client_ = new cricket::PhoneSessionClient(
      xmpp_client_->jid(),session_manager_);
  phone_client_->SignalCallCreate.connect(this, &CallClient::OnCallCreate);
  phone_client_->SignalSendStanza.connect(this, &CallClient::OnSendStanza);

  receiver_ = new cricket::Receiver(xmpp_client_, phone_client_);
  receiver_->Start();

  worker_thread_->Start();
}

void CallClient::OnRequestSignaling() {
  session_manager_->OnSignalingReady();
}

void CallClient::OnCallCreate(cricket::Call* call) {
  call->SignalSessionState.connect(this, &CallClient::OnSessionState);
}

void CallClient::OnSessionState(cricket::Call* call,
                                cricket::Session* session,
                                cricket::Session::State state) {
  if (state == cricket::Session::STATE_RECEIVEDINITIATE) {
    buzz::Jid jid(session->remote_address());
    Console()->Printf("Incoming call from '%s'", jid.Str().c_str());
    Console()->Push(new ReceiveTask(this, jid, call));
  }
}

void CallClient::OnSendStanza(cricket::SessionClient *client, const buzz::XmlElement* stanza) {
  cricket::SessionSendTask* sender =
      new cricket::SessionSendTask(xmpp_client_, phone_client_);
  sender->Send(stanza);
  sender->Start();
}

void CallClient::InitPresence() {
  presence_push_ = new buzz::PresencePushTask(xmpp_client_);
  presence_push_->SignalStatusUpdate.connect(
    this, &CallClient::OnStatusUpdate);
  presence_push_->Start();

  buzz::Status my_status;
  my_status.set_jid(xmpp_client_->jid());
  my_status.set_available(true);
  my_status.set_invisible(false);
  my_status.set_show(buzz::Status::SHOW_ONLINE);
  my_status.set_priority(0);
  my_status.set_know_capabilities(true);
  my_status.set_phone_capability(true);
  my_status.set_is_google_client(true);
  my_status.set_version("1.0.0.66");

  buzz::PresenceOutTask* presence_out_ =
      new buzz::PresenceOutTask(xmpp_client_);
  presence_out_->Send(my_status);
  presence_out_->Start();
}

void CallClient::OnStatusUpdate(const buzz::Status& status) {
  RosterItem item;
  item.jid = status.jid();
  item.show = status.show();
  item.status = status.status();

  std::string key = item.jid.Str();

  if (status.available() && status.phone_capability()) {
    Console()->Printf("Adding to roster: %s", key.c_str());
    (*roster_)[key] = item;
  } else {
    Console()->Printf("Removing from roster: %s", key.c_str());
    RosterMap::iterator iter = roster_->find(key);
    if (iter != roster_->end())
      roster_->erase(iter);
  }
}

void CallClient::PrintRoster() {
  Console()->Printf("Roster contains %d callable", roster_->size());
  RosterMap::iterator iter = roster_->begin();
  while (iter != roster_->end()) {
    Console()->Printf("%s - %s",
                      iter->second.jid.BareJid().Str().c_str(),
                      DescribeStatus(iter->second.show, iter->second.status));
    iter++;
  }
}

void CallClient::MakeCallTo(const std::string& name) {
  bool found = false;
  buzz::Jid found_jid;

  RosterMap::iterator iter = roster_->begin();
  while (iter != roster_->end()) {
    if (iter->second.jid.node() == name) {
      found = true;
      found_jid = iter->second.jid;
      break;
    }
    ++iter;
  }

  if (found) {
    Console()->Printf("Found online friend '%s'", found_jid.Str().c_str());
    Console()->Push(new CallTask(this, found_jid, NULL));
  } else {
    Console()->Printf("Could not find online friend '%s'", name.c_str());
  } 
}
