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

#ifndef _SESSIONCLIENT_H_
#define _SESSIONCLIENT_H_

#include "talk/p2p/base/sessiondescription.h"
#include "talk/p2p/base/sessionmessage.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/xmllite/xmlelement.h"
#include "talk/xmpp/jid.h"
namespace cricket {

// Generic XMPP session client. This class knows how to translate
// a SessionMessage to and from XMPP stanzas. The SessionDescription
// is a custom description implemented by the client.

// This class knows how to talk to the session manager, however the
// session manager doesn't have knowledge of a particular SessionClient.

class SessionClient : public sigslot::has_slots<> {
public:
  SessionClient(SessionManager *psm);
  virtual ~SessionClient();

  // Call this method to determine if a stanza is for this session client
  bool IsClientStanza(const buzz::XmlElement *stanza);

  // Call this method to deliver a stanza to this session client
  void OnIncomingStanza(const buzz::XmlElement *stanza);

  // Call this whenever an error is recieved in response to an outgoing
  // session IQ.  Include the original stanza and any failure stanza.  If
  // the failure is due to a time out, the failure_stanza should be NULL
  void OnFailedSend(const buzz::XmlElement* original_stanza,
                    const buzz::XmlElement* failure_stanza);

  SessionManager *session_manager();

  // Implement this method for stanza sending
  sigslot::signal2<SessionClient*, const buzz::XmlElement*> SignalSendStanza;

protected:
  // Override these to know when sessions belonging to this client create/destroy

  virtual void OnSessionCreate(Session * /*session*/, bool /*received_initiate*/) {}
  virtual void OnSessionDestroy(Session * /*session*/) {}

  // Implement these methods for a custom session description
  virtual const SessionDescription *CreateSessionDescription(const buzz::XmlElement *element) = 0;
  virtual buzz::XmlElement *TranslateSessionDescription(const SessionDescription *description) = 0;
  virtual const std::string &GetSessionDescriptionName() = 0;
  virtual const buzz::Jid &GetJid() const = 0;

  SessionManager *session_manager_;

private:
  void OnSessionCreateSlot(Session *session, bool received_initiate);
  void OnSessionDestroySlot(Session *session);
  void OnOutgoingMessage(Session *session, const SessionMessage &message);
  void ParseHeader(const buzz::XmlElement *stanza, SessionMessage &message);
  bool ParseCandidate(const buzz::XmlElement *child, Candidate* candidate);
  bool ParseIncomingMessage(const buzz::XmlElement *stanza,
                            SessionMessage& message);
  void ParseInitiateAcceptModify(const buzz::XmlElement *stanza, SessionMessage &message);
  void ParseCandidates(const buzz::XmlElement *stanza, SessionMessage &message);
  void ParseRejectTerminate(const buzz::XmlElement *stanza, SessionMessage &message);
  void ParseRedirect(const buzz::XmlElement *stanza, SessionMessage &message);
  buzz::XmlElement *TranslateHeader(const SessionMessage &message);
  buzz::XmlElement *TranslateCandidate(const Candidate &candidate);
  buzz::XmlElement *TranslateInitiateAcceptModify(const SessionMessage &message);
  buzz::XmlElement *TranslateCandidates(const SessionMessage &message);
  buzz::XmlElement *TranslateRejectTerminate(const SessionMessage &message);
  buzz::XmlElement *TranslateRedirect(const SessionMessage &message);

};

} // namespace cricket

#endif // _SESSIONCLIENT_H_
