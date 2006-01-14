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

#if defined(_MSC_VER) && _MSC_VER < 1300
#pragma warning(disable:4786)
#endif
#include "talk/p2p/client/sessionclient.h"
#include "talk/p2p/base/helpers.h"
#include "talk/base/logging.h"
#include "talk/xmllite/qname.h"
#include "talk/xmpp/constants.h"
#include "talk/xmllite/xmlprinter.h"
#include <iostream>
#undef SetPort

namespace {

// We only allow usernames to be this many characters or fewer.
const size_t kMaxUsernameSize = 16;

}

namespace cricket {

#if 0
>>>>>>
<iq from="..." to="..." type="set" id="27">
  <session xmlns="http://www.google.com/session" type="initiate" id="Dr45JU8A34DF" initiator="...">
    <description xmlns="http://www.whoever.com/whatever">
      ...
    </description>
  </session>
</iq>

<<<<<<
<iq from="..." to="..." type="result" id="27"/>

>>>>>>
<iq from="..." to="..." type="set" id="28">
  <session xmlns="http://www.google.com/session" type="candidates" id="Dr45JU8A34DF" initiator="...">
    <candidate name="rtp" address="X.X.X.X" port="NNN" username="asdf" password="asdf" preference="1.0" type="udp" network="bleh"/>
    <candidate name="rtp" address="X.X.X.X" port="NNN" username="asdf" password="asdf" preference="1.0" type="udp" network="bleh"/>
    <candidate name="rtp" address="X.X.X.X" port="NNN" username="asdf" password="asdf" preference="1.0" type="udp" network="bleh"/>
  </session>
</iq>>

<<<<<<
<iq from="..." to="..." type="result" id="28"/>

#endif

const std::string NS_GOOGLESESSION("http://www.google.com/session");
const buzz::QName QN_GOOGLESESSION_SESSION(true, NS_GOOGLESESSION, "session");
const buzz::QName QN_GOOGLESESSION_CANDIDATE(true, NS_GOOGLESESSION, "candidate");
const buzz::QName QN_GOOGLESESSION_TARGET(true, NS_GOOGLESESSION, "target");
const buzz::QName QN_GOOGLESESSION_COOKIE(true, NS_GOOGLESESSION, "cookie");
const buzz::QName QN_GOOGLESESSION_REGARDING(true, NS_GOOGLESESSION, "regarding");

const buzz::QName QN_TYPE(true, buzz::STR_EMPTY, "type");
const buzz::QName QN_ID(true, buzz::STR_EMPTY, "id");
const buzz::QName QN_INITIATOR(true, buzz::STR_EMPTY, "initiator");
const buzz::QName QN_NAME(true, buzz::STR_EMPTY, "name");
const buzz::QName QN_PORT(true, buzz::STR_EMPTY, "port");
const buzz::QName QN_NETWORK(true, buzz::STR_EMPTY, "network");
const buzz::QName QN_GENERATION(true, buzz::STR_EMPTY, "generation");
const buzz::QName QN_ADDRESS(true, buzz::STR_EMPTY, "address");
const buzz::QName QN_USERNAME(true, buzz::STR_EMPTY, "username");
const buzz::QName QN_PASSWORD(true, buzz::STR_EMPTY, "password");
const buzz::QName QN_PREFERENCE(true, buzz::STR_EMPTY, "preference");
const buzz::QName QN_PROTOCOL(true, buzz::STR_EMPTY, "protocol");
const buzz::QName QN_KEY(true, buzz::STR_EMPTY, "key");

class XmlCookie: public SessionMessage::Cookie {
public:
  XmlCookie(const buzz::XmlElement* elem)
    : elem_(new buzz::XmlElement(*elem)) {
  }

  virtual ~XmlCookie() {
    delete elem_;
  }

  const buzz::XmlElement* elem() const { return elem_; }

