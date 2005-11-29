/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart @ kde.org>

   code from KNotify/KNotifyClient
   Copyright (c) 1997 Christian Esken (esken@kde.org)
                 2000 Charles Samuels (charles@kde.org)
                 2000 Stefan Schimanski (1Stein@gmx.de)
                 2000 Matthias Ettrich (ettrich@kde.org)
                 2000 Waldo Bastian <bastian@kde.org>
                 2000-2003 Carsten Pfeiffer <pfeiffer@kde.org>
                 2005 Allan Sandfeld Jensen <kde@carewolf.com>

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

#include "knotification.h"
#include "knotificationmanager.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kpassivepopup.h>
#include <kactivelabel.h>
#include <kprocess.h>
#include <kdialog.h>
#include <kmacroexpander.h>
#include <kwin.h>
#include <kdebug.h>

#include <kvbox.h>
#include <QMap>
#include <QPixmap>
#include <dcopclient.h>
#include <qpointer.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qtabwidget.h>
#include <kapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <QDateTime>
#include <QMetaObject>
#include <QMetaEnum>

//TODO,  make the KNotification aware of the systemtray.
#include "kopeteuiglobal.h"
static WId checkWinId( const QString &/*appName*/, WId senderWinId )
{
	if(senderWinId==0)
		senderWinId=Kopete::UI::Global::sysTrayWId();

	return senderWinId;
}


struct KNotification::Private
{
	QWidget *widget;
	QString text;
	QStringList actions;
	int level;
	QString eventId;
	QString title;
	unsigned int id;
	
	int ref;
	
	Private() : widget(0l) , id(0), ref(1) {}
};

KNotification::KNotification(QObject *parent) :
		QObject(parent) , d(new Private)
{
}

KNotification::~KNotification()
{
	if(d ->id != 0)
		KNotificationManager::self()->remove( d->id );
	delete d;
}

QString KNotification::eventId() const
{
	return d->eventId;
}

QString KNotification::text() const
{
	return d->text;
}

QString KNotification::title() const
{
	return d->title;
}

QWidget *KNotification::widget() const
{
	return d->widget;
}


void KNotification::notifyByExecute(const QString &command, const QString& event,
							  const QString& fromApp, const QString& text,
							  int winId, int eventId)
{
	if (!command.isEmpty()) {
// 	kdDebug() << "executing command '" << command << "'" << endl;
		QHash<QChar,QString> subst;
		subst.insert( 'e', event );
		subst.insert( 'a', fromApp );
		subst.insert( 's', text );
		subst.insert( 'w', QString::number( winId ));
		subst.insert( 'i', QString::number( eventId ));
		QString execLine = KMacroExpander::expandMacrosShellQuote( command, subst );
		if ( execLine.isEmpty() )
			execLine = command; // fallback

		KProcess p;
		p.setUseShell(true);
		p << execLine;
		p.start(KProcess::DontCare);
// 		return true;
	}
// 	return false;
}


void KNotification::notifyByMessagebox()
{
		// ignore empty messages
	if ( d->text.isEmpty() )
		return;

	QStringList actions=d->actions;
	WId winId=d->widget ? d->widget->topLevelWidget()->winId()  : 0;

	if( actions.count() == 0)
	{
	// display message box for specified event level
		switch( d->level )
		{
			case Warning:
				KMessageBox::sorryWId( winId, d->text, i18n( "Warning" ) , false );
				break;
			case Error:
				KMessageBox::errorWId( winId, d->text, i18n( "Error" ) , false );
				break;
			case Catastrophe:
				KMessageBox::errorWId( winId, d->text, i18n( "Catastrophe!" ) , false );
				break;
			default:
			case Notification:
				KMessageBox::informationWId( winId, d->text, i18n( "Notification" ) , 0 , false );
				break;

		}
	}
	else
	{ //we may show the specific action button
		int result=0;
		QPointer<KNotification> _this=this; //this can be deleted
		switch( d->level )
		{
			default:
			case Notification:
				result = KMessageBox::questionYesNo(d->widget, d->text, i18n( "Notification" ), actions[0], KStdGuiItem::cancel(), QString::null, false );
				break;
			case Warning:
				result = KMessageBox::warningYesNo( d->widget, d->text, i18n( "Warning" ), actions[0], KStdGuiItem::cancel(), QString::null, false );
				break;
			case Error:
				result = KMessageBox::warningYesNo( d->widget, d->text, i18n( "Error" ), actions[0], KStdGuiItem::cancel(), QString::null, false );
				break;
			case Catastrophe:
				result = KMessageBox::warningYesNo( d->widget, d->text, i18n( "Catastrophe!" ), actions[0], KStdGuiItem::cancel(), QString::null, false );
				break;
		}
		if(result==KMessageBox::Yes && _this)
		{
			activate(0);
		}
	}
}

