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

#include "talk/xmpp/xmppclientsettings.h"
#include "talk/examples/login/xmppthread.h"
#include "talk/examples/login/xmppauth.h"
#include "talk/examples/call/callclient.h"
#include "talk/examples/call/console.h"

void GetString(const char* desc, char* out) {
  printf("%s: ", desc);
  fflush(stdout);
  scanf("%s", out);
}

int main(int argc, char **argv) {
  // TODO: Make this into a console task
  char username[256], auth_cookie[256];
  GetString("Username", username);
  GetString("Auth Cookie", auth_cookie);

  printf("Logging in as %s@gmail.com\n", username);

  // We will run the console and the XMPP client on the main thread.  The
  // CallClient maintains a separate worker thread for voice.

  cricket::PhysicalSocketServer ss;
  cricket::Thread main_thread(&ss);
  cricket::ThreadManager::SetCurrent(&main_thread);

  InitConsole(&ss);
  XmppPump pump;
  CallClient client(pump.client());

  buzz::XmppClientSettings xcs;
  xcs.set_user(username);
  xcs.set_host("gmail.com");
  xcs.set_use_tls(false);
  xcs.set_auth_cookie(auth_cookie);
  xcs.set_server(cricket::SocketAddress("talk.google.com", 5222));
  pump.DoLogin(xcs, new XmppSocket(false), new XmppAuth());

  main_thread.Loop();

  return 0;
}
