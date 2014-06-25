/*************************************************************************
 * Copyright <2007 - 2013>  <Michael Zanetti> <mzanetti@kde.org>         *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/

/**
  * @author Michael Zanetti
  */

#include "otrlchatinterface.h"
#include "privkeypopup.h"

#include <kopetechatsession.h>
#include <kopeteaccount.h>
#include <kopeteaccountmanager.h>
#include <kopetemessageevent.h>
#include <kopetecontactlist.h>
#include <kopetemetacontact.h>
#include <ui/kopeteview.h>
#include <kopeteprotocol.h>
#include <kopetecontact.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kpassivepopup.h>
#include <kpushbutton.h>

#include <Qt>
#include <qlabel.h>
#include <qnamespace.h>
#include <qeventloop.h>
#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qwidget.h>
#include <qtextdocument.h>

extern "C"{
#include <sys/types.h>
#include <sys/stat.h>
}

OtrlChatInterface *OtrlChatInterface::mSelf = 0;
static OtrlUserState userstate;
static OtrlPolicy confPolicy;
//static void *updateContextList = 0;
static Kopete::Plugin *chatPlugin = 0;

/***************************** Gui_UI_Ops for libotr **********************************/
OtrlPolicy OtrlChatInterface::policy(void *opdata, ConnContext *context){

	Q_UNUSED(context)

	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	bool noerr;

	// Disable OTR for IRC
	if( session->protocol()->pluginId() == "IRCProtocol" ){
//		kdDebug() << "Disabling OTR for: " << session->protocol()->pluginId() << endl;
		return OTRL_POLICY_NEVER;
	}
	QString policy;
	policy = session->members().first()->metaContact()->pluginData( chatPlugin, QString("otr_policy") );
//	kdDebug() << "Metacontact policy is: " << policy.toInt( &noerr, 10) << endl;
	switch( policy.toInt( &noerr, 10 ) ){
		case 1:
			return OTRL_POLICY_ALWAYS;
		case 2:
			return OTRL_POLICY_OPPORTUNISTIC;
		case 3:
			return OTRL_POLICY_MANUAL;
		case 4:
			return OTRL_POLICY_NEVER;
		default:
			return confPolicy;
	}
}

void OtrlChatInterface::create_privkey(void *opdata, const char *accountname, const char *protocol){
	Q_UNUSED(accountname)
	Q_UNUSED(protocol)
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);

	if( !session->view() ){
		session->raiseView();
	}
	PrivKeyPopup *popup = new PrivKeyPopup( session->view()->mainWidget() );
	popup->show();
	popup->setCloseLock( true );

	OtrlChatInterface::self()->generatePrivateKey(session->account()->accountId(), session->protocol()->displayName());

	popup->setCloseLock( false );
	popup->close();

	OtrlChatInterface::self()->replayStoredMessages();
}

int OtrlChatInterface::is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient){

	Q_UNUSED(accountname)
	Q_UNUSED(protocol)

	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::ContactPtrList list = session->members();
	for( int i = 0; i < list.size(); i++ ){
		if( list.at(i)->contactId().compare( recipient) == 0 ){
			Kopete::OnlineStatus status = session->contactOnlineStatus( list.at(i) );
			if( status == Kopete::OnlineStatus::Unknown){
				return -1;
			} else if( status == Kopete::OnlineStatus::Offline ){
				return 0;
			} else {
				return 1;
			}
		}
	}
	return -1;
}

void OtrlChatInterface::inject_message( void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message ){

	Q_UNUSED(accountname)
	Q_UNUSED(protocol)

//	kDebug(14318) << "Sending message:" << message;

	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::ContactPtrList list = session->members();
	for( int i = 0; i < list.size(); i++ ){
		if( list.at(i)->contactId().compare( recipient ) == 0 ){
			Kopete::Message msg( session->account()->myself(), list.at(i) );
			msg.setPlainBody( QString( message ) );
			msg.setDirection( Kopete::Message::Outbound );
			session->sendMessage( msg );
			return;
		}
	}
}

void OtrlChatInterface::update_context_list(void *opdata){
//Not used...
	Q_UNUSED(opdata)
}

void OtrlChatInterface::new_fingerprint(void *opdata, OtrlUserState us, const char *accountname, const char *protocol, const char *username, unsigned char fingerprint[20]){

	Q_UNUSED(us)
	Q_UNUSED(accountname)
	Q_UNUSED(protocol)
	Q_UNUSED(username)
	Q_UNUSED(fingerprint)

//	kdDebug() << "Received a new Fingerprint" << endl;
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::Message msg( session->members().first(), session->account()->myself() );
	msg.setHtmlBody( i18n("<b>Received a new fingerprint from <a>%1</a>. You should authenticate this contact.</b>", session->members().first()->contactId()) );
	msg.setDirection( Kopete::Message::Internal );
	session->appendMessage( msg );
}

