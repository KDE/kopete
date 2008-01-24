/***************************************************************************
                          otrplugin.h  -  description
                             -------------------
    begin                : 11 03 2007
    copyright            : (C) 2007-2007 by Michael Zanetti
    email                : michael_zanetti@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OTRPLUGIN_H
#define OTRPLUGIN_H


#include <kopeteplugin.h>
#include <kopetemessagehandler.h>

#include "otrlchatinterface.h"

#include "qvariant.h"

/**
  * @author Michael Zanetti
  */

class OTRPlugin;
class KSelectAction;

class OtrMessageHandler : public Kopete::MessageHandler
{
private:
	OTRPlugin *plugin;
public:
	OtrMessageHandler( OTRPlugin *plugin ) : plugin(plugin) {
//		kdDebug() << "MessageHandler created" << endl;
	}
	~OtrMessageHandler(){
//		kdDebug() << "MessageHandler destroyed" << endl;
	}
	void handleMessage( Kopete::MessageEvent *event );
};

class OtrMessageHandlerFactory : public Kopete::MessageHandlerFactory
{
private:
	OTRPlugin *plugin;
	OtrMessageHandler *messageHandler;
public:
	OtrMessageHandlerFactory( OTRPlugin *plugin ) : plugin(plugin) {}
	Kopete::MessageHandler *create( Kopete::ChatSession *, Kopete::Message::MessageDirection direction )
	{
		return new OtrMessageHandler(plugin);
	}
	int filterPosition( Kopete::ChatSession *, Kopete::Message::MessageDirection )
	{
		return Kopete::MessageHandlerFactory::InStageToSent+1;
	}
};

class OTRPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:

	static OTRPlugin  *plugin();

	OTRPlugin( QObject *parent, const QVariantList &args );
	~OTRPlugin();

	void emitGoneSecure( Kopete::ChatSession *session, int status );
	QMap<QString, QString> getMessageCache();

public slots:

	void slotOutgoingMessage( Kopete::Message& msg );
	void slotEnableOtr( Kopete::ChatSession *session, bool enable );
	void slotSettingsChanged();
	void slotVerifyFingerprint( Kopete::ChatSession *session );

private slots:
	void slotNewChatSessionWindow(Kopete::ChatSession * );
	void slotSelectionChanged( bool single );
	void slotSetPolicy();
	void accountReady( Kopete::Account *account );
	void slotSecuritySate(Kopete::ChatSession *session, int state);

private:
	static OTRPlugin* pluginStatic_;
	OtrMessageHandlerFactory *m_inboundHandler;
	OtrlChatInterface *otrlChatInterface;
	QMap<QString, QString> messageCache;
	KSelectAction* otrPolicyMenu;

/*	KActionMenu *otrPolicyMenuBar;
	KActionMenu *otrPolicyPopup;
	KAction *otrPolicyDefault;
	KAction *otrPolicyAlways;
	KAction *otrPolicyOpportunistic;
	KAction *otrPolicyManual;
	KAction *otrPolicyNever;
//	SessionManager manager
*/

signals:
	void goneSecure( Kopete::ChatSession *session, int state );


};

#endif
