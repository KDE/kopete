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
#include "searchdialog.h"
#include "akonadihistorylogger.h"

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

#include <Akonadi/Monitor>
#include <Akonadi/CollectionFetchJob>


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

	KAction *viewDialog = new KAction( KIcon("view-history"), i18n("Search &History" ), this);
	actionCollection()->addAction("viewAkonadiHistoryDialog", viewDialog);
	viewDialog->setShortcut(KShortcut (Qt::CTRL + Qt::Key_H));
	connect(viewDialog, SIGNAL(triggered(bool)), this, SLOT(slotViewHistoryDialog()) );
	setXMLFile ( "historyui.rc" );


	Akonadi::CollectionFetchJob *fetch = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(),Akonadi::CollectionFetchJob::Recursive);
	connect(fetch,SIGNAL(finished(KJob*)),this,SLOT(collectionFetch(KJob*)) );
      
	Akonadi::Monitor *monitor = new Akonadi::Monitor;
	monitor->setCollectionMonitored(Akonadi::Collection::root());
	monitor->fetchCollection(true) ;
	connect(monitor, SIGNAL(collectionAdded(Akonadi::Collection,Akonadi::Collection)) ,
		SLOT(collectionAddedSlot(Akonadi::Collection,Akonadi::Collection)) );
            
	connect(monitor, SIGNAL(collectionRemoved(Akonadi::Collection)) , 
		SLOT(collectionRemovedSlot(Akonadi::Collection)) );


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
    
    AkonadiHistoryLogger *l=m_loggers[msg.manager()]->logger();
     if (l)
    {

        QList<Kopete::Contact*> mb = msg.manager()->members();
	kDebug() <<"calling append message";
	l->appendMessage(msg,mb.first());
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
	
	SearchDialog *s = new SearchDialog();
	
	s->setInitialSize( QSize( 800, 600 ) );
	s->show();
}



void AkonadiHistoryMessageHandler::handleMessage(Kopete::MessageEvent* event)
{
    if(m_plugin)
	m_plugin->messageDisplayed(event->message());
    
    Kopete::MessageHandler::handleMessage(event);
}

Akonadi::Collection AkonadiHistoryPlugin::getCollection(QString myId, QString contactId)
{
  kDebug() << "  ";
    Akonadi::Collection coll;
    
    if ( myId.isEmpty() )
	return m_kopeteChat;
    
    if ( !m_collectionHash.contains(myId) )
      return coll;
    
    Akonadi::Collection parent = m_collectionHash.value(myId);
    
    if ( contactId.isEmpty() )
      return parent;
    
    Akonadi::Collection::List list = m_idCollectionHash.value( parent.id() ) ;
    
    foreach( const Akonadi::Collection &collection, list)
    {
	if ( collection.name() == contactId )
	      coll=collection;
    }

    return coll;
}


void AkonadiHistoryPlugin::collectionFetch(KJob* job)
{
    kDebug() << "\ncollectiofetch job done";
    
    if (job->error() )
    {
	kWarning() << "collection fetch job failed" <<job->errorString();
	return;
    }
  
    Akonadi::CollectionFetchJob *fetchJob = static_cast<Akonadi::CollectionFetchJob *>(job);
    Akonadi::Collection::List collList= fetchJob->collections();
    
    if ( !m_kopeteChat.isValid() )
    {
	foreach ( const Akonadi::Collection &collection, collList)
	{
	    if (collection.name() == "KopeteChat")
	    {
		m_kopeteChat = collection;
		break;
	    }
	}
	
	if ( !m_kopeteChat.isValid() ) 
	{
	    kWarning() << " base collection doesnt exist. configure the resource";
	    return;
	}
	
	kDebug() <<"seting up the monitor";
	
    }

    if ( !m_kopeteChat.isValid() )
    {
	kWarning() << "the base collection kopete chat is invalid";
	return;
    }

    foreach ( const Akonadi::Collection &coll, collList)
    {
	if ( m_kopeteChat.id() == coll.parent() )
	{
	    m_collectionHash.insert(coll.name(), coll);
	    kDebug() <<coll.name();
	}
    }
    
    QHashIterator<QString, Akonadi::Collection> i(m_collectionHash);
    
    while ( i.hasNext() )
    {
	i.next();
	Akonadi::Collection c = i.value();
	
	Akonadi::Collection::List collectionList;
	
	foreach ( const Akonadi::Collection &coll, collList)
	{
	    if ( c.id() == coll.parent() )
	    {
		collectionList << coll;
		kDebug() << coll.name();
	    }
	}
	
	m_idCollectionHash.insert(c.id() , collectionList);
    }
    list() ;
}

