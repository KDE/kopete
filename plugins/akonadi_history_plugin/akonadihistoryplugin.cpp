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

#include <QTextDocument>


#include <KDebug>
#include <kgenericfactory.h>


#include <kopetemessageevent.h>

/**

*/

K_PLUGIN_FACTORY ( AkonadiHistoryMessagePluginFactory, registerPlugin<AkonadiHistoryPlugin>(); )
K_EXPORT_PLUGIN (  AkonadiHistoryMessagePluginFactory ( "akonadi_kopete_history" ) )

AkonadiHistoryPlugin::AkonadiHistoryPlugin(QObject* parent, const QVariantList &args)
     : Kopete::Plugin(AkonadiHistoryMessagePluginFactory::componentData(), parent), m_messageHandlerFactory(this)
{
    kDebug() << "AkonadiHistoryMessagePluginFactory Loaded :) ..............";
    
}

AkonadiHistoryPlugin::~AkonadiHistoryPlugin()
{
    kDebug() << "Akonadi History Plugin Distructor ";
}


void AkonadiHistoryPlugin::messageDisplayed(const Kopete::Message& msg)
{
    kDebug() << msg.body()->toPlainText() ;
}


void AkonadiHistoryMessageHandler::handleMessage(Kopete::MessageEvent* event)
{
    if(m_plugin)
	m_plugin->messageDisplayed(event->message());
    
    Kopete::MessageHandler::handleMessage(event);
}