void OtrlChatInterface::write_fingerprints(void *opdata){

	Q_UNUSED(opdata)

//	kdDebug() << "Writing fingerprints" << endl;
	QString savePath = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints";
	otrl_privkey_write_fingerprints( userstate, savePath.toLocal8Bit() );
}

void OtrlChatInterface::gone_secure(void *opdata, ConnContext *context){
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);

	if( context->active_fingerprint->trust && context->active_fingerprint->trust[0] ){
		Kopete::Message msg( session->members().first(), session->account()->myself() );
		msg.setHtmlBody( i18n("<b>Private OTR session started.</b>") );
		msg.setDirection( Kopete::Message::Internal );
		session->appendMessage( msg );
		OtrlChatInterface::self()->emitGoneSecure(session, 2);
	} else {
		Kopete::Message msg( session->members().first(), session->account()->myself() );
		msg.setHtmlBody( i18n("<b>Unverified OTR session started.</b>") );
		msg.setDirection( Kopete::Message::Internal );
		session->appendMessage( msg );
		OtrlChatInterface::self()->emitGoneSecure( ((Kopete::ChatSession*)opdata), 1 );
	}

	session->setProperty("otr-instag", QString::number(context->their_instance));
}

void OtrlChatInterface::gone_insecure(void *opdata, ConnContext *context){

	Q_UNUSED(context)

//	kdDebug() << "gone insecure" << endl;
	OtrlChatInterface::self()->emitGoneSecure(((Kopete::ChatSession*)opdata), 0);
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::Message msg( session->members().first(), session->account()->myself() );
	msg.setHtmlBody( i18n("<b>OTR Session ended. Note that the conversation is now insecure.</b>") );
	msg.setDirection( Kopete::Message::Internal );
	session->appendMessage( msg );
}

void OtrlChatInterface::still_secure(void *opdata, ConnContext *context, int is_reply){

	Q_UNUSED(is_reply)

	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);

	Kopete::Message msg( session->members().first(), session->account()->myself() );
	msg.setHtmlBody( i18n("<b>OTR connection refreshed successfully.</b>") );
	msg.setDirection( Kopete::Message::Internal );
	session->appendMessage( msg );

	if( context->active_fingerprint->trust && context->active_fingerprint->trust[0] ){
		OtrlChatInterface::self()->emitGoneSecure( session, 2);
	} else {
		OtrlChatInterface::self()->emitGoneSecure( session, 1);
	}
}

int OtrlChatInterface::max_message_size(void *opdata, ConnContext *context){
	Q_UNUSED(context)

	if (!opdata) {
		kDebug(14318) << "Trying to determine max message size of unknown session. Fragmentation will not work.";
		return 0;
	}

	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);

	kDebug(14318) << session->protocol()->pluginId();

	if( session->protocol()->pluginId() == "WlmProtocol" ){
		return 1409;
	} else if( session->protocol()->pluginId() == "ICQProtocol" ){
		return 1274;
	} else if( session->protocol()->pluginId() == "AIMProtocol" ){
		return 1274;
	} else if( session->protocol()->pluginId() == "YahooProtocol" ){
		return 700;
	}

	// Jabber doesn't need fragmentation. Return 0 to disable.
	// GaduGadu seems to not need fragmentation too.
	return 0;
}

const char* OtrlChatInterface::otr_error_message(void *opdata, ConnContext *context, OtrlErrorCode err_code) {
	Q_UNUSED(opdata)

	char *err_msg = 0;
	switch (err_code)
	{
	case OTRL_ERRCODE_NONE :
		break;
	case OTRL_ERRCODE_ENCRYPTION_ERROR : {
		QString message = i18n("Error occurred encrypting message.");
		err_msg = (char*)malloc(message.length() + 1);
		memset(err_msg, 0, message.length() + 1);
		memcpy(err_msg, message.toUtf8().data(), message.length());
		break;
	}
	case OTRL_ERRCODE_MSG_NOT_IN_PRIVATE :
		if (context) {
		QString message = i18n("You sent encrypted data to %1, who wasn't expecting it.", QLatin1String(context->accountname));
		err_msg = (char*)malloc(message.length() + 1);
		memset(err_msg, 0, message.length() + 1);
		memcpy(err_msg, message.toUtf8().data(), message.length());
		}
		break;
	case OTRL_ERRCODE_MSG_UNREADABLE : {
		QString message = i18n("You transmitted an unreadable encrypted message.");
		err_msg = (char*)malloc(message.length() + 1);
		memset(err_msg, 0, message.length() + 1);
		memcpy(err_msg, message.toUtf8().data(), message.length());
		break;
	}
	case OTRL_ERRCODE_MSG_MALFORMED : {
		QString message = i18n("You transmitted a malformed data message.");
		err_msg = (char*)malloc(message.length() + 1);
		memset(err_msg, 0, message.length() + 1);
		memcpy(err_msg, message.toUtf8().data(), message.length());
		break;
	}
	}
	return err_msg;
}