void KNotification::notifyByPassivePopup(const QPixmap &pix, const QString& sound )
{	
	ref();
	d->id=KNotificationManager::self()->notify( this , pix , d->actions , sound );
	kdDebug() << k_funcinfo << d->id << endl;
}


bool KNotification::notifyByLogfile(const QString &text, const QString &file)
{
    // ignore empty messages
	if ( text.isEmpty() )
		return true;

    // open file in append mode
	QFile logFile(file);
	if ( !logFile.open(QIODevice::WriteOnly | QIODevice::Append) )
		return false;

    // append msg
	QTextStream strm( &logFile );
	strm << "- KNotify " << QDateTime::currentDateTime().toString() << ": ";
	strm << text << endl;

    // close file
	logFile.close();
	return true;
}

bool KNotification::notifyByStderr(const QString &text)
{
    // ignore empty messages
	if ( text.isEmpty() )
		return true;

    // open stderr for output
	QTextStream strm( stderr, QIODevice::WriteOnly );

    // output msg
	strm << "KNotify " << QDateTime::currentDateTime().toString() << ": ";
	strm << text << endl;

	return true;
}

bool KNotification::notifyByTaskbar( WId win )
{
	if( win == 0 )
		return false;
	KWin::demandAttention( win );
	return true;
}



bool KNotification::notifyBySound( const QString &sound, const QString &appname, int eventId )
{
	return KNotificationManager::self()->notifyBySound( sound, appname, eventId );
}


void KNotification::slotPopupLinkClicked(const QString &adr)
{
	unsigned int action=adr.toUInt();
	if(action==0)
		return;

	activate(action);
}



void KNotification::activate(unsigned int action)
{
	if(action==0)
		emit activated();
	emit activated(action);
	kdDebug() << k_funcinfo << d->id << " " << action << endl;
	deleteLater();
}


void KNotification::close()
{
	if(d->id != 0)
		KNotificationManager::self()->close( d->id );
	emit closed();
	deleteLater();
}


void KNotification::raiseWidget()
{
	if(!d->widget)
		return;

	raiseWidget(d->widget);
}


void KNotification::raiseWidget(QWidget *w)
{
	//TODO  this funciton is far from finished.
	if(w->isTopLevel())
	{
		w->raise();
		KWin::activateWindow( w->winId() );
	}
	else
	{
		QWidget *pw=w->parentWidget();
		raiseWidget(pw);

		if( QTabWidget *tab_widget=dynamic_cast<QTabWidget*>(pw))
		{
			tab_widget->showPage(w);
		}
	}
}


//to workaround the fact that , i not allowed in a macro
#define VIRGULE ,    

//this is temporary code
class ConfigHelper
{
public:
	KConfig eventsfile,configfile;
	KNotification::ContextList contexts;
	QString eventid;
	ConfigHelper(const QString &appname, const KNotification::ContextList &_contexts , const QString &_eventid) 
		:	eventsfile( appname+QString::fromAscii( "/eventsrc" ), true, false, "data"),
			configfile( appname+QString::fromAscii( ".eventsrc" ), true, false),
			contexts(_contexts) , eventid(_eventid)
	{}
				
