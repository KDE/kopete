/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart @ kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "knotificationmanager.h"
#include "knotification.h"

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <klocale.h>

#include <QHash>

typedef QHash<QString,QString> Dict;

struct KNotificationManager::Private
{
	QHash<int , KNotification*> notifications;
};

KNotificationManager * KNotificationManager::s_self = 0L;

KNotificationManager * KNotificationManager::self()
{
	static KStaticDeleter<KNotificationManager> deleter;
	if(!s_self)
		deleter.setObject( s_self, new KNotificationManager() );
	return s_self;
}



KNotificationManager::KNotificationManager() : DCOPObject("KNotification") , d(new Private)
{
	
	bool b1=connectDCOPSignal("knotify", "Notify",
					  "NotificationClosed(unsigned int,unsigned int)", 
					  "notificationClosed(unsigned int,unsigned int)", false);
	bool b2=connectDCOPSignal("knotify", "Notify",
					  "ActionInvoked (unsigned int,unsigned int)", 
					  "notificationActivated(unsigned int,unsigned int)", false);
	
	kdDebug() << k_funcinfo << b1 << " " << b2 << endl;
}


KNotificationManager::~KNotificationManager()
{
	s_self = 0L;
	delete d;
}


ASYNC KNotificationManager::notificationActivated( unsigned int id, unsigned int action )
{
	kdDebug() << k_funcinfo << id << " " << action << endl;
	if(d->notifications.contains(id))
	{
		KNotification *n = d->notifications[id];
		kdDebug() << k_funcinfo << n->title() << endl;
		d->notifications.remove(id);
		n->activate( action );
	}
}

ASYNC KNotificationManager::notificationClosed( unsigned int id, unsigned int /*reason*/ )
{
	if(d->notifications.contains(id))
	{
		KNotification *n = d->notifications[id];
		d->notifications.remove(id);
		n->close();
	}
}


void KNotificationManager::close(unsigned int id)
{
	DCOPClient *client=KApplication::dcopClient();
	QByteArray data;
	QDataStream arg(&data, IO_WriteOnly);
	arg << id;
	if (!client->send("knotify", "Notify", "CloseNotification(unsigned int)", data))
	{
		kdDebug() << k_funcinfo << "error while contacting knotify server" << endl;
	}
}

unsigned int KNotificationManager::notify( KNotification* n , const QPixmap &pix , const QStringList &actions , const QString & sound)
{
	DCOPClient *client=KApplication::dcopClient();
	QByteArray data, replyData;
	DCOPCString replyType;
	QDataStream arg(&data, IO_WriteOnly);
	QString appname=kapp->instanceName();
	KConfig eventsFile( appname+QString::fromAscii( "/eventsrc" ), true, false, "data");
	KConfigGroup config( &eventsFile, "!Global!" );
	QString iconName = config.readEntry( "IconName", appname );
	KIconLoader iconLoader( appname );
	QPixmap appicon = iconLoader.loadIcon( iconName, KIcon::Small );

	Dict  hints;
	if(! sound.isEmpty() )
		hints["sound-file"]=sound;
	else
		hints["suppress-sound"]="true";

	if( n->widget() )
		hints["win-id"]=QString::number(n->widget()->winId());
	
	arg << appname << appicon << 0 << n->eventId() << 0 << n->title() << n->text() << pix ;
	arg << actions << hints << false << 0;
	if (!client->call("knotify", "Notify", "Notify(QString,QPixmap,unsigned int,QString,int,QString,QString,QPixmap,QStringList,Dict,bool,unsigned int)" ,
		 	 data, replyType, replyData))
	{
		kdDebug() << k_funcinfo << "error while contacting knotify server" << endl;
	}
	else 
	{
		QDataStream reply(&replyData, IO_ReadOnly);
		if (replyType == "unsigned int") 
		{
			unsigned int result;
			reply >> result;
			d->notifications.insert(result, n);
			return result;
		}
		else
			kdDebug() << k_funcinfo << "bad reply from server" << endl;
	}
	return 0;
}


void KNotificationManager::remove( unsigned int id)
{
	d->notifications.remove(id);
}

bool KNotificationManager::notifyBySound(const QString &file, const QString& appname,unsigned int id)
{
	DCOPClient *client=KApplication::dcopClient();
	QByteArray data;
	QDataStream arg(&data, IO_WriteOnly);
	arg << file ;
	arg << appname ;
	arg << (int)id;
	if (!client->send("knotify", "Notify", "notifyBySound(QString,QString,int)", data))
	{
		kdDebug() << k_funcinfo << "error while contacting knotify server" << endl;
		return false;
	}
	return true;
	
}

