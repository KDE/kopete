/*
    Privacy Plugin - Filter messages 

    Copyright (c) 2006 by Andre Duffeck <andre@duffeck.de>
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

#include "kopeteplugin.h"

namespace Kopete { 
	class Message;
	class MetaContact;
	class ChatSession;
}
class PrivacyMessageHandlerFactory;

class PrivacyPlugin : public Kopete::Plugin
{
	Q_OBJECT
public:
	static PrivacyPlugin *plugin();

	PrivacyPlugin( QObject *parent, const QStringList &args );
	~PrivacyPlugin();

private Q_SLOTS:
	void slotSettingsChanged();
	void slotIncomingMessage( Kopete::MessageEvent *event );

private:
	static PrivacyPlugin *pluginStatic_;
	PrivacyMessageHandlerFactory *m_inboundHandler;
};

#endif