void OtrlChatInterface::otr_error_message_free(void *opdata, const char *err_msg) {
	Q_UNUSED(opdata)
	if (err_msg) {
		free((char*)err_msg);
	}
}

const char *OtrlChatInterface::resent_msg_prefix(void *opdata, ConnContext *context) {
	Q_UNUSED(opdata)
	Q_UNUSED(context)

	QString message = i18n("[resent]");
	char *msg_prefix = (char*)malloc(message.length() + 1);
	memset(msg_prefix, 0, message.length() + 1);
	memcpy(msg_prefix, message.toUtf8().data(), message.length());
	return msg_prefix;
}

void OtrlChatInterface::resent_msg_prefix_free(void *opdata, const char *prefix) {
	Q_UNUSED(opdata)

	if (prefix) {
		free((char*)prefix);
	}
}

void OtrlChatInterface::handle_smp_event(void *opdata, OtrlSMPEvent smp_event, ConnContext *context, unsigned short progress_percent, char *question) {
    Q_UNUSED(progress_percent)

	Kopete::ChatSession *chatSession = (Kopete::ChatSession*)opdata;

	if (!context) {
		return;
	}

	switch (smp_event) {
	case OTRL_SMPEVENT_NONE :
		break;
	case OTRL_SMPEVENT_ASK_FOR_SECRET :
		new AuthenticationWizard( chatSession->view(true)->mainWidget(), context, chatSession, false );
		break;
	case OTRL_SMPEVENT_ASK_FOR_ANSWER :
		new AuthenticationWizard( chatSession->view(true)->mainWidget(), context, chatSession, false, QLatin1String(question) );
		break;
	case OTRL_SMPEVENT_IN_PROGRESS :
		AuthenticationWizard::findWizard(chatSession)->nextState();
		break;
	case OTRL_SMPEVENT_SUCCESS :
		if (context->active_fingerprint->trust && context->active_fingerprint->trust[0]) {
			AuthenticationWizard::findWizard(chatSession)->finished(true, true);
			kDebug(14318) << "trust found";
			Kopete::Message msg( chatSession->members().first(), chatSession->account()->myself() );
			msg.setHtmlBody( i18n("Authentication with <b>%1</b> successful. The conversation is now secure.", formatContact(chatSession->members().first()->contactId())));
			msg.setDirection( Kopete::Message::Internal );
			chatSession->appendMessage( msg );
			OtrlChatInterface::self()->emitGoneSecure( chatSession, 2 );
		} else {
			AuthenticationWizard::findWizard(chatSession)->finished(true, false);
			kDebug(14318) << "trust _NOT_ found";
			Kopete::Message msg( chatSession->members().first(), chatSession->account()->myself() );
			msg.setHtmlBody( i18n("<b>%1</b> has successfully authenticated you. You may want to authenticate this contact as well by asking your own question.", formatContact(chatSession->members().first()->contactId())));
			msg.setDirection( Kopete::Message::Internal );
			chatSession->appendMessage( msg );
			OtrlChatInterface::self()->emitGoneSecure( chatSession, 1 );
		}
		break;
	case OTRL_SMPEVENT_FAILURE : {
		AuthenticationWizard::findWizard(chatSession)->finished(false, false);
		Kopete::Message msg( chatSession->members().first(), chatSession->account()->myself() );
		msg.setHtmlBody( i18n("Authentication with <b>%1</b> failed. The conversation is now insecure.", formatContact(chatSession->members().first()->contactId())));
		msg.setDirection( Kopete::Message::Internal );
		chatSession->appendMessage( msg );
		OtrlChatInterface::self()->emitGoneSecure( chatSession, 1 );
		break;
	}
	case OTRL_SMPEVENT_ABORT :
	case OTRL_SMPEVENT_CHEATED :
	case OTRL_SMPEVENT_ERROR :
		AuthenticationWizard *wizard = AuthenticationWizard::findWizard(chatSession);
		if (wizard) {
			wizard->finished(false, false);
		}
		OtrlChatInterface::self()->abortSMP( context, chatSession );
		break;
	}
}

