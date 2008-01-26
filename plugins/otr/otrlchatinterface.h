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

#ifndef OTRLCHATINTERFACE_H
#define OTRLCHATINTERFACE_H

/**
  * @author Michael Zanetti
  */

#include <qstring.h>
#include <q3ptrlist.h>
#include <q3valuelist.h>
#include <qthread.h>
#include <qmutex.h>

#include <kopetechatsession.h>
#include <kopeteplugin.h>

#include "smppopup.h"
//#include "otrplugin.h"

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

	int decryptMessage( QString *msg, QString accountId, QString protocol, QString contactId, Kopete::ChatSession *chatSession );
	QString encryptMessage( QString msg, QString accountId,
	QString protocol, QString contactId , Kopete::ChatSession *chatSession );
	QString getDefaultQuery( QString accountId );
	void disconnectSession( Kopete::ChatSession *chatSession );
	void setPolicy( OtrlPolicy policy );
	bool shouldDiscard( QString message );
	OtrlUserState getUserstate();
	int privState( Kopete::ChatSession *session );
	QString formatContact(QString contactId);
	bool isVerified( Kopete::ChatSession *session );
	void updateKeyfile( Kopete::Account *account );
	void checkFilePermissions( QString file );
	QString findActiveFingerprint( Kopete::ChatSession *session );
	void verifyFingerprint( Kopete::ChatSession *session );
	void setPlugin(Kopete::Plugin *plugin);
	void emitGoneSecure(Kopete::ChatSession *sesseion, int state);
	void abortSMP( ConnContext *context, Kopete::ChatSession *session );
	void respondSMP( ConnContext *context, Kopete::ChatSession *session, QString secret, bool initiate );
	void setTrust( Kopete::ChatSession *session, bool trust );

private:
	OtrlChatInterface();
	static OtrlChatInterface *mSelf;
	Fingerprint *findFingerprint( QString username );

signals:
	void goneSecure(Kopete::ChatSession* session, int state);
};

 class KeyGenThread : public QThread {

private:
	QString accountname;
	QString protocol;

public:
	KeyGenThread( QString accountname, QString protocol );
	virtual void run();
};

#endif
