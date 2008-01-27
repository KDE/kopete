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

#ifndef OTRLCONFINTERFACE_H
#define OTRLCONFINTERFACE_H

/**
  * @author Michael Zanetti
  */

#include <qstring.h>
#include <qthread.h>

#include <kopetechatsession.h>

extern "C" {
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
#include <libotr/context.h>
}

class OtrlConfInterface : public QObject
{
	Q_OBJECT
public:
	~OtrlConfInterface();
	OtrlConfInterface( QWidget *preferencesDialog );

	QString getPrivFingerprint( QString accountId, QString protocol );
	void generateNewPrivKey( QString accountId, QString protocol );
	QList<QStringList> readAllFingerprints();
	bool hasPrivFingerprint( QString accountId, QString protocol);
	void forgetFingerprint( QString strFingerprint );
	void verifyFingerprint( QString strFingerprint, bool trust );
	bool isVerified( QString strFingerprint );
	bool isEncrypted( QString strFingerprint );

private:
	OtrlUserState userstate;
	QWidget *preferencesDialog;

	Fingerprint *findFingerprint( QString strFingerprint );	
};

#endif
