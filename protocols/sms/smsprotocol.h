/*  *************************************************************************
    *   copyright: (C) 2003 Richard Lärkäng <nouseforaname@home.se>         *
    *   copyright: (C) 2003 Gav Wood <gav@kde.org>                          *
    *************************************************************************
*/

/*  *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef SMSPROTOCOL_H
#define SMSPROTOCOL_H

#include <qmap.h>
#include <qmovie.h>
#include <qpixmap.h>
#include <qptrdict.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include "kopeteprotocol.h"
#include "kopeteonlinestatus.h"
#include "kopetecontact.h"

class KAction;
class KActionMenu;

class KopeteContact;
class KopeteMetaContact;
class KopeteMessage;
class KopeteMessageManager;
class SMSContact;

class SMSProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	SMSProtocol(QObject *parent, const char *name, const QStringList &args);
	~SMSProtocol();

	static SMSProtocol *protocol();

	/**
	 * Deserialize contact data
	 */
	virtual KopeteContact *deserializeContact(KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	virtual AddContactPage *createAddContactWidget(QWidget *parent , KopeteAccount *i);
	virtual KopeteEditAccountWidget *createEditAccountWidget(KopeteAccount *account, QWidget *parent);
	virtual KopeteAccount *createNewAccount(const QString &accountId);

	const KopeteOnlineStatus SMSOnline;
	const KopeteOnlineStatus SMSUnknown;
	const KopeteOnlineStatus SMSOffline;

private:
	static SMSProtocol *s_protocol;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

