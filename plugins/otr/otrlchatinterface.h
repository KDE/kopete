/*************************************************************************
 * Copyright <2007>  <Michael Zanetti> <michael_zanetti@gmx.net>         *
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

#include <kopetechatsession.h>
#include <kopeteplugin.h>

#include "authenticationwizard.h"

extern "C" {
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
}


class OtrlChatInterface: public QObject
{
	Q_OBJECT
public:
	~OtrlChatInterface();
	static OtrlChatInterface *self();

	int decryptMessage( QString *msg, const QString &accountId, const QString &protocol, const QString &contactId, Kopete::ChatSession *chatSession );
	QString *encryptMessage( QString *msg, const QString &accountId,
	const QString &protocol, const QString &contactId , Kopete::ChatSession *chatSession );
	QString getDefaultQuery( const QString &accountId );
	void disconnectSession( Kopete::ChatSession *chatSession );
	void setPolicy( OtrlPolicy policy );
	bool shouldDiscard( const QString &message );
	OtrlUserState getUserstate();
	int privState( Kopete::ChatSession *session );
	QString formatContact( const QString &contactId);
	bool isVerified( Kopete::ChatSession *session );
	void checkFilePermissions( const QString &file );
	QString findActiveFingerprint( Kopete::ChatSession *session );
	void verifyFingerprint( Kopete::ChatSession *session );
	void setPlugin(Kopete::Plugin *plugin);
	void emitGoneSecure(Kopete::ChatSession *sesseion, int state);
	void abortSMP( ConnContext *context, Kopete::ChatSession *session );
	void initSMP( ConnContext *context, Kopete::ChatSession *session, const QString &secret );
	void initSMPQ( ConnContext *context, Kopete::ChatSession *session, const QString &question, const QString &secret );
	void respondSMP( ConnContext *context, Kopete::ChatSession *session, const QString &secret );
	void setTrust( Kopete::ChatSession *session, bool trust );

private:
	OtrlChatInterface();
	static OtrlChatInterface *mSelf;
	Fingerprint *findFingerprint( const QString &username );

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
