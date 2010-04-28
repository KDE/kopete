/*
  akonadihistoryplugin.h 
  
   Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>
   Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>
 
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

#ifndef AKONADIHISTORYPLUGIN_H
#define AKONADIHISTORYPLUGIN_H

#include <QVariantList>
#include <QPointer>

#include <kopeteplugin.h>
#include <kopetemessagehandler.h>
#include <KComponentData>

class KopeteView;
class HistoryActionManager;
class AkonadiHistoryPlugin;

class AkonadiHistoryMessageHandler : public Kopete::MessageHandler 
{
private:
  QPointer<AkonadiHistoryPlugin> m_plugin;
public:
    AkonadiHistoryMessageHandler(AkonadiHistoryPlugin *plugin) : m_plugin(plugin) {}
    
    void handleMessage( Kopete::MessageEvent *event );
  
};



class AkonadiHistoryMessageHandlerFactory : public Kopete::MessageHandlerFactory
{
private:
    QPointer<AkonadiHistoryPlugin> m_plugin ;
public:
    AkonadiHistoryMessageHandlerFactory(AkonadiHistoryPlugin *plugin) : m_plugin(plugin) {}

    Kopete::MessageHandler *create(Kopete::ChatSession *, Kopete::Message::MessageDirection direction)
    {
        if ( direction != Kopete::Message::Inbound )
            return 0;
        
	return new AkonadiHistoryMessageHandler(m_plugin);
    }
    
    int filterPosition( Kopete::ChatSession *, Kopete::Message::MessageDirection )
    {
        return Kopete::MessageHandlerFactory::InStageToSent+10;
    }	
};


class AkonadiHistoryPlugin : public Kopete::Plugin
{
    Q_OBJECT
public:
	AkonadiHistoryPlugin(QObject* parent, const QVariantList &args); 
	~AkonadiHistoryPlugin();
    
	void messageDisplayed(const Kopete::Message &msg) ;
	const KComponentData &xmlGuiInstance() { return m_XmlGuiInstance; }
    
private:
	AkonadiHistoryMessageHandlerFactory m_messageHandlerFactory;
	QHash<Kopete::ChatSession*,HistoryActionManager*> m_loggers;
	KComponentData m_XmlGuiInstance ;
    
private slots:
	void slotViewCreated(KopeteView*);
	void slotKMMClosed( Kopete::ChatSession* );
	void slotAddTag() ;
	void slotViewHistoryDialog();
    
};

#endif // AKONADIHISTORYPLUGIN_H
