/*
    yahooprotocol.h - Yahoo Plugin for Kopete

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

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

// Yahoo Protocol
class YahooProtocol : public KopeteProtocol
{
	Q_OBJECT
public:
	YahooProtocol( QObject *parent, const char *name, const QStringList &args );
	~YahooProtocol();
	
	static YahooProtocol *protocol();
	
	virtual void init();
	virtual bool unload();

	virtual const QString protocolIcon() { return "yahoo_online"; }
	virtual void deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );
	
	QString server() { return m_server; }
	int port() { return m_port; }

public slots:
	virtual AddContactPage *createAddContactWidget(QWidget * parent);
	virtual EditAccountWidget *createEditAccountWidget(KopeteAccount *account, QWidget *parent);
	virtual KopeteAccount *createNewAccount(const QString &accountId);

	void slotSettingsChanged(void);					// Callback when settings changed

private:
	static YahooProtocol* s_protocolStatic_;
	YahooPreferences *m_prefs;
	
	// Configuration data
	QString m_server;
	int m_port;
	bool m_logAll;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