void OtrlChatInterface::handle_msg_event(void *opdata, OtrlMessageEvent msg_event, ConnContext *context, const char* message, gcry_error_t err) {

	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::ContactPtrList list = session->members();
	Kopete::Message msg( session->members().first(), session->account()->myself() );

	switch (msg_event)
	{
	case OTRL_MSGEVENT_NONE:
		break;
	case OTRL_MSGEVENT_ENCRYPTION_REQUIRED:
		msg.setHtmlBody( i18n( "You attempted to send an unencrypted message to %1.", QLatin1String(context->username) ) );
		msg.setDirection( Kopete::Message::Internal );
		break;
	case OTRL_MSGEVENT_ENCRYPTION_ERROR:
		msg.setHtmlBody( i18n( "An error occurred when encrypting your message. The message was not sent." ) );
		msg.setDirection( Kopete::Message::Internal );
		break;
	case OTRL_MSGEVENT_CONNECTION_ENDED:
		msg.setHtmlBody( i18n( "%1 has already closed his/her private connection to you. Your message was not sent. Either end your private conversation, or restart it.", QLatin1String(context->username) ) );
		msg.setDirection( Kopete::Message::Internal );
		break;
	case OTRL_MSGEVENT_SETUP_ERROR:
		if (!err) {
			err = GPG_ERR_INV_VALUE;
		}
		switch(gcry_err_code(err)) {
		case GPG_ERR_INV_VALUE:
			kDebug(14318) << "Error setting up private conversation: Malformed message received";
		default:
			kDebug(14318) << "Error setting up private conversation:" << err;
		}

		msg.setHtmlBody( i18n( "OTR error" ) );
		msg.setDirection( Kopete::Message::Internal );
		break;
	case OTRL_MSGEVENT_MSG_REFLECTED:
		msg.setHtmlBody( i18n( "We are receiving our own OTR messages. You are either trying to talk to yourself, or someone is reflecting your messages back at you." ) );
		msg.setDirection( Kopete::Message::Internal );
		break;
	case OTRL_MSGEVENT_MSG_RESENT:
		msg.setHtmlBody( i18n( "<b>The last message to %1 was resent.</b>", QLatin1String(context->username) ) );
		msg.setDirection( Kopete::Message::Internal );
		break;
	case OTRL_MSGEVENT_RCVDMSG_NOT_IN_PRIVATE:
		msg.setHtmlBody( i18n( "<b>The encrypted message received from %1 is unreadable, as you are not currently communicating privately.</b>", QLatin1String(context->username) ) );
		msg.setDirection( Kopete::Message::Inbound );
		OtrlChatInterface::self()->m_blackistIds.append(msg.id());
		break;
	case OTRL_MSGEVENT_RCVDMSG_UNREADABLE:
		msg.setHtmlBody( i18n( "We received an unreadable encrypted message from %1.", QLatin1String(context->username) ) );
		msg.setDirection( Kopete::Message::Internal );
		break;
	case OTRL_MSGEVENT_RCVDMSG_MALFORMED:
		msg.setHtmlBody( i18n( "We received a malformed data message from %1.", QLatin1String(context->username) ) );
		msg.setDirection( Kopete::Message::Internal );
		break;
	case OTRL_MSGEVENT_LOG_HEARTBEAT_RCVD:
		kDebug(14318) << "Heartbeat received from" << context->username;
		return;
	case OTRL_MSGEVENT_LOG_HEARTBEAT_SENT:
		kDebug(14318) << "Heartbeat sent to" << context->username;
		break;
	case OTRL_MSGEVENT_RCVDMSG_GENERAL_ERR:
		msg.setHtmlBody( QLatin1String(message) );
		msg.setDirection( Kopete::Message::Internal );
		session->appendMessage( msg );
		break;
	case OTRL_MSGEVENT_RCVDMSG_UNENCRYPTED:
		msg.setHtmlBody( i18n("<b>The following message received from %1 was <i>not</i> encrypted: [</b>%2<b>]</b>", QLatin1String(context->username), QLatin1String(message) ) );
		msg.setDirection( Kopete::Message::Inbound );
		OtrlChatInterface::self()->m_blackistIds.append(msg.id());
		break;
	case OTRL_MSGEVENT_RCVDMSG_UNRECOGNIZED:
		kDebug(14318) << "Unrecognized OTR message received from" << context->username;
		break;
	case OTRL_MSGEVENT_RCVDMSG_FOR_OTHER_INSTANCE:
		msg.setHtmlBody( i18n( "%1 has sent an encrypted message intended for a different session. If you are logged in multiple times, another session may have received the message.", QLatin1String(context->username) ) );
		msg.setDirection( Kopete::Message::Inbound );
		OtrlChatInterface::self()->m_blackistIds.append(msg.id());
		break;
	}

	session->appendMessage( msg );
}

