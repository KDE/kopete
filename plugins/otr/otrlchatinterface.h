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

#ifndef OTRLCHATINTERFACE_H
#define OTRLCHATINTERFACE_H

/**
  * @author Michael Zanetti
  */

#include <qstring.h>
#include <qthread.h>
#include <qmutex.h>
#include <qmetatype.h>
#include <qtimer.h>

#include <kopete_export.h>
#include <kopetechatsession.h>
#include <kopeteplugin.h>

#include "authenticationwizard.h"

extern "C" {
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}

Q_DECLARE_METATYPE(otrl_instag_t)

class KeyGenThread;

class KOPETE_OTR_SHARED_EXPORT OtrlChatInterface: public QObject
{
	Q_OBJECT
public:
	~OtrlChatInterface();
	static OtrlChatInterface *self();

	int decryptMessage( Kopete::Message &message, bool allowNonOtr=false, bool autoEnd=false);
	int encryptMessage( Kopete::Message &message );
	QString getDefaultQuery( const QString &accountId );
	void disconnectSession( Kopete::ChatSession *chatSession );
	void setPolicy( OtrlPolicy policy );
	bool shouldDiscard( const QString &message );
	OtrlUserState getUserstate();
	int privState( Kopete::ChatSession *session );
	bool isVerified( Kopete::ChatSession *session );
	void checkFilePermissions( const QString &file );
	QString fingerprint( Kopete::ChatSession *session );
	void verifyFingerprint( Kopete::ChatSession *session );
	void setPlugin(Kopete::Plugin *plugin);
	void emitGoneSecure(Kopete::ChatSession *sesseion, int state);
	void abortSMP( ConnContext *context, Kopete::ChatSession *session );
	void initSMP( ConnContext *context, Kopete::ChatSession *session, const QString &secret );
	void initSMPQ( ConnContext *context, Kopete::ChatSession *session, const QString &question, const QString &secret );
	void respondSMP( ConnContext *context, Kopete::ChatSession *session, const QString &secret );
	void setTrust( Kopete::ChatSession *session, bool trust );
	void generatePrivateKey(const QString &account, const QString &protocol);

	static QString formatContact( const QString &contactId);
private:
	OtrlChatInterface();
	static OtrlChatInterface *mSelf;
	Fingerprint *findFingerprint( Kopete::ChatSession *session );
	QList<uint> m_blackistIds;
	KeyGenThread *m_keyGenThread;
	QTimer m_forwardSecrecyTimer;
	QList<Kopete::Message> m_storedMessages;

	static OtrlMessageAppOps ui_ops;
	static OtrlPolicy policy(void *opdata, ConnContext *context);
	static void create_privkey(void *opdata, const char *accountname, const char *protocol);
	static int is_logged_in(void *opdata, const char *accountname, const char *protocol, const char *recipient);
	static void inject_message( void *opdata, const char *accountname, const char *protocol, const char *recipient, const char *message );
	static void update_context_list(void *opdata);
	static void new_fingerprint(void *opdata, OtrlUserState us, const char *accountname, const char *protocol, const char *username, unsigned char fingerprint[20]);
	static void write_fingerprints(void *opdata);
	static void gone_secure(void *opdata, ConnContext *context);
	static void gone_insecure(void *opdata, ConnContext *context);
	static void still_secure(void *opdata, ConnContext *context, int is_reply);
	static int max_message_size(void *opdata, ConnContext *context);
	static const char* otr_error_message(void *opdata, ConnContext *context, OtrlErrorCode err_code);
	static void otr_error_message_free(void *opdata, const char *err_msg);
	static const char *resent_msg_prefix(void *opdata, ConnContext *context);
	static void resent_msg_prefix_free(void *opdata, const char *prefix);
	static void handle_msg_event(void *opdata, OtrlMessageEvent msg_event, ConnContext *context, const char* message, gcry_error_t err);
	static void handle_smp_event(void *opdata, OtrlSMPEvent smp_event, ConnContext *context, unsigned short progress_percent, char *question);
	static void create_instag(void *opdata, const char *accountname, const char *protocol);
	static void timer_control(void *opdata, unsigned int interval);

private slots:
	void otrlMessagePoll();
	void replayStoredMessages();
	void chatSessionDestroyed(Kopete::ChatSession *chatSession);

signals:
	void goneSecure(Kopete::ChatSession* session, int state);
};

class KeyGenThread : public QThread {

private:
	QString accountname;
	QString protocol;

public:
	KeyGenThread( const QString &accountname, const QString &protocol );
	virtual void run();
};

#endif
