/***************************************************************************
 *   Copyright (C) 2007 by Michael Zanetti   
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


/**
  * @author Michael Zanetti
  */

#include <sys/types.h>
#include <sys/stat.h>

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
//#include <kprogress.h>
#include <kpassivepopup.h>
//#include <kanimwidget.h>
#include <kpushbutton.h>

#include <q3vbox.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qeventloop.h>
#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <q3ptrlist.h>
#include <qwidget.h>

#include "otrlchatinterface.h"
#include "otrguiclient.h"
#include "otrplugin.h"
//#include "privkeypopup.h"

OtrlChatInterface *OtrlChatInterface::mSelf = 0;
static OtrlUserState userstate;
static OtrlPolicy confPolicy;
static void *updateContextList = 0;

/***************************** Gui_UI_Ops for libotr **********************************/
static OtrlPolicy policy(void *opdata, ConnContext *context){
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	bool noerr;
	
	// Disable OTR for IRC
	if( session->protocol()->pluginId() == "IRCProtocol" ){
//		kdDebug() << "Disabling OTR for: " << session->protocol()->pluginId() << endl;
		return OTRL_POLICY_NEVER;
	}
#warning Port me! Determine user specific policy!
//	QString policy = session->members().first()->metaContact()->pluginData( OTRPlugin::plugin(), "otr_policy" );
	QString policy = "2";
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

static void create_privkey(void *opdata, const char *accountname, const char *protocol){

	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);

//	PrivKeyPopup *popup = new PrivKeyPopup( session->view()->mainWidget(), i18n("Generating private key"), Qt::Popup | Qt::WindowStaysOnTopHint );
#warning Port me!
//	PrivKeyPopup *popup = new PrivKeyPopup( session->view()->mainWidget() );
//	KAnimWidget *anim = new KAnimWidget( "kde", 72, popup->animFrame, "kopete" );
//	anim->start();
//	anim->show();

//	popup->setCloseLock( true );
//	popup->show();
	KMessageBox::information(session->view()->mainWidget(), "Bla" );
	KeyGenThread *keyGenThread = new KeyGenThread( accountname, protocol );
	keyGenThread->start();
	while( !keyGenThread->wait(100) ){
//		qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput | QEventLoop::ExcludeSocketNotifiers, 100);
//		qApp->x11ProcessEvent(); 
	}
//	popup->setCloseLock( false );
//	popup->close();
}