void OtrlChatInterface::create_instag(void *opdata, const char *accountname, const char *protocol) {
	Q_UNUSED(opdata)
	QString storeFile = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "instags";
	otrl_instag_generate(OtrlChatInterface::self()->getUserstate(), storeFile.toLocal8Bit(), accountname, protocol);
}

void OtrlChatInterface::timer_control(void *opdata, unsigned int interval) {
	Q_UNUSED(opdata)
	if (interval > 0) {
		OtrlChatInterface::self()->m_forwardSecrecyTimer.start(interval * 1000);
	} else {
		OtrlChatInterface::self()->m_forwardSecrecyTimer.stop();
	}
}

OtrlMessageAppOps OtrlChatInterface::ui_ops = {
	OtrlChatInterface::policy,
	OtrlChatInterface::create_privkey,
	OtrlChatInterface::is_logged_in,
	OtrlChatInterface::inject_message,
	OtrlChatInterface::update_context_list,
	OtrlChatInterface::new_fingerprint,
	OtrlChatInterface::write_fingerprints,
	OtrlChatInterface::gone_secure,
	OtrlChatInterface::gone_insecure,
	OtrlChatInterface::still_secure,
	OtrlChatInterface::max_message_size,
	NULL,           /* account_name */
	NULL,           /* account_name_free */
	NULL,           /* received symkey */
	OtrlChatInterface::otr_error_message,
	OtrlChatInterface::otr_error_message_free,
	OtrlChatInterface::resent_msg_prefix,
	OtrlChatInterface::resent_msg_prefix_free,
	OtrlChatInterface::handle_smp_event,
	OtrlChatInterface::handle_msg_event,
	OtrlChatInterface::create_instag,
	NULL,           /* convert_data */
	NULL,           /* convert_data_free */
	OtrlChatInterface::timer_control
};

/*********************** Gui_UI_Ops finished *************************/


/*********************** Constructor/Destructor **********************/

OtrlChatInterface::OtrlChatInterface():
	m_keyGenThread(0)
{
	mSelf = this;
	OTRL_INIT;

	userstate = otrl_userstate_create();
	QString readPath = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "privkeys";
	otrl_privkey_read( userstate, readPath.toLocal8Bit() );

	unsigned int interval = otrl_message_poll_get_default_interval(userstate);
	m_forwardSecrecyTimer.start(interval * 1000);
	QObject::connect(&m_forwardSecrecyTimer, SIGNAL(timeout()), this, SLOT(otrlMessagePoll()));

	readPath = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints";
	otrl_privkey_read_fingerprints(userstate, readPath.toLocal8Bit(), NULL, NULL);

	readPath = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true)) + "instags";
	otrl_instag_read(userstate, readPath.toLocal8Bit());
}

OtrlChatInterface::~ OtrlChatInterface(){
	otrl_userstate_free(userstate);
}

OtrlChatInterface *OtrlChatInterface::self(){
	if( !mSelf ){
		new OtrlChatInterface();
	}
	return mSelf;
}

void OtrlChatInterface::setPlugin( Kopete::Plugin *plugin ){
	chatPlugin = plugin;
}

/********************* Chat section ***************************/

OtrlUserState OtrlChatInterface::getUserstate(){
	return userstate;
}


int OtrlChatInterface::decryptMessage( Kopete::Message &message){

	if (m_blackistIds.contains(message.id())) {
		m_blackistIds.removeAll(message.id());
		return 1;
	}

	Kopete::ChatSession *chatSession = message.manager();
	QString accountId = chatSession->account()->accountId();
	QString protocol = chatSession->protocol()->displayName();
	QString contactId = message.from()->contactId();
	QString body = message.plainBody();

	OtrlTLV *tlvs = NULL;
	char *newMessage = NULL;

	if (m_keyGenThread != 0) {
		// Currently generating the private key... must be a plaintext message anyways...

		if (otrl_proto_message_type(body.toLatin1()) == OTRL_MSGTYPE_DATA) {
			// an OTR message while we are generating the key... cache the message and replay once the key is generated...
			connect(message.manager(), SIGNAL(closing(Kopete::ChatSession*)), SLOT(chatSessionDestroyed(Kopete::ChatSession*)));
			m_storedMessages.append(message);
		}
		return 1;
	}

	int ignoremessage = otrl_message_receiving( userstate, &ui_ops, chatSession, accountId.toLocal8Bit(), protocol.toLocal8Bit(), contactId.toLocal8Bit(), body.toLocal8Bit(), &newMessage, &tlvs, NULL, NULL, NULL );

	ConnContext *context = otrl_context_find( userstate, contactId.toLocal8Bit(), accountId.toLocal8Bit(), protocol.toLocal8Bit(), 0, 0, NULL, NULL, NULL);
	if (context) {
		OtrlTLV *tlv = otrl_tlv_find(tlvs, OTRL_TLV_DISCONNECTED);
		if( tlv ){
			Kopete::Message msg( chatSession->members().first(), chatSession->account()->myself() );
			msg.setHtmlBody( i18n("<b>%1</b> has ended the OTR session. You should do the same.",chatSession->members().first()->contactId() ) );
			msg.setDirection( Kopete::Message::Internal );
			chatSession->appendMessage( msg );
			OtrlChatInterface::self()->emitGoneSecure( chatSession, 3 );

			otrl_tlv_free(tlvs);
		}

	}

	// message is now decrypted or is a Plaintext message and ready to deliver
	if( !ignoremessage ){
		// message is decrypted
		if( newMessage != NULL ){
			body = QString::fromUtf8(newMessage);
			otrl_message_free( newMessage );
			body.replace( QString('\n'), QString("<br>") );
			message.setHtmlBody( body );

			return 0; // message is decrypted and ready to deliver
		} else {
			return 1; // message was a plaintext message. Better not touching it :)
		}
	}
	return 2; // internal OTR message. Ignore it.
}

