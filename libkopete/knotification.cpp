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
#include <q3cstring.h>
#include <qpointer.h>
#include <q3stylesheet.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qtabwidget.h>
#include <kapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <QDateTime>

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
	
	Private() : widget(0l) , id(0) {}
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

	QString action=d->actions[0];
	WId winId=d->widget ? d->widget->topLevelWidget()->winId()  : 0;

	if( action.isEmpty())
	{
	// display message box for specified event level
		switch( d->level )
		{
			default:
			case Notification:
				KMessageBox::informationWId( winId, d->text, i18n( "Notification" ) , 0 , false );
				break;
			case Warning:
				KMessageBox::sorryWId( winId, d->text, i18n( "Warning" ) , false );
				break;
			case Error:
				KMessageBox::errorWId( winId, d->text, i18n( "Error" ) , false );
				break;
			case Catastrophe:
				KMessageBox::errorWId( winId, d->text, i18n( "Catastrophe!" ) , false );
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
				result = KMessageBox::questionYesNo(d->widget, d->text, i18n( "Notification" ), action, KStdGuiItem::cancel(), QString::null, false );
				break;
			case Warning:
				result = KMessageBox::warningYesNo( d->widget, d->text, i18n( "Warning" ), action, KStdGuiItem::cancel(), QString::null, false );
				break;
			case Error:
				result = KMessageBox::warningYesNo( d->widget, d->text, i18n( "Error" ), action, KStdGuiItem::cancel(), QString::null, false );
				break;
			case Catastrophe:
				result = KMessageBox::warningYesNo( d->widget, d->text, i18n( "Catastrophe!" ), action, KStdGuiItem::cancel(), QString::null, false );
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
	if ( !logFile.open(IO_WriteOnly | IO_Append) )
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
	QTextStream strm( stderr, IO_WriteOnly );

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




KNotification *KNotification::event( const QString& message , const QString& text,
			const QPixmap& pixmap, QWidget *widget, const QStringList &actions,
			ContextList contexts, unsigned int flags)
{
	/* NOTE:  this function still use the KNotifyClient,
	 *        in the future (KDE4) all the function of the knotifyclient will be moved there.
	 *  Some code here is derived from the old KNotify deamon
	 */
	
	//todo: handle context

	int level=Default;
	QString sound;
	QString file;
	QString commandline;
	QString appname = QString::fromAscii(kapp->instanceName());
	// get config file
	KConfig eventsFile( appname+QString::fromAscii( "/eventsrc" ), true, false, "data");
	eventsFile.setGroup(message);

	KConfig configFile( appname+QString::fromAscii( ".eventsrc" ), true, false);
	configFile.setGroup(message);

	int present=getPresentation(message);
	if(present==-1)
		present=getDefaultPresentation(message);
	if(present==-1)
		present=0;

	// get sound file name
	if( present & Sound ) {
		QString theSound = configFile.readPathEntry( "soundfile" );
		if ( theSound.isEmpty() )
			theSound = eventsFile.readPathEntry( "default_sound" );
		if ( !theSound.isEmpty() )
			sound = theSound;
	}

	// get log file name
	if( present & Logfile ) {
		QString theFile = configFile.readPathEntry( "logfile" );
		if ( theFile.isEmpty() )
			theFile = eventsFile.readPathEntry( "default_logfile" );
		if ( !theFile.isEmpty() )
			file = theFile;
	}

	// get default event level
	if( present & Messagebox )
		level = eventsFile.readNumEntry( "level", 0 );

	// get command line
	if (present & Execute ) {
		commandline = configFile.readPathEntry( "commandline" );
		if ( commandline.isEmpty() )
			commandline = eventsFile.readPathEntry( "default_commandline" );
	}

	KNotification *notify=new KNotification(widget);
	notify->d->widget=widget;
	notify->d->text=text;
	notify->d->actions=actions;
	notify->d->level=level;
	notify->d->eventId=message;
	notify->d->title=eventsFile.readEntry( "Comment", appname );

	WId winId=widget ? widget->topLevelWidget()->winId()  : 0;

	if ( present & PassivePopup )
	{
		notify->notifyByPassivePopup( pixmap , (present & Sound) ? sound : QString::null );
	}
	else if ( present & Sound ) // && QFile(sound).isReadable()
		notify->notifyBySound( sound, kapp->instanceName(), 0 );

	
	
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
		notify->notifyByExecute(commandline, QString::null,appname,text, winId, 0 );
	}
	

	if ( present & Logfile ) // && QFile(file).isWritable()
		notify->notifyByLogfile( text, file );
	
	if ( present & Stderr )
		notify->notifyByStderr( text );

	if ( present & Taskbar )
		notify->notifyByTaskbar( checkWinId( kapp->instanceName(), winId ));
	
	
#if 0  //TODO
	QByteArray qbd;
	QDataStream ds(&qbd, IO_WriteOnly);
	ds << event << fromApp << text << sound << file << present << level
			<< winId << eventId;
	emitDCOPSignal("notifySignal(QString,QString,QString,QString,QString,int,int,int,int)", qbd);
#endif

	return notify;
}



#if 0

/* This code is there before i find a great way to perform context-dependent notifications
 * in a way independent of kopete.
 *   i'm in fact still using the Will's old code.
 */



#include "kopeteeventpresentation.h"
#include "kopetegroup.h"
#include "kopetenotifydataobject.h"
#include "kopetenotifyevent.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include <qimage.h>


static KNotification *performCustomNotifications( QWidget *widget, Kopete::MetaContact * mc, const QString &message, bool& suppress)
{
	KNotification *n=0L;
	//kdDebug( 14010 ) << k_funcinfo << endl;
	if ( suppress )
		return n;

	// Anything, including the MC itself, may set suppress and prevent further notifications

	/* This is a really ugly piece of logic now.  The idea is to check for notifications
	* first on the metacontact, then on each of its groups, until something suppresses
	* any further notifications.
	* So on the first run round this loop, dataObj points to the metacontact, and on subsequent
	* iterations it points to one of the contact's groups.  The metacontact pointer is maintained
	* so that if a group has a chat notification set for this event, we can call execute() on the MC.
	*/

	bool checkingMetaContact = true;
	Kopete::NotifyDataObject * dataObj = mc;
	do {
		QString sound;
		QString text;

		if ( dataObj )
		{
			Kopete::NotifyEvent *evt = dataObj->notifyEvent( message );
			if ( evt )
			{
				suppress = evt->suppressCommon();
				int present = 0;
				// sound
				Kopete::EventPresentation *pres = evt->presentation( Kopete::EventPresentation::Sound );
				if ( pres && pres->enabled() )
				{
					present = present | KNotifyClient::Sound;
					sound = pres->content();
					evt->firePresentation( Kopete::EventPresentation::Sound );
				}
				// message
				if ( ( pres = evt->presentation( Kopete::EventPresentation::Message ) )
								   && pres->enabled() )
				{
					present = present | KNotifyClient::PassivePopup;
					text = pres->content();
					evt->firePresentation( Kopete::EventPresentation::Message );
				}
				// chat
				if ( ( pres = evt->presentation( Kopete::EventPresentation::Chat ) )
								   && pres->enabled() )
				{
					if ( mc )
						mc->execute();
					evt->firePresentation( Kopete::EventPresentation::Chat );
				}
				// fire the event
				n=KNotification::userEvent( text, QPixmap::fromImage( mc->photo() ), widget, QStringList(),
                                            present, 0, sound, QString::null, QString::null , KNotification::CloseOnTimeout);
			}
		}

		if ( mc )
		{
			if ( checkingMetaContact )
			{
				// only executed on first iteration
				checkingMetaContact = false;
				dataObj = mc->groups().first();
			}
			else
				dataObj = mc->groups().next();
		}
	}
	while ( dataObj && !suppress );
	return n;
}


#endif



int KNotification::getPresentation(const QString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;

	KConfig eventsfile( kapp->instanceName()+".eventsrc", true, false);
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("presentation", -1);

	return present;
}

QString KNotification::getFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile( kapp->instanceName()+".eventsrc", true, false);
	eventsfile.setGroup(eventname);

	switch (present)
	{
		case (Sound):
			return eventsfile.readPathEntry("soundfile");
		case (Logfile):
			return eventsfile.readPathEntry("logfile");
	}

	return QString::null;
}

int KNotification::getDefaultPresentation(const QString &eventname)
{
	int present;
	if (eventname.isEmpty()) return Default;

	KConfig eventsfile( kapp->instanceName()+"/eventsrc", true, false, "data");
	eventsfile.setGroup(eventname);

	present=eventsfile.readNumEntry("default_presentation", -1);

	return present;
}

QString KNotification::getDefaultFile(const QString &eventname, int present)
{
	if (eventname.isEmpty()) return QString::null;

	KConfig eventsfile( kapp->instanceName()+"/eventsrc", true, false, "data");
	eventsfile.setGroup(eventname);

	switch (present)
	{
		case (Sound):
			return eventsfile.readPathEntry("default_sound");
		case (Logfile):
			return eventsfile.readPathEntry("default_logfile");
	}

	return QString::null;
}



#include "knotification.moc"