  virtual Cookie* Copy() {
    return new XmlCookie(elem_);
  }

private:
  buzz::XmlElement* elem_;
};

SessionClient::SessionClient(SessionManager *session_manager) {
  session_manager_ = session_manager;
  session_manager_->SignalSessionCreate.connect(this, &SessionClient::OnSessionCreateSlot);
  session_manager_->SignalSessionDestroy.connect(this, &SessionClient::OnSessionDestroySlot);
}

SessionClient::~SessionClient() {
}

void SessionClient::OnSessionCreateSlot(Session *session, bool received_initiate) {
  // Does this session belong to this session client?
  if (session->name() == GetSessionDescriptionName()) {
    session->SignalOutgoingMessage.connect(this, &SessionClient::OnOutgoingMessage);
    OnSessionCreate(session, received_initiate);
  }
}

void SessionClient::OnSessionDestroySlot(Session *session) {
  if (session->name() == GetSessionDescriptionName()) {
    session->SignalOutgoingMessage.disconnect(this);
    OnSessionDestroy(session);
  }
}

bool SessionClient::IsClientStanza(const buzz::XmlElement *stanza) {
  // Is it a IQ set stanza?
  if (stanza->Name() != buzz::QN_IQ)
    return false;
  if (stanza->Attr(buzz::QN_TYPE) != buzz::STR_SET)
    return false;

  // Make sure it has the right child element
  const buzz::XmlElement* element
    = stanza->FirstNamed(QN_GOOGLESESSION_SESSION);
  if (element == NULL)
    return false;

  // Is it one of the allowed types?
  std::string type;
  if (element->HasAttr(QN_TYPE)) {
    type = element->Attr(QN_TYPE);
    if (type != "initiate" && type != "accept" && type != "modify" &&
        type != "candidates" && type != "reject" && type != "redirect" &&
        type != "terminate") {
      return false;
    }
  }

  // Does this client own the session description namespace?
  buzz::QName qn_session_desc(GetSessionDescriptionName(), "description");
  const buzz::XmlElement* description = element->FirstNamed(qn_session_desc);
  if (type == "initiate" || type == "accept" || type == "modify") {
    if (description == NULL)
      return false;
  } else {
    if (description != NULL)
      return false;
  }

  // It's good
  return true;
}

void SessionClient::OnIncomingStanza(const buzz::XmlElement *stanza) {
  SessionMessage message;
  if (!ParseIncomingMessage(stanza, message))
    return;

  session_manager_->OnIncomingMessage(message);
}

void SessionClient::OnFailedSend(const buzz::XmlElement *original_stanza,
                                 const buzz::XmlElement *failure_stanza) {
  SessionMessage message;
  if (!ParseIncomingMessage(original_stanza, message))
    return;

  // Note the from/to represents the *original* stanza and not the from/to
  // on any return path
  session_manager_->OnIncomingError(message);
}

bool SessionClient::ParseIncomingMessage(const buzz::XmlElement *stanza,
                                         SessionMessage& message) {
  // Parse stanza into SessionMessage
  const buzz::XmlElement* element
    = stanza->FirstNamed(QN_GOOGLESESSION_SESSION);

  std::string type = element->Attr(QN_TYPE);
  if (type == "initiate" || type == "accept" || type == "modify") {
    ParseInitiateAcceptModify(stanza, message);
  } else if (type == "candidates") {
    ParseCandidates(stanza, message);
  } else if (type == "reject" || type == "terminate") {
    ParseRejectTerminate(stanza, message);
  } else if (type == "redirect") {
    ParseRedirect(stanza, message);
  } else {
    return false;
  }

  return true;
}

void SessionClient::ParseHeader(const buzz::XmlElement *stanza, SessionMessage &message) {
  if (stanza->HasAttr(buzz::QN_FROM))
    message.set_from(stanza->Attr(buzz::QN_FROM));
  if (stanza->HasAttr(buzz::QN_TO))
    message.set_to(stanza->Attr(buzz::QN_TO));

  const buzz::XmlElement *element
    = stanza->FirstNamed(QN_GOOGLESESSION_SESSION);
  if (element->HasAttr(QN_ID))
    message.session_id().set_id_str(element->Attr(QN_ID));

  if (element->HasAttr(QN_INITIATOR))
    message.session_id().set_initiator(element->Attr(QN_INITIATOR));

  std::string type = element->Attr(QN_TYPE);
  if (type == "initiate") {
    message.set_type(SessionMessage::TYPE_INITIATE);
  } else if (type == "accept") {
    message.set_type(SessionMessage::TYPE_ACCEPT);
  } else if (type == "modify") {
    message.set_type(SessionMessage::TYPE_MODIFY);
  } else if (type == "candidates") {
    message.set_type(SessionMessage::TYPE_CANDIDATES);
  } else if (type == "reject") {
    message.set_type(SessionMessage::TYPE_REJECT);
  } else if (type == "redirect") {
    message.set_type(SessionMessage::TYPE_REDIRECT);
  } else if (type == "terminate") {
    message.set_type(SessionMessage::TYPE_TERMINATE);
  } else {
    assert(false);
  }
}

void SessionClient::ParseInitiateAcceptModify(const buzz::XmlElement *stanza, SessionMessage &message) {
  // Pull the standard header pieces out
  ParseHeader(stanza, message);

  // Parse session description
  const buzz::XmlElement *session
    = stanza->FirstNamed(QN_GOOGLESESSION_SESSION);
  buzz::QName qn_session_desc(GetSessionDescriptionName(), "description");
  const buzz::XmlElement* desc_elem = session->FirstNamed(qn_session_desc);
  const SessionDescription *description = NULL;
  if (desc_elem)
    description = CreateSessionDescription(desc_elem);
  message.set_name(GetSessionDescriptionName());
  message.set_description(description);
}

void SessionClient::ParseCandidates(const buzz::XmlElement *stanza, SessionMessage &message) {
  // Pull the standard header pieces out
  ParseHeader(stanza, message);

  // Parse candidates and session description
  std::vector<Candidate> candidates;
  const buzz::XmlElement *element
    = stanza->FirstNamed(QN_GOOGLESESSION_SESSION);
  const buzz::XmlElement *child = element->FirstElement();
  while (child != NULL) {
    if (child->Name() == QN_GOOGLESESSION_CANDIDATE) {
      Candidate candidate;
      if (ParseCandidate(child, &candidate))
        candidates.push_back(candidate);
    }
    child = child->NextElement();
  }
  message.set_name(GetSessionDescriptionName());
  message.set_candidates(candidates);
}

void SessionClient::ParseRejectTerminate(const buzz::XmlElement *stanza, SessionMessage &message) {
  // Reject and terminate are very simple
  ParseHeader(stanza, message);
}

bool SessionClient::ParseCandidate(const buzz::XmlElement *child,
                                   Candidate* candidate) {
  // Check for all of the required attributes.
  if (!child->HasAttr(QN_NAME) ||
      !child->HasAttr(QN_ADDRESS) ||
      !child->HasAttr(QN_PORT) ||
      !child->HasAttr(QN_USERNAME) ||
      !child->HasAttr(QN_PREFERENCE) ||
      !child->HasAttr(QN_PROTOCOL) ||
      !child->HasAttr(QN_GENERATION)) {
    LOG(LERROR) << "Candidate missing required attribute";
    return false;
  }

  SocketAddress address;
  address.SetIP(child->Attr(QN_ADDRESS));
  std::istringstream ist(child->Attr(QN_PORT));
  int port;
  ist >> port;
  address.SetPort(port);

  if (address.IsAny()) {
    LOG(LERROR) << "Candidate has address 0";
    return false;
  }

  // Always disallow addresses that refer to the local host.
  if (address.IsLocalIP()) {
    LOG(LERROR) << "Candidate has local IP address";
    return false;
  }

  // Disallow all ports below 1024, except for 80 and 443 on public addresses.
  if (port < 1024) {
    if ((port != 80) && (port != 443)) {
      LOG(LERROR) << "Candidate has port below 1024, not 80 or 443";
      return false;
    }
    if (address.IsPrivateIP()) {
      LOG(LERROR) << "Candidate has port of 80 or 443 with private IP address";
      return false;
    }
  }

  candidate->set_name(child->Attr(QN_NAME));
  candidate->set_address(address);
  candidate->set_username(child->Attr(QN_USERNAME));
  candidate->set_preference_str(child->Attr(QN_PREFERENCE));
  candidate->set_protocol(child->Attr(QN_PROTOCOL));
  candidate->set_generation_str(child->Attr(QN_GENERATION));

  // Check that the username is not too long and does not use any bad chars.
  if (candidate->username().size() > kMaxUsernameSize) {
    LOG(LERROR) << "Candidate username is too long";
    return false;
  }
  if (!IsBase64Encoded(candidate->username())) {
    LOG(LERROR) << "Candidate username has non-base64 encoded characters";
    return false;
  }

  // Look for the non-required attributes.
  if (child->HasAttr(QN_PASSWORD))
    candidate->set_password(child->Attr(QN_PASSWORD));
  if (child->HasAttr(QN_TYPE))
    candidate->set_type(child->Attr(QN_TYPE));
  if (child->HasAttr(QN_NETWORK))
    candidate->set_network_name(child->Attr(QN_NETWORK));

  return true;
}

void SessionClient::ParseRedirect(const buzz::XmlElement *stanza, SessionMessage &message) {
  // Pull the standard header pieces out
  ParseHeader(stanza, message);
  const buzz::XmlElement *session = stanza->FirstNamed(QN_GOOGLESESSION_SESSION);

  // Parse the target and cookie.

  const buzz::XmlElement* target = session->FirstNamed(QN_GOOGLESESSION_TARGET);
  if (target)
    message.set_redirect_target(target->Attr(QN_NAME));

  const buzz::XmlElement* cookie = session->FirstNamed(QN_GOOGLESESSION_COOKIE);
  if (cookie)
    message.set_redirect_cookie(new XmlCookie(cookie));
}

void SessionClient::OnOutgoingMessage(Session *session, const SessionMessage &message) {
  // Translate the message into an XMPP stanza

  buzz::XmlElement *result = NULL;
  switch (message.type()) {
  case SessionMessage::TYPE_INITIATE:
  case SessionMessage::TYPE_ACCEPT:
  case SessionMessage::TYPE_MODIFY:
    result = TranslateInitiateAcceptModify(message);
    break;

  case SessionMessage::TYPE_CANDIDATES:
    result = TranslateCandidates(message);
    break;

  case SessionMessage::TYPE_REJECT:
  case SessionMessage::TYPE_TERMINATE:
    result = TranslateRejectTerminate(message);
    break;

  case SessionMessage::TYPE_REDIRECT:
    result = TranslateRedirect(message);
    break;
  }

  // Send the stanza. Note that SessionClient is passing on ownership
  // of result.
  if (result != NULL) {
    SignalSendStanza(this, result);
  }
}

buzz::XmlElement *SessionClient::TranslateHeader(const SessionMessage &message) {
  buzz::XmlElement *result = new buzz::XmlElement(buzz::QN_IQ);
  result->AddAttr(buzz::QN_TO, message.to());
  result->AddAttr(buzz::QN_TYPE, buzz::STR_SET);
  buzz::XmlElement *session = new buzz::XmlElement(QN_GOOGLESESSION_SESSION, true);
  result->AddElement(session);
  switch (message.type()) {
  case SessionMessage::TYPE_INITIATE:
    session->AddAttr(QN_TYPE, "initiate");
    break;
  case SessionMessage::TYPE_ACCEPT:
    session->AddAttr(QN_TYPE, "accept");
    break;
  case SessionMessage::TYPE_MODIFY:
    session->AddAttr(QN_TYPE, "modify");
    break;
  case SessionMessage::TYPE_CANDIDATES:
    session->AddAttr(QN_TYPE, "candidates");
    break;
  case SessionMessage::TYPE_REJECT:
    session->AddAttr(QN_TYPE, "reject");
    break;
  case SessionMessage::TYPE_REDIRECT:
    session->AddAttr(QN_TYPE, "redirect");
    break;
  case SessionMessage::TYPE_TERMINATE:
    session->AddAttr(QN_TYPE, "terminate");
    break;
  }
  session->AddAttr(QN_ID, message.session_id().id_str());
  session->AddAttr(QN_INITIATOR, message.session_id().initiator());
  return result;
}

buzz::XmlElement *SessionClient::TranslateCandidate(const Candidate &candidate) {
  buzz::XmlElement *result = new buzz::XmlElement(QN_GOOGLESESSION_CANDIDATE);
  result->AddAttr(QN_NAME, candidate.name());
  result->AddAttr(QN_ADDRESS, candidate.address().IPAsString());
  result->AddAttr(QN_PORT, candidate.address().PortAsString());
  result->AddAttr(QN_USERNAME, candidate.username());
  result->AddAttr(QN_PASSWORD, candidate.password());
  result->AddAttr(QN_PREFERENCE, candidate.preference_str());
  result->AddAttr(QN_PROTOCOL, candidate.protocol());
  result->AddAttr(QN_TYPE, candidate.type());
  result->AddAttr(QN_NETWORK, candidate.network_name());
  result->AddAttr(QN_GENERATION, candidate.generation_str());
  return result;
}

buzz::XmlElement *SessionClient::TranslateInitiateAcceptModify(const SessionMessage &message) {
  // Header info common to all message types
  buzz::XmlElement *result = TranslateHeader(message);
  buzz::XmlElement *session = result->FirstNamed(QN_GOOGLESESSION_SESSION);

  // Candidates
  assert(message.candidates().size() == 0);

  // Session Description
  buzz::XmlElement* description = TranslateSessionDescription(message.description());
  assert(description->Name().LocalPart() == "description");
  assert(description->Name().Namespace() == GetSessionDescriptionName());
  session->AddElement(description);

  if (message.redirect_cookie() != NULL) {
    const buzz::XmlElement* cookie =
      reinterpret_cast<XmlCookie*>(message.redirect_cookie())->elem();
    for (const buzz::XmlElement* elem = cookie->FirstElement(); elem; elem = elem->NextElement())
      session->AddElement(new buzz::XmlElement(*elem));
  }

  return result;
}

buzz::XmlElement *SessionClient::TranslateCandidates(const SessionMessage &message) {
  // Header info common to all message types
  buzz::XmlElement *result = TranslateHeader(message);
  buzz::XmlElement *session = result->FirstNamed(QN_GOOGLESESSION_SESSION);

  // Candidates
  std::vector<Candidate>::const_iterator it;
  for (it = message.candidates().begin(); it != message.candidates().end(); it++)
    session->AddElement(TranslateCandidate(*it));

  return result;
}

buzz::XmlElement *SessionClient::TranslateRejectTerminate(const SessionMessage &message) {
  // These messages are simple, and only have a header
  return TranslateHeader(message);
}

buzz::XmlElement *SessionClient::TranslateRedirect(const SessionMessage &message) {
  // Header info common to all message types
  buzz::XmlElement *result = TranslateHeader(message);
  buzz::XmlElement *session = result->FirstNamed(QN_GOOGLESESSION_SESSION);

  assert(message.candidates().size() == 0);
  assert(message.description() == NULL);

  assert(message.redirect_target().size() > 0);
  buzz::XmlElement* target = new buzz::XmlElement(QN_GOOGLESESSION_TARGET);
  target->AddAttr(QN_NAME, message.redirect_target());
  session->AddElement(target);

  buzz::XmlElement* cookie = new buzz::XmlElement(QN_GOOGLESESSION_COOKIE);
  session->AddElement(cookie);

  // If the message does not have a redirect cookie, then this is a redirect
  // initiated by us.  We will automatically add a regarding cookie.
  if (message.redirect_cookie() == NULL) {
    buzz::XmlElement* regarding = new buzz::XmlElement(QN_GOOGLESESSION_REGARDING);
    regarding->AddAttr(QN_NAME, GetJid().BareJid().Str());
    cookie->AddElement(regarding);
  } else {
    const buzz::XmlElement* cookie_elem = 
        reinterpret_cast<const XmlCookie*>(message.redirect_cookie())->elem();
    const buzz::XmlElement* elem;
    for (elem = cookie_elem->FirstElement(); elem; elem = elem->NextElement())
      cookie->AddElement(new buzz::XmlElement(*elem));
  }

  return result;
}

SessionManager *SessionClient::session_manager() {
  return session_manager_;
}

} // namespace cricket