int OtrlChatInterface::encryptMessage( Kopete::Message &message ){
	char *newMessage = 0;

	QString msgBody;
	bool plaintext = message.format() == Qt::PlainText;

	if(plaintext){
		msgBody = Qt::escape(message.plainBody()).replace('\n', "<br/>");
	} else {
		msgBody = message.escapedBody();
	}

	Kopete::ChatSession *chatSession = message.manager();
	QString accountId = chatSession->account()->accountId();
	QString protocol = chatSession->protocol()->displayName();
	QString contactId = message.to().first()->contactId();

	if( otrl_proto_message_type( msgBody.toLocal8Bit() ) == OTRL_MSGTYPE_NOTOTR ){

		otrl_instag_t instance = message.manager()->property("otr-instag").toUInt();

		int err = otrl_message_sending( userstate, &ui_ops, chatSession, accountId.toLocal8Bit(), protocol.toLocal8Bit(), contactId.toLocal8Bit(), instance, msgBody.toUtf8(), NULL, &newMessage, OTRL_FRAGMENT_SEND_ALL_BUT_LAST, NULL, NULL, NULL );

		if( err != 0 ){
			message.setPlainBody(i18n("An error occurred while encrypting the message."));
			return -1;
		} else if (newMessage) {
			msgBody = QString::fromUtf8(newMessage);
			otrl_message_free(newMessage);
		} else {
			/* Message was not changed */
			return 2;
		}


		OtrlMessageType type = otrl_proto_message_type( msgBody.toLatin1() );
		if( type == OTRL_MSGTYPE_TAGGEDPLAINTEXT ){
			kDebug(14318) << "Tagged plaintext!";

			/* Here we have a problem... libotr tags messages with whitespaces to
			be recognized by other clients. Those whitespaces are discarded
			if we use setHtmlBody() and breaks opportunistic mode.
			If we use setPlainBody() for messages containing tags, those tags
			are escaped and will be visible in the other sides chatwindow.

			This approach should always send out correct messages but will
			break opportunistic mode if the user enables RTF formatting.
			Sorry folks. No way to deal with this currently (Would need changes
			in the rich text handling in libkopete).
			*/
			if(plaintext){
				message.setPlainBody(msgBody);
			} else {
				message.setHtmlBody(msgBody);
			}
			return 1;
		}

		// Always set plaintext if the message has been encrypted.
		// The parser wouldn't understand anything after encryption anyways...
		message.setPlainBody( msgBody );
		message.setType(Kopete::Message::TypeNormal);
		return 0;
	}

	return 2; // Message is still plaintext. Better not touching it
}

QString OtrlChatInterface::getDefaultQuery( const QString &accountId ){
	char *message;
	message = otrl_proto_default_query_msg( accountId.toLatin1(), OTRL_POLICY_ALLOW_V3 | OTRL_POLICY_ALLOW_V2 );
	QString msg( message );
	otrl_message_free( message );
	return msg;
}

void OtrlChatInterface::disconnectSession( Kopete::ChatSession *chatSession ){
	otrl_instag_t instance = chatSession->property("otr-instag").toUInt();
	otrl_message_disconnect( userstate, &ui_ops, chatSession, chatSession->account()->accountId().toLatin1(), chatSession->account()->protocol()->displayName().toLatin1(), chatSession->members().first()->contactId().toLocal8Bit(), instance);
	OtrlChatInterface::self()->emitGoneSecure( chatSession, 0 );

	Kopete::Message msg( chatSession->account()->myself(), chatSession->members().first() );
	msg.setPlainBody( i18n("Terminating OTR session.") );
	msg.setDirection( Kopete::Message::Internal );
	chatSession->appendMessage( msg );
}

