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

#ifndef OTRPLUGIN_H
#define OTRPLUGIN_H


#include <kopeteplugin.h>
#include <kopetemessagehandler.h>

#include "otrlchatinterface.h"

#include "qvariant.h"
#include <QPointer>
#include <QPair>

/**
  * @author Michael Zanetti
  */

class OTRPlugin;
class KSelectAction;

class OtrMessageHandler : public Kopete::MessageHandler
{
private:
	QPointer<OTRPlugin> plugin;
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
		Q_UNUSED(direction)
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
	QMap<QString, QPair<QString, bool> > getMessageCache();

public slots:

	void slotOutgoingMessage( Kopete::Message& msg );
	void slotEnableOtr( Kopete::ChatSession *session, bool enable );
	void slotSettingsChanged();
	void slotVerifyFingerprint( Kopete::ChatSession *session );

private slots:
	void slotNewChatSessionWindow(Kopete::ChatSession * );
	void slotSelectionChanged( bool single );
	void slotSetPolicy();
	void slotSecuritySate(Kopete::ChatSession *session, int state);

private:
	static OTRPlugin* pluginStatic_;
	OtrMessageHandlerFactory *m_inboundHandler;
	OtrlChatInterface *otrlChatInterface;
	QMap<QString, QPair<QString, bool> > messageCache;
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
