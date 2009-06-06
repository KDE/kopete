/*
    Privacy Plugin - Filter messages 

    Copyright (c) 2006 by Andre Duffeck <duffeck@kde.org>
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef PRIVACY_PLUGIN_H
#define PRIVACY_PLUGIN_H

#include <QObject>
#include <QMap>

#include "kopeteplugin.h"
#include <QVariantList>

namespace Kopete { 
	class Message;
	class ChatSession;
	class MessageEvent;
	class Contact;
}
class KopeteView;
class PrivacyMessageHandlerFactory;
class PrivacyGUIClient;

class PrivacyPlugin : public Kopete::Plugin
{
	Q_OBJECT
public:
	static PrivacyPlugin *plugin();

	PrivacyPlugin( QObject *parent, const QVariantList &args );
	~PrivacyPlugin();

	void addContactsToWhiteList( QList<const Kopete::Contact *> list );
	void addContactsToBlackList( QList<const Kopete::Contact *> list );

private Q_SLOTS:
	void slotSettingsChanged();
	void slotIncomingMessage( Kopete::MessageEvent *event );
	void slotAddToWhiteList();
	void slotAddToBlackList();

	void slotViewCreated( KopeteView* );
	void slotChatSessionClosed( Kopete::ChatSession* );
private:
	static PrivacyPlugin *pluginStatic_;
	PrivacyMessageHandlerFactory *m_inboundHandler;
	QMap<Kopete::ChatSession*,PrivacyGUIClient*> m_guiClients;
};

#endif