bool OtrlChatInterface::shouldDiscard( const QString &message ){
	if( !message.isEmpty() && !message.isNull() ){
		switch( otrl_proto_message_type( message.toLatin1() ) ){
			case OTRL_MSGTYPE_TAGGEDPLAINTEXT:
			//case OTRL_MSGTYPE_UNKNOWN: // Fragmented messages seem to be type UNKNOWN
			case OTRL_MSGTYPE_NOTOTR:
				return false;
			default:
				return true;
		}
	} else {
		return false;
	}
}


void OtrlChatInterface::setPolicy( OtrlPolicy policy ){
	confPolicy = policy;
}


int OtrlChatInterface::privState( Kopete::ChatSession *session ){
	otrl_instag_t instance = session->property("otr-instag").toUInt();
	if (instance == 0) {
		instance = OTRL_INSTAG_BEST;
	}
	ConnContext *context = otrl_context_find(userstate, session->members().first()->contactId().toLocal8Bit(), session->account()->accountId().toLocal8Bit(), session->account()->protocol()->displayName().toLocal8Bit(), instance, 0, NULL, NULL, NULL);

	if( context ){
		switch( context->msgstate ){
		case OTRL_MSGSTATE_PLAINTEXT:
			return 0;
		case OTRL_MSGSTATE_ENCRYPTED:
			if( context->active_fingerprint->trust && context->active_fingerprint->trust[0] != '\0' )
				return 2;
		else
			return 1;
		case OTRL_MSGSTATE_FINISHED:
			return 3;
		}
	}
	return 0;
	}

	QString OtrlChatInterface::formatContact(const QString &contactId){

	Kopete::MetaContact *metaContact = Kopete::ContactList::self()->findMetaContactByContactId(contactId);
	if( metaContact ){
		QString displayName = metaContact->displayName();
		if((displayName != contactId) && !displayName.isNull()){
			return displayName + " (" + contactId + ')';
		}
	}
	return contactId;
}

void OtrlChatInterface::verifyFingerprint( Kopete::ChatSession *session ){
	ConnContext *context;

	otrl_instag_t instance = session->property("otr-instag").toUInt();
	context = otrl_context_find( userstate, session->members().first()->contactId().toLocal8Bit(), session->account()->accountId().toLocal8Bit(), session->protocol()->displayName().toLocal8Bit(), instance, 0, NULL, NULL, NULL);

	new AuthenticationWizard( session->view()->mainWidget(), context, session, true );
}

Fingerprint *OtrlChatInterface::findFingerprint( Kopete::ChatSession *session ){
	ConnContext *context;


	otrl_instag_t instance = session->property("otr-instag").toUInt();
	if (instance == 0) {
		instance = OTRL_INSTAG_BEST;
	}
	context = otrl_context_find( userstate, session->members().first()->contactId().toLocal8Bit(), session->account()->accountId().toLocal8Bit(), session->protocol()->displayName().toLocal8Bit(), instance, 0, NULL, NULL, NULL);
	if (context && context->active_fingerprint && context->active_fingerprint->fingerprint) {
		return context->active_fingerprint;
	}
	return NULL;
}

QString OtrlChatInterface::fingerprint( Kopete::ChatSession *session ){
	char hash[45];
	memset(hash, 0, 45);

	Fingerprint *fp = findFingerprint(session);
	if (fp) {
		otrl_privkey_hash_to_human( hash, fp->fingerprint );
		return QLatin1String(hash);
	}

	return QString();
	}

	bool OtrlChatInterface::isVerified( Kopete::ChatSession *session ){
	Fingerprint *fingerprint = findFingerprint( session );

	if( fingerprint->trust && fingerprint->trust[0] != '\0' ){
		return true;
	} else {
		return false;
	}
}

void OtrlChatInterface::checkFilePermissions( const QString &file ){
	if( QFile::exists( file ) ){
		QFile privkeys( file );
		QFileInfo privkeysInfo( privkeys );
		if( !privkeysInfo.permission( QFile::ReadOwner | QFile::WriteOwner ) |
			privkeysInfo.permission( QFile::ReadGroup ) |
			privkeysInfo.permission( QFile::WriteGroup ) |
			privkeysInfo.permission( QFile::ExeGroup ) |
			privkeysInfo.permission( QFile::ReadOther ) |
			privkeysInfo.permission( QFile::WriteOther ) |
			privkeysInfo.permission( QFile::ExeOther ) ){
		chmod( file.toLocal8Bit(), 0600);
		}
	}

}