	QString readEntry(const QString& entry , bool path=false)
	{
		foreach( QPair<QString VIRGULE QString> context , contexts )
		{
			const QString group="Event/" + eventid + "/" + context.first + "/" + context.second;
			if(configfile.hasGroup( group ) )
			{
				configfile.setGroup(group);
				QString p=path ?  configfile.readPathEntry(entry) : configfile.readEntry(entry);
				kdDebug() << k_funcinfo << "group=" << group << " value=" << p << " isNull=" << p.isNull() << endl; 
				if(!p.isNull())
					return p;
			}
		}
		const QString group="Event/" + eventid ;
		kdDebug() << k_funcinfo << "group=" << group  << endl;
		if(configfile.hasGroup( group ) )
		{
			configfile.setGroup(group);
			QString p=path ?  configfile.readPathEntry(entry) : configfile.readEntry(entry);
			kdDebug() << k_funcinfo << "bib group=" << group << " value=" << p << " isNull=" << p.isNull() << endl;
			if(!p.isNull())
				return p;
		}
		if(eventsfile.hasGroup( group ) )
		{
			eventsfile.setGroup(group);
			QString p=path ?  eventsfile.readPathEntry(entry) : eventsfile.readEntry(entry);
			kdDebug() << k_funcinfo << "bob group=" << group << " value=" << p << " isNull=" << p.isNull() << endl;
			if(!p.isNull())
				return p;
		}
		return QString::null;
	}

};
	


KNotification *KNotification::event( const QString& eventid , const QString& text,
			const QPixmap& pixmap, QWidget *widget, const QStringList &actions,
			ContextList contexts, unsigned int flags)
{
	int level=Notification;
	QString appname = QString::fromAscii(kapp->instanceName());
	KNotification *notify=new KNotification(widget);
	notify->d->widget=widget;
	notify->d->text=text;
	notify->d->actions=actions;
	notify->d->level=level;
	notify->d->eventId=eventid;

	WId winId=widget ? widget->topLevelWidget()->winId()  : 0;
	
		// get config file
	ConfigHelper config( appname, contexts, eventid );
	config.eventsfile.setGroup("Global");
	notify->d->title=config.eventsfile.readEntry( "Comment", appname );
	
	int n = notify->metaObject()->indexOfEnumerator( "NotifyPresentation" );
	QString presentstring=config.readEntry("Action");
	int present= notify->metaObject()->enumerator(n).keysToValue( presentstring.toLatin1() );
	if(present == -1)
		present=0;
	
	kdDebug() << k_funcinfo << "Action: " << presentstring << " - " << present << endl;

	// get default event level
	if( present & Messagebox )
		level = config.readEntry( "level"  ).toUInt();

	if ( present & PassivePopup )
	{
		notify->notifyByPassivePopup( pixmap , (present & Sound) ? config.readEntry( "sound" , true) : QString::null );
	}
	else if ( present & Sound ) // && QFile(sound).isReadable()
		notify->notifyBySound( config.readEntry( "sound" , true), kapp->instanceName(), 0 );

	
	
	if ( present & Messagebox )
	{
		QTimer::singleShot(0,notify,SLOT(notifyByMessagebox()));
	}
	else  //not a message box  (because closing the event when a message box is there is suicide)
		if(flags & CloseOnTimeout)
	{
		QTimer::singleShot(6*1000, notify, SLOT(close()));
	}
	if ( present & Execute )
	{
		notify->notifyByExecute(config.readEntry( "commandline" ), QString::null,appname,text, winId, 0 );
	}
	

	if ( present & Logfile ) // && QFile(file).isWritable()
		notify->notifyByLogfile( text, config.readEntry( "logfile" , true) );
	
	if ( present & Stderr )
		notify->notifyByStderr( text );

	if ( present & Taskbar )
		notify->notifyByTaskbar( checkWinId( kapp->instanceName(), winId ));
	
	//after a small timeout, the notification will be deleted if all presentation are finished
	QTimer::singleShot(1000, notify, SLOT(deref()));
	
#if 0  //TODO
	QByteArray qbd;
	QDataStream ds(&qbd, QIODevice::WriteOnly);
	ds << event << fromApp << text << sound << file << present << level
			<< winId << eventId;
	emitDCOPSignal("notifySignal(QString,QString,QString,QString,QString,int,int,int,int)", qbd);
#endif

	return notify;
}

void KNotification::ref()
{
	d->ref++;
}

void KNotification::deref()
{
	d->ref--;
	if(d->ref==0)
		close();
}

#include "knotification.moc"



