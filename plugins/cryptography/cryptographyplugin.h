/*
    cryptographyplugin.h  -  description

    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2007      by Charles Connell        <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef CRYPTOGRAPHYPLUGIN_H
#define CRYPTOGRAPHYPLUGIN_H

#include <kopete/kopeteplugin.h>

#include "cryptographyconfig.h"

#include <QVariantList>

class QString;
class QTimer;
class KAction;

class CryptographyGUIClient;

namespace Kopete
{
	class Message;
	class ChatSession;
	class SimpleMessageHandlerFactory;
}

/**
  * @author Olivier Goffart
  * Main plugin class, handles mesages. Also has static functions used by rest of plugin
  */

class CryptographyPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	static CryptographyPlugin  *plugin();
	
	static QStringList supportedProtocols() { QStringList l; return l << "MSNProtocol" << "MessengerProtocol" << "JabberProtocol" << "YahooProtocol"; }
	static QStringList getKabcKeys (QString uid);
	static QString KabcKeySelector ( QString displayName, QString addresseeName, QStringList keys, QWidget *parent );

	CryptographyPlugin( QObject *parent, const QVariantList &args );
	~CryptographyPlugin();

public slots:
	void slotIncomingMessage( Kopete::Message& msg );
	void slotOutgoingMessage( Kopete::Message& msg );
	void slotContactSelectionChanged ();
	void slotExportSelectedMetaContactKeys ();	
	
private slots:
	void slotSelectContactKey();
	void slotNewKMM(Kopete::ChatSession *);
	
private:
	static CryptographyPlugin* mPluginStatic;
	Kopete::SimpleMessageHandlerFactory *mInboundHandler;
	KAction * mExportKeys;
};

#endif

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

