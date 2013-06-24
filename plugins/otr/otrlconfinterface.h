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

#ifndef OTRLCONFINTERFACE_H
#define OTRLCONFINTERFACE_H

/**
  * @author Michael Zanetti
  */

#include <qstring.h>
#include <qthread.h>

#include <kopete_export.h>
#include <kopetechatsession.h>

extern "C" {
#include <libotr/privkey.h>
#include <libotr/proto.h>
#include <libotr/message.h>
#include <libotr/userstate.h>
#include <libotr/context.h>
}

class KOPETE_OTR_SHARED_EXPORT OtrlConfInterface : public QObject
{
	Q_OBJECT
public:
	~OtrlConfInterface();
	OtrlConfInterface( QWidget *preferencesDialog );

	QString getPrivFingerprint( const QString &accountId, const QString &protocol );
	void generateNewPrivKey( const QString &accountId, const QString &protocol );
	QList<QStringList> readAllFingerprints();
	bool hasPrivFingerprint( const QString &accountId, const QString &protocol);
	void forgetFingerprint( const QString &strFingerprint );
	void verifyFingerprint( const QString &strFingerprint, bool trust );
	bool isVerified( const QString &strFingerprint );
	bool isEncrypted( const QString &strFingerprint );

private:
	OtrlUserState userstate;
	QWidget *preferencesDialog;

	Fingerprint *findFingerprint( const QString &strFingerprint );	
};

#endif
