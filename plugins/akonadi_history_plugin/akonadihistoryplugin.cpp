/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "akonadihistoryplugin.h"
#include "historyactionmanager.h"

#include <QTextDocument>


#include <KDebug>
#include <kgenericfactory.h>


#include <kopetemessageevent.h>
#include <kopetechatsession.h>
#include <kopetechatsessionmanager.h>
#include "kopeteview.h"
#include <kopeteviewplugin.h>

#include <KAction>
#include <KActionCollection>
#include <KPluginInfo>


/**

*/

K_PLUGIN_FACTORY ( AkonadiHistoryMessagePluginFactory, registerPlugin<AkonadiHistoryPlugin>(); )
K_EXPORT_PLUGIN (  AkonadiHistoryMessagePluginFactory ( "akonadi_kopete_history" ) )

AkonadiHistoryPlugin::AkonadiHistoryPlugin(QObject* parent, const QVariantList &args)
     : Kopete::Plugin(AkonadiHistoryMessagePluginFactory::componentData(), parent), m_messageHandlerFactory(this)
{
	kDebug() << "AkonadiHistoryMessagePluginFactory Loaded :) ..............";
	m_XmlGuiInstance = AkonadiHistoryMessagePluginFactory::componentData() ;
	
	connect(Kopete::ChatSessionManager::self(), SIGNAL(viewCreated(KopeteView*)), this, SLOT(slotViewCreated(KopeteView*)) ); 

	KAction *viewDialog = new KAction( KIcon("view-history"), i18n("View &History" ), this);
	actionCollection()->addAction("viewAkonadiHistoryDialog", viewDialog);
	viewDialog->setShortcut(KShortcut (Qt::CTRL + Qt::Key_H));
	connect(viewDialog, SIGNAL(triggered(bool)), this, SLOT(slotViewHistoryDialog()) );
	setXMLFile ( "historyui.rc" );

}

AkonadiHistoryPlugin::~AkonadiHistoryPlugin()
{
    kDebug() << "Akonadi History Plugin Distructor ";
}


void AkonadiHistoryPlugin::messageDisplayed(const Kopete::Message& msg)
{
    kDebug() << msg.body()->toPlainText();
    if (msg.direction()==Kopete::Message::Internal || !msg.manager() ||
            (msg.type() == Kopete::Message::TypeFileTransferRequest && msg.plainBody().isEmpty() ) )
    {	
        return;
    }
    
    if (!m_loggers.contains(msg.manager()))
    {
	m_loggers.insert(msg.manager() , new HistoryActionManager( msg.manager() , this ) );
	
        connect(msg.manager(), SIGNAL(closing(Kopete::ChatSession*)),
                this, SLOT(slotKMMClosed(Kopete::ChatSession*)));
    }
}


void AkonadiHistoryPlugin::slotKMMClosed(Kopete::ChatSession* kmm)
{
    m_loggers[kmm]->deleteLater();
    m_loggers.remove(kmm);
}

void AkonadiHistoryPlugin::slotAddTag()
{
	kDebug() << "slot add tag";
}

void AkonadiHistoryPlugin::slotViewCreated(KopeteView* v)
{
    if (v->plugin()->pluginInfo().pluginName() != QString::fromLatin1("kopete_chatwindow") )
        return;  //Email chat windows are not supported.

    KopeteView *m_currentView = v;
    Kopete::ChatSession *m_currentChatSession = v->msgManager();
    

    if (!m_currentChatSession)
        return; //i am sorry
    
    if (!m_loggers.contains(m_currentChatSession))
    {	
        m_loggers.insert(m_currentChatSession , new HistoryActionManager( m_currentChatSession , this ) );
        connect( m_currentChatSession, SIGNAL(closing(Kopete::ChatSession*)),
                 this , SLOT(slotKMMClosed(Kopete::ChatSession*)));
    }

}

void AkonadiHistoryPlugin::slotViewHistoryDialog()
{
	kDebug() << "Slot view history Dialog called";
}



void AkonadiHistoryMessageHandler::handleMessage(Kopete::MessageEvent* event)
{
    if(m_plugin)
	m_plugin->messageDisplayed(event->message());
    
    Kopete::MessageHandler::handleMessage(event);
}