static int is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient){
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::ContactPtrList list = session->members();		
/*	for ( Q3PtrListIterator<Kopete::Contact> it( list ); Kopete::Contact *contact = it.current(); ++it ){
		if( contact->contactId().compare( recipient ) == 0 ){
			Kopete::OnlineStatus status = session->contactOnlineStatus( contact );
			if( status == Kopete::OnlineStatus::Unknown){
				return -1;
			} else if( status == Kopete::OnlineStatus::Offline ){
				return 0;
			} else {
				return 1;
			}
		}
	}
*/	for( int i = 0; i < list.size(); i++ ){
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

static void inject_message( void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message ){
	//KMessageBox::information( NULL, QString(accountname) + ":" + QString(protocol) + ":" + QString(recipient) + ":" + QString(message) );
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

static void notify(void *opdata, OtrlNotifyLevel level, const char *accountname, const char *protocol, const char *username, const char *title, const char *primary, const char *secondary){
	KMessageBox::information(NULL, QString( primary ) + QString( secondary ), QString( title ) );
}

static int display_otr_message( void *opdata, const char *accountname, const char *protocol, const char *username, const char *message ){
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::ContactPtrList list = session->members();		
	for( int i = 0; i < list.size(); i++ ){
		if( list.at(i)->contactId().compare( username ) == 0 ){
			Kopete::Message msg( session->members().first(), session->account()->myself() );
			msg.setPlainBody( QString( message ) );
			msg.setDirection( Kopete::Message::Internal );
			session->appendMessage( msg );
			return 0;
		}
	}
	return 1;
}

static void update_context_list(void *opdata){
//Not used...
}

static const char *protocol_name(void *opdata, const char *protocol){
//Never seen...
//	kdDebug() << "protocol_name called" << endl;
}

static void protocol_name_free(void *opdata, const char *protocol_name){
//Never seen...
//	kdDebug() << "protocol_name_free called" << endl;
}

static void new_fingerprint(void *opdata, OtrlUserState us, const char *accountname, const char *protocol, const char *username, unsigned char fingerprint[20]){
//	kdDebug() << "Received a new Fingerprint" << endl;
	char human[45];
	otrl_privkey_hash_to_human( human, fingerprint );
	ConnContext *context = otrl_context_find( us, username, accountname, protocol, 0, NULL, NULL, NULL);
	Fingerprint *fpPointer = otrl_context_find_fingerprint( context, fingerprint, 0, NULL );
	if( OtrlChatInterface::self()->verifyQuestion( ((Kopete::ChatSession*)opdata), human ) ){
		otrl_context_set_trust( fpPointer, "verified" );
	} else {
		otrl_context_set_trust( fpPointer, NULL );
	}
}

static void write_fingerprints(void *opdata){
//	kdDebug() << "Writing fingerprints" << endl;
	QString savePath = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints";
	otrl_privkey_write_fingerprints( userstate, savePath.toLocal8Bit() );
}

static void gone_secure(void *opdata, ConnContext *context){
//	kdDebug() << "gone secure" << endl;
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);

	if( context->active_fingerprint->trust && context->active_fingerprint->trust[0] ){
		Kopete::Message msg( session->members().first(), session->account()->myself() );
		msg.setPlainBody( i18n("<b>Private OTR session started.</b>") );
		msg.setDirection( Kopete::Message::Internal );
		session->appendMessage( msg );
#warning Port me! Tell otr-plugin the new security state!
//		OTRPlugin::plugin()->emitGoneSecure( ((Kopete::ChatSession*)opdata), 2 );
	} else {
		Kopete::Message msg( session->members().first(), session->account()->myself() );
		msg.setPlainBody( i18n("<b>Unverified OTR session started.</b>") );
		msg.setDirection( Kopete::Message::Internal );
		session->appendMessage( msg );
#warning Port me! Tell otr-plugin the new security state!
//		OTRPlugin::plugin()->emitGoneSecure( ((Kopete::ChatSession*)opdata), 1 );
	}
}

/* Actually I've never seen this event but its implemented in case someone should receive it 
   kopete, gaim and miranda send a heartbeat message at disconnect. See log_message.
   Searching libotr I could not find any call of gone_insecure. */
static void gone_insecure(void *opdata, ConnContext *context){
//	kdDebug() << "gone insecure" << endl;
#warning Port me! Tell otr-plugin the new security state!
//	OTRPlugin::plugin()->emitGoneSecure(((Kopete::ChatSession*)opdata), 0);
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::Message msg( session->members().first(), session->account()->myself() );
	msg.setPlainBody( i18n("<b>OTR Session ended. The conversation is now insecure!</b>") );
	msg.setDirection( Kopete::Message::Internal );
	session->appendMessage( msg );
}

static void still_secure(void *opdata, ConnContext *context, int is_reply){
//	kdDebug() << "still secure" << endl;
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::Message msg( session->members().first(), session->account()->myself() );
	msg.setPlainBody( i18n("<b>OTR connection refreshed successfully.</b>") );
	msg.setDirection( Kopete::Message::Internal );
	session->appendMessage( msg );

	if( context->active_fingerprint->trust && context->active_fingerprint->trust[0] ){
#warning Port me! Tell otr-plugin the new security state!
//		OTRPlugin::plugin()->emitGoneSecure( session, 2);
	} else {
#warning Port me! Tell otr-plugin the new security state!
//		OTRPlugin::plugin()->emitGoneSecure( session, 1);
	}
}

static void log_message(void *opdata, const char *message){
//	kdDebug() << "libotr: "<< message << endl;
// setting gui icon to disconnected because libotr sends a heartbeat message when calling otrl_context_disconnect()
// and this doesn't call gone_insecure... I also have never seen a heartbeat message not generated while disconnect.
	Kopete::ChatSession *session= ((Kopete::ChatSession*)opdata);
	Kopete::Message msg( session->members().first(), session->account()->myself()  );
	msg.setPlainBody( i18n("<b>%1</b> has ended the OTR session. You should do the same.").arg(session->members().first()->contactId()) );
	msg.setDirection( Kopete::Message::Internal );
	session->appendMessage( msg );

#warning Port me! Tell otr-plugin the new security state!
//	OTRPlugin::plugin()->emitGoneSecure(((Kopete::ChatSession*)opdata), 3);
}

static OtrlMessageAppOps ui_ops = {
	policy,
	create_privkey,
	is_logged_in,
	inject_message,
	notify,
	display_otr_message,
	update_context_list,
	protocol_name,
	protocol_name_free,
	new_fingerprint,
	write_fingerprints,
	gone_secure,
	gone_insecure,
	still_secure,
	log_message
};

/*********************** Gui_UI_Ops finished *************************/


/*********************** Constructor/Destructor **********************/

OtrlChatInterface::OtrlChatInterface(){
//	kdDebug() << "Creating OtrlChatInterface" << endl;
	mSelf = this;
	OTRL_INIT;	

	userstate = otrl_userstate_create();
	QString readPath = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "privkeys";
	otrl_privkey_read( userstate, readPath.toLocal8Bit() );
	
	
	QString savePath = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints";
	otrl_privkey_read_fingerprints(userstate, savePath.toLocal8Bit(), NULL, NULL);

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

/********************* Chat section ***************************/

OtrlUserState OtrlChatInterface::getUserstate(){
	return userstate;
}


int OtrlChatInterface::decryptMessage( QString *msg, QString accountId,
	QString protocol, QString contactId , Kopete::ChatSession *chatSession){

	int ignoremessage;
	char *newMessage = NULL;
	OtrlTLV *tlvs = NULL;

	ignoremessage = otrl_message_receiving( userstate, &ui_ops, chatSession, accountId.toLocal8Bit(), protocol.toLocal8Bit(), contactId.toLocal8Bit(), msg->toLocal8Bit(), &newMessage, &tlvs, NULL, NULL );
	// message is now decrypted or is a Plaintext message and ready to deliver
	if( !ignoremessage ){
		// message is decrypted
		if( newMessage != NULL ){
			*msg = QString::fromUtf8(newMessage);
			otrl_message_free( newMessage );
			msg->replace( QString('\n'), QString("<br>") );
		}
	}
	return ignoremessage;
}

QString OtrlChatInterface::encryptMessage( QString msg, QString accountId,
	QString protocol, QString contactId , Kopete::ChatSession *chatSession ){
	int err;
	char * newMessage;
	if( otrl_proto_message_type( msg.toLocal8Bit() ) == OTRL_MSGTYPE_NOTOTR ){
		msg.replace( QString('<'), QString("&lt;") );
		err = otrl_message_sending( userstate, &ui_ops, chatSession, accountId.toLocal8Bit(), protocol.toLocal8Bit(), contactId.toLocal8Bit(), msg.toUtf8(), NULL, &newMessage, NULL, NULL );
	
		if( err != 0 ){
			msg = i18n("Encryption error");
		} else {
			if( newMessage != NULL ){
				msg = QString::fromUtf8( newMessage );
				otrl_message_free( newMessage );
			}
		}
	}
	OtrlMessageType type = otrl_proto_message_type( msg.toLocal8Bit() );
	if( type == OTRL_MSGTYPE_NOTOTR | type == OTRL_MSGTYPE_TAGGEDPLAINTEXT ){
		msg.replace( QString("&lt;"), QString("<") );		
	}
	return msg;
}

QString OtrlChatInterface::getDefaultQuery( QString accountId ){
	char *message;
	message = otrl_proto_default_query_msg( accountId.toLatin1(), OTRL_POLICY_ALLOW_V2 );
	QString msg( message );
	otrl_message_free( message );
	return msg;
}

void OtrlChatInterface::disconnectSession( Kopete::ChatSession *chatSession ){
	otrl_message_disconnect( userstate, &ui_ops, chatSession, chatSession->account()->accountId().toLatin1(), chatSession->account()->protocol()->displayName().toLatin1(), chatSession->members().first()->contactId().toLocal8Bit() );
#warning Port me! Tell otr-plugin the new security state!
//	OTRPlugin::plugin()->emitGoneSecure( chatSession, false );

	Kopete::Message msg( chatSession->account()->myself(), chatSession->members().first() );
	msg.setPlainBody( i18n("Terminating OTR session.") );
	msg.setDirection( Kopete::Message::Internal );
//	msg.setBody( QString( message ), Kopete::Message::RichText );
	chatSession->appendMessage( msg );

}

bool OtrlChatInterface::shouldDiscard( QString message ){
	if( !message.isEmpty() && !message.isNull() ){
		switch( otrl_proto_message_type( message.toLatin1() ) ){
			case OTRL_MSGTYPE_TAGGEDPLAINTEXT:
			case OTRL_MSGTYPE_UNKNOWN:
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
	ConnContext *context;
	
	context = otrl_context_find(userstate, session->members().first()->contactId().toLocal8Bit(), session->account()->accountId().toLocal8Bit(), session->account()->protocol()->displayName().toLocal8Bit(), 0, NULL, NULL, NULL);

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

QString  OtrlChatInterface::formatContact(QString contactId){
	
	Kopete::MetaContact *metaContact = Kopete::ContactList::self()->findMetaContactByContactId(contactId);
	if( metaContact ){
		QString displayName = metaContact->displayName();
		if((displayName != contactId) && !displayName.isNull()){
			return displayName + " (" + contactId+")"; 
		}
	}
	return contactId;
}

void OtrlChatInterface::verifyFingerprint( Kopete::ChatSession *session, bool trust ){
	Fingerprint *fingerprint;

	fingerprint = findFingerprint( session->members().first()->contactId() );
	if( fingerprint != 0 ){
		if( trust ){
			otrl_context_set_trust( fingerprint, "verified" );
		} else {
			otrl_context_set_trust( fingerprint, NULL );
		}
//		kdDebug() << "Writing fingerprints" << endl;
		QString savePath = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints";
		otrl_privkey_write_fingerprints( userstate, savePath.toLocal8Bit() );
#warning Port me! Tell otr-plugin the new security state!
//		OTRPlugin::plugin()->emitGoneSecure( session, privState( session ) );
	} else {
//		kdDebug() << "could not find fingerprint" << endl;
	}

}

Fingerprint *OtrlChatInterface::findFingerprint( QString account ){
	ConnContext *context;

	for( context = userstate->context_root; context != NULL; context = context->next ){
//		kdDebug() << context->username << endl;
		if( strcmp( context->username, account.toLocal8Bit() ) == 0 ){
//			kdDebug() << "found Context" << endl;
			return context->active_fingerprint ? context->active_fingerprint : NULL;
		}
	}
	return NULL;
}

QString OtrlChatInterface::findActiveFingerprint( Kopete::ChatSession *session ){
	ConnContext *context;
	char hash[45];

	for( context = userstate->context_root; context != NULL; context = context->next ){
//		kdDebug() << context->username << endl;
		if( strcmp( context->username, session->members().first()->contactId().toLocal8Bit() ) == 0 ){
//			otrl_privkey_hash_to_human( hash, context->fingerprint_root.next->fingerprint );
			otrl_privkey_hash_to_human( hash, context->active_fingerprint->fingerprint );
			return hash;
		}
	}
	return NULL;
}

bool OtrlChatInterface::isVerified( Kopete::ChatSession *session ){
//	kdDebug() << "checking for trust" << endl;
	Fingerprint *fingerprint = findFingerprint( session->members().first()->contactId() );

	if( fingerprint->trust && fingerprint->trust[0] != '\0' ){
//		kdDebug() << "verified" << endl;
		return true;
	} else {
//		kdDebug() << "not verified" << endl;
		return false;
	}
}

void OtrlChatInterface::updateKeyfile( Kopete::Account *account ){
// Updating private keys from <=0.3
//	kdDebug() << "updating keys" << endl;
	QFile keyfile( QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "privkeys" );
	QString line;
	char *lineChar;
	QString file;

	if( keyfile.open( QIODevice::ReadWrite ) ){
//		kdDebug() << "file open" << endl;
		while( keyfile.readLine( lineChar, 200 ) != -1){
			line = QString::fromLocal8Bit(lineChar);
			if( line.contains( "protocol" ) != -1 ){
				if( line.contains( account->accountLabel() ) != -1 ){
					line.replace( account->accountLabel(), account->protocol()->displayName() );
//					kdDebug() << "Successfully updated keyfile for account " << account->accountId() << endl;
				}
			}
		file.append( line );
		}
	}
	keyfile.remove();
	keyfile.open( QIODevice::ReadWrite );
	keyfile.write( file.toLatin1() );
	keyfile.close();
	otrl_privkey_forget_all( userstate );
	otrl_privkey_read( userstate, QString((KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "privkeys").toLatin1() );

	file = "";
	line = "";
// Updating fingerprints from <=0.3
	kDebug() << "updating fingerprints";
	QFile fingerprintfile( QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints" );

	if( fingerprintfile.open( QIODevice::ReadWrite ) ){
		kDebug() << "file open";
		line = fingerprintfile.readLine( 200 );
		while( !line.isEmpty() ){
			int pos = line.indexOf( account->accountLabel() );
			if( pos != -1 ){
				line.replace( pos, account->accountLabel().length(), account->protocol()->displayName() );
				kdDebug() << "Successfully updated fingerprint for account " << account->accountId() << endl;
			}
			line = fingerprintfile.readLine( 200 );
		file.append( line );
		}
	}
	fingerprintfile.remove();
	fingerprintfile.open( QIODevice::ReadWrite );
	fingerprintfile.write( file.toLatin1() );
	fingerprintfile.close();
	otrl_context_forget_all( userstate );
	otrl_privkey_read_fingerprints(userstate, QString((KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "fingerprints").toLatin1(), NULL, NULL);	

}

void OtrlChatInterface::checkFilePermissions( QString file ){
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
//			kdDebug() << "Permissions of OTR storage file are wrong! Correcting..." << endl;
			chmod( file.toLocal8Bit(), 0600);	
		}
	}

}

bool OtrlChatInterface::verifyQuestion( Kopete::ChatSession *session, QString fingerprint ){
//	kdDebug() << "searching for Fingerprint" << endl;

	if( fingerprint != NULL ){
		int doVerify = KMessageBox::questionYesNo( 
			NULL, 
			i18n("Please contact %1 via another secure way and verify that the following Fingerprint is correct:").arg( formatContact(session->members().first()->contactId())) + "\n\n" + fingerprint + "\n\n" + i18n("Are you sure you want to trust this fingerprint?"),
			i18n("Verify fingerprint")  );
		if( doVerify == KMessageBox::Yes ){
			return true;
		} else {
			return false;
			verifyFingerprint( session, false );
		}
	} else {
		KMessageBox::error( NULL, i18n( "No fingerprint yet received from this contact." ), i18n( "No fingerprint found" ) );
	}
	return false;	
}


/****************** KeyGenThread *******************/

KeyGenThread::KeyGenThread( QString accountname, QString protocol ){
	this->accountname = accountname;
	this->protocol = protocol;
}


void KeyGenThread::run()
{
//	kdDebug() << "Creating private key... Storing to: " + QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true)) + "privkeys" << endl;
	QString storeFile = QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "privkeys";
	otrl_privkey_generate(OtrlChatInterface::self()->getUserstate(), storeFile.toLocal8Bit(), accountname.toLocal8Bit(), protocol.toLocal8Bit());
	OtrlChatInterface::self()->checkFilePermissions( QString(KGlobal::dirs()->saveLocation("data", "kopete_otr/", true )) + "privkeys" );
}

