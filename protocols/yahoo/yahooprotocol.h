/*
    yahooprotocol.h - Yahoo Plugin for Kopete

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2003 by Matt Rogers <mattrogers@sbcglobal.net

    Copyright (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOPROTOCOL_H
#define YAHOOPROTOCOL_H

#include "libyahoo2/yahoo2.h"
#include "libyahoo2/yahoo2_callbacks.h"

// Local Includes
#include "yahooprefs.h"
#include "kyahoo.h"

// Kopete Includes

// QT Includes
#include <qpixmap.h>
#include <qmap.h>

// KDE Includes
#include "kopeteprotocol.h"

class YahooContact;
class KPopupMenu;
class KActionMenu;
class KAction;
class KopeteMetaContact;
class KopeteMessage;
class YahooPreferences;

class YahooProtocol : public KopeteProtocol
{
	Q_OBJECT
public:
	YahooProtocol( QObject *parent, const char *name, const QStringList &args );
	~YahooProtocol();

	static YahooProtocol *protocol();

	virtual void deserializeContact( KopeteMetaContact *metaContact,
					 const QMap<QString,QString> &serializedData,
					 const QMap<QString, QString> &addressBookData );

	QString server() const { return m_server; }
	int port() const { return m_port; }
	bool importContacts() const { return m_importContacts; }
	bool useGroupNames() const { return m_useGroupNames; }

public slots:
	virtual AddContactPage *createAddContactWidget(QWidget * parent, KopeteAccount* a);
	virtual EditAccountWidget *createEditAccountWidget(KopeteAccount *account, QWidget *parent);
	virtual KopeteAccount *createNewAccount(const QString &accountId);

	/**
	 * Callback when settings changed
	 */
	void slotSettingsChanged(void);

private:
	static YahooProtocol* s_protocolStatic_;
	YahooPreferences *m_prefs;

	// Configuration data
	QString m_server;
	int m_port;
	bool m_logAll, m_useGroupNames, m_importContacts;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