void OtrlChatInterface::emitGoneSecure( Kopete::ChatSession *session, int state ){
	emit goneSecure( session, state );
}

void OtrlChatInterface::setTrust( Kopete::ChatSession *session, bool trust ){
	Fingerprint *fingerprint;

	fingerprint = findFingerprint( session );
	if( fingerprint != 0 ){
		if( trust ){
			otrl_context_set_trust( fingerprint, "verified" );
		} else {
			otrl_context_set_trust( fingerprint, NULL );
		}
		kDebug(14318) << "Writing fingerprints";
		otrl_privkey_write_fingerprints( userstate, QString( QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints" ).toLocal8Bit() );
		emitGoneSecure( session, privState( session ) );
	} else {
		kDebug(14318) << "could not find fingerprint";
	}
}

void OtrlChatInterface::generatePrivateKey(const QString &account, const QString &protocol)
{
	m_keyGenThread = new KeyGenThread ( account.toLatin1(), protocol.toLatin1() );
	m_keyGenThread->start();
	while( !m_keyGenThread->wait(100) ){
		qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers, 100);
	}
	m_keyGenThread->deleteLater();
	m_keyGenThread = 0;
}

/****************** SMP implementations ****************/

void OtrlChatInterface::abortSMP( ConnContext *context, Kopete::ChatSession *session ){
	otrl_message_abort_smp( userstate, &ui_ops, session, context);
	if (context->active_fingerprint->trust && !context->active_fingerprint->trust[0]) {
		emitGoneSecure( session, 1 );

		Kopete::Message msg( session->members().first(), session->account()->myself() );
		msg.setHtmlBody( i18n("<b>Authentication aborted. Note that the conversation is now insecure.</b>") );
		msg.setDirection( Kopete::Message::Internal );
		session->appendMessage( msg );
	}
}

void OtrlChatInterface::initSMP( ConnContext *context, Kopete::ChatSession *session, const QString &secret){
	otrl_message_initiate_smp( userstate, &ui_ops, session, context, (unsigned char*)secret.toLocal8Bit().data(), secret.length() );

	Kopete::Message msg( session->members().first(), session->account()->myself() );
	msg.setHtmlBody( i18n("<b>Authenticating contact...</b>") );
	msg.setDirection( Kopete::Message::Internal );

	session->appendMessage( msg );
}

void OtrlChatInterface::initSMPQ( ConnContext *context, Kopete::ChatSession *session, const QString &question, const QString &secret){
	otrl_message_initiate_smp_q( userstate, &ui_ops, session, context, (const char*)question.toLocal8Bit().data(), (unsigned char*)secret.toLocal8Bit().data(), secret.length() );

	Kopete::Message msg( session->members().first(), session->account()->myself() );
	msg.setHtmlBody( i18n("<b>Authenticating contact...</b>") );
	msg.setDirection( Kopete::Message::Internal );

	session->appendMessage( msg );
}

void OtrlChatInterface::respondSMP( ConnContext *context, Kopete::ChatSession *session, const QString &secret ){

	kDebug(14318) << "resonding SMP";

	otrl_message_respond_smp( userstate, &ui_ops, session, context, (unsigned char*)secret.toLocal8Bit().data(), secret.length());

	Kopete::Message msg( session->members().first(), session->account()->myself() );
	msg.setHtmlBody( i18n("<b>Authenticating contact...</b>") );
	msg.setDirection( Kopete::Message::Internal );

	session->appendMessage( msg );
}

void OtrlChatInterface::otrlMessagePoll()
{
    otrl_message_poll(userstate, 0, 0);
}

void OtrlChatInterface::replayStoredMessages()
{
    while (m_storedMessages.isEmpty()) {
        Kopete::Message msg = m_storedMessages.takeFirst();
        msg.manager()->appendMessage(msg);
    }
}

void OtrlChatInterface::chatSessionDestroyed(Kopete::ChatSession *chatSession)
{
    QList<Kopete::Message> tmpList;
    foreach (const Kopete::Message &msg, m_storedMessages) {
        if (msg.manager() != chatSession) {
            tmpList.append(msg);
        }
    }
    m_storedMessages = tmpList;
}
/****************** KeyGenThread *******************/

KeyGenThread::KeyGenThread( const QString &accountname, const QString &protocol ){
	this->accountname = accountname;
	this->protocol = protocol;
}


void KeyGenThread::run()
{
	QString storeFile = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "privkeys";
	otrl_privkey_generate(OtrlChatInterface::self()->getUserstate(), storeFile.toLocal8Bit(), accountname.toLocal8Bit(), protocol.toLocal8Bit());
	OtrlChatInterface::self()->checkFilePermissions( QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "privkeys" );
}