void AkonadiHistoryPlugin::collectionAddedSlot(Akonadi::Collection coll , Akonadi::Collection parent )
{
    kDebug() << " in collection adeed slot";
    
    if ( !coll.isValid() )
    {
	kWarning() << " invalid collection monotored";
	return;
    }
    else kDebug() << "collection is valid" << coll.id();
    
    if ( m_kopeteChat.id() == parent.id() )
    {
	m_collectionHash.insert(coll.name(), coll);
	Akonadi::Collection::List list;
	m_idCollectionHash.insert(coll.id(), list);
	kDebug() << "coll is child of kopete chat"<<coll.id();
    }
    else
    {
	if ( m_idCollectionHash.contains( parent.id() ) )
	{
	    Akonadi::Collection::List list;
	    list = m_idCollectionHash.value( parent.id() );
	    list.append(coll);
	    m_idCollectionHash.insert(parent.id() , list );
	    kDebug() << "grandchild" << coll.id() ;
	}
    }
    
    list();
}


void AkonadiHistoryPlugin::collectionRemovedSlot(Akonadi::Collection coll)
{
    kDebug() << "\n collection removed sot" << coll.name();
    
    if ( m_kopeteChat.id() == coll.parent() )
    {
	m_collectionHash.remove( coll.name() ) ;
	m_idCollectionHash.remove( coll.id() );
	kDebug() << " coll removed. child";
    }
    else
    {
	if ( m_idCollectionHash.contains( coll.parent() ) )
	{
	    int i = m_idCollectionHash.value( coll.parent() ).indexOf( coll );
	    Akonadi::Collection::List list;
	    list = m_idCollectionHash.value( coll.parent() );
	    list.removeAt(i);
	    m_idCollectionHash.insert(coll.parent() , list);
	    kDebug() << " coll removed from list(granchild)";
	}
	
    }
    
    list();	
}



void AkonadiHistoryPlugin::list()
{
    kDebug() << "collections in ,m_collectionHash" ;
 
    QHashIterator<QString, Akonadi::Collection> it(m_collectionHash);
    while ( it.hasNext() )
    {
	it.next() ;
	kDebug() << it.key() ;
    }
    
    QHashIterator<Akonadi::Entity::Id, QList<Akonadi::Collection> > x( m_idCollectionHash );
    
    while ( x.hasNext() )
    {
	x.next();
	Akonadi::Collection::List l = x.value() ;
	Akonadi::Collection::Id id= x.key();
	Akonadi::Collection c( id );
	kDebug() << c.remoteId() ;
	foreach ( const Akonadi::Collection coll, l)
	    kDebug() << "\t " << coll.name() ;
    }

}

Akonadi::Item AkonadiHistoryPlugin::getItem(Akonadi::Entity::Id id)
{
    if( m_collItemBySession.contains( id ) )
    {
      kDebug() << "contains item";
      Akonadi::Item i = Akonadi::Item( m_collItemBySession.value( id ) ) ;
      return i;
      }
      
      kDebug() << "does not contain";
      
      return Akonadi::Item::Item() ;
}


void AkonadiHistoryPlugin::setItem(Akonadi::Collection::Id c, Akonadi::Item::Id i )
{
    kDebug() << c << i ;
    m_collItemBySession.insert(c, i) ;
}


#include "akonadihistoryplugin.moc"