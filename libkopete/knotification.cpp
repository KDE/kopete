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

#include "knotification.h"

#include <kdebug.h>
#include <kapplication.h>
#include <knotifyclient.h>
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


#include <qvbox.h>
#include <dcopclient.h>
#include <qcstring.h>
#include <qguardedptr.h>
#include <qstylesheet.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qtabwidget.h>



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
};

KNotification::KNotification(QObject *parent) :
		QObject(parent) , d(new Private)
{
	m_linkClicked = false;
}

KNotification::~KNotification() 
{
	delete d;
}


void KNotification::notifyByExecute(const QString &command, const QString& event,
							  const QString& fromApp, const QString& text,
							  int winId, int eventId)
{
	if (!command.isEmpty())
	{
	// kdDebug() << "executing command '" << command << "'" << endl;
		QMap<QChar,QString> subst;
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
//		return true;
	}
	//return false;
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
			case KNotifyClient::Notification:
				KMessageBox::informationWId( winId, d->text, i18n( "Notification" ) );
				break;
			case KNotifyClient::Warning:
				KMessageBox::sorryWId( winId, d->text, i18n( "Warning" ) );
				break;
			case KNotifyClient::Error:
				KMessageBox::errorWId( winId, d->text, i18n( "Error" ) );
				break;
			case KNotifyClient::Catastrophe:
				KMessageBox::errorWId( winId, d->text, i18n( "Fatal" ) );
				break;
		}
	}
	else
	{ //we may show the specific action button
		int result=0;
		QGuardedPtr<KNotification> _this=this; //this can be deleted
		switch( d->level )
		{
			default:
			case KNotifyClient::Notification:
				result = KMessageBox::questionYesNo(d->widget, d->text, i18n( "Notification" ), action, KStdGuiItem::cancel(), QString::null, false );
				break;
			case KNotifyClient::Warning:
				result = KMessageBox::warningYesNo( d->widget, d->text, i18n( "Warning" ), action, KStdGuiItem::cancel(), QString::null, false );
				break;
			case KNotifyClient::Error:
				result = KMessageBox::warningYesNo( d->widget, d->text, i18n( "Error" ), action, KStdGuiItem::cancel(), QString::null, false );
				break;
			case KNotifyClient::Catastrophe:
				result = KMessageBox::warningYesNo( d->widget, d->text, i18n( "Fatal" ), action, KStdGuiItem::cancel(), QString::null, false );
				break;
		}
		if(result==KMessageBox::Yes && _this)
		{
			activate(0);
		}
	}
}



void KNotification::notifyByPassivePopup(const QPixmap &pix )
{
	QString appName = QString::fromAscii( KNotifyClient::instance()->instanceName() );
	KIconLoader iconLoader( appName );
	KConfig eventsFile( QString::fromAscii( KNotifyClient::instance()->instanceName()+"/eventsrc" ), true, false, "data");
	KConfigGroup config( &eventsFile, "!Global!" );
	QString iconName = config.readEntry( "IconName", appName );
	QPixmap icon = iconLoader.loadIcon( iconName, KIcon::Small );
	QString title = config.readEntry( "Comment", appName );
    //KPassivePopup::message(title, text, icon, senderWinId);

	WId winId=d->widget ? d->widget->topLevelWidget()->winId()  : 0;
	
	KPassivePopup *pop = new KPassivePopup( checkWinId(appName, winId) );
	QObject::connect(this, SIGNAL(closed()), pop, SLOT(deleteLater()));

	QVBox *vb = pop->standardView( title, pix.isNull() ? d->text: QString::null , icon );
	QVBox *vb2=vb;

	if(!pix.isNull())
	{
		QHBox *hb = new QHBox(vb);
		hb->setSpacing(KDialog::spacingHint());
		QLabel *pil=new QLabel(hb);
		pil->setPixmap(pix);
		pil->setScaledContents(true);
		if(pix.height() > 80 && pix.height() > pix.width() )
		{
			pil->setMaximumHeight(80);
			pil->setMaximumWidth(80*pix.width()/pix.height());
		}
		else if(pix.width() > 80 && pix.height() <= pix.width())
		{
			pil->setMaximumWidth(80);
			pil->setMaximumHeight(80*pix.height()/pix.width());
		}
		vb=new QVBox(hb);
		QLabel *msg = new QLabel( d->text, vb, "msg_label" );
		msg->setAlignment( AlignLeft );
	}


	if ( !d->actions.isEmpty() )
	{
		QString linkCode=QString::fromLatin1("<p align=\"right\">");
		int i=0;
		for ( QStringList::ConstIterator it = d->actions.begin() ; it != d->actions.end(); ++it )
		{
			i++;
			linkCode+=QString::fromLatin1("&nbsp;<a href=\"%1\">%2</a> ").arg( QString::number(i) , QStyleSheet::escape(*it)  );
		}
		linkCode+=QString::fromLatin1("</p>");
		KActiveLabel *link = new KActiveLabel(linkCode , vb );
		//link->setAlignment( AlignRight );
		QObject::disconnect(link, SIGNAL(linkClicked(const QString &)), link, SLOT(openLink(const QString &)));
		QObject::connect(link, SIGNAL(linkClicked(const QString &)), this, SLOT(slotPopupLinkClicked(const QString &)));
		QObject::connect(link, SIGNAL(linkClicked(const QString &)), pop, SLOT(hide()));
	}

	pop->setAutoDelete( true );
	//pop->setTimeout(-1);

	pop->setView( vb2 );
	pop->show();

}

void KNotification::slotPopupLinkClicked(const QString &adr)
{
	m_linkClicked = true;
	unsigned int action=adr.toUInt();
	if(action==0)
		return;

	activate(action);

	// since we've hidden the message (KNotification::notifyByPassivePopup(const QPixmap &pix ))
	// we must now schedule overselves for deletion
	close();
}

void KNotification::activate(unsigned int action)
{
	if(action==0)
		emit activated();

	emit activated(action);
	deleteLater();
}


void KNotification::close()
{
	// if the user hasn't clicked the link, and if we got here, it means the dialog closed
	// and we were ignored
	if (!m_linkClicked)
	{
		emit ignored();
	}

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
			const QPixmap& pixmap, QWidget *widget,
			const QStringList &actions, unsigned int flags)
{
	/* NOTE:  this function still use the KNotifyClient,
	 *        in the future (KDE4) all the function of the knotifyclient will be moved there.
	 *  Some code here is derived from the old KNotify deamon
	 */

	int level=KNotifyClient::Default;
	QString sound;
	QString file;
	QString commandline;

	// get config file
	KConfig eventsFile( QString::fromAscii( KNotifyClient::instance()->instanceName()+"/eventsrc" ), true, false, "data");
	eventsFile.setGroup(message);

	KConfig configFile( QString::fromAscii( KNotifyClient::instance()->instanceName()+".eventsrc" ), true, false);
	configFile.setGroup(message);

	int present=KNotifyClient::getPresentation(message);
	if(present==-1)
		present=KNotifyClient::getDefaultPresentation(message);
	if(present==-1)
		present=0;

	// get sound file name
	if( present & KNotifyClient::Sound ) {
		QString theSound = configFile.readPathEntry( "soundfile" );
		if ( theSound.isEmpty() )
			theSound = eventsFile.readPathEntry( "default_sound" );
		if ( !theSound.isEmpty() )
			sound = theSound;
	}

	// get log file name
	if( present & KNotifyClient::Logfile ) {
		QString theFile = configFile.readPathEntry( "logfile" );
		if ( theFile.isEmpty() )
			theFile = eventsFile.readPathEntry( "default_logfile" );
		if ( !theFile.isEmpty() )
			file = theFile;
	}

	// get default event level
	if( present & KNotifyClient::Messagebox )
		level = eventsFile.readNumEntry( "level", 0 );

	// get command line
	if (present & KNotifyClient::Execute ) {
		commandline = configFile.readPathEntry( "commandline" );
		if ( commandline.isEmpty() )
			commandline = eventsFile.readPathEntry( "default_commandline" );
	}

	return userEvent( text, pixmap, widget, actions,  present , level, sound, file, commandline, flags );
}

KNotification *KNotification::userEvent( const QString& text, const QPixmap& pixmap, QWidget *widget,
				QStringList actions,int present, int level, const QString &sound, const QString &file,
				const QString &commandline, unsigned int flags)
{

	/* NOTE:  this function still use the KNotifyClient,
	 *        in the futur (KDE4) all the function of the knotifyclient will be moved there.
	 *  Some code of this function fome from the old KNotify deamon
	 */

	
	KNotification *notify=new KNotification(widget);
	notify->d->widget=widget;
	notify->d->text=text;
	notify->d->actions=actions;
	notify->d->level=level;
	WId winId=widget ? widget->topLevelWidget()->winId()  : 0;
	
	
	//we will catch some event that will not be fired by the old deamon
	

	//we remove presentation that has been already be played, and we fire the event in the old way
	
	
	KNotifyClient::userEvent(winId,text,present & ~( KNotifyClient::PassivePopup|KNotifyClient::Messagebox|KNotifyClient::Execute),level,sound,file);


	if ( present & KNotifyClient::PassivePopup )
	{
		notify->notifyByPassivePopup( pixmap );
	}
	if ( present & KNotifyClient::Messagebox )
	{
		QTimer::singleShot(0,notify,SLOT(notifyByMessagebox()));
	}
	else  //not a message box  (because closing the event when a message box is there is suicide)
		if(flags & CloseOnTimeout)
	{
		QTimer::singleShot(6*1000, notify, SLOT(close()));
	}
	if ( present & KNotifyClient::Execute )
	{
		QString appname = QString::fromAscii( KNotifyClient::instance()->instanceName() );
		notify->notifyByExecute(commandline, QString::null,appname,text, winId, 0 );
	}

	return notify;
	
}



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
					mc->execute();
					evt->firePresentation( Kopete::EventPresentation::Chat );
				}
				// fire the event
				n=KNotification::userEvent( text, mc->photo(), widget, QStringList() , present, 0, sound, QString::null, QString::null , KNotification::CloseOnTimeout);
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




KNotification *KNotification::event( Kopete::MetaContact *mc, const QString& message ,
			const QString& text, const QPixmap& pixmap, QWidget *widget,
			const QStringList &actions, unsigned int flags)
{
	if (message.isEmpty()) return 0;
    
	bool suppress = false;
	KNotification *n=performCustomNotifications( widget, mc, message, suppress);
		 
	if ( suppress )
	{
		//kdDebug( 14000 ) << "suppressing common notifications" << endl;
		return n; // custom notifications don't create a single unique id
	}
	else
	{
		//kdDebug( 14000 ) << "carrying out common notifications" << endl;
		return event(  message, text, pixmap, widget , actions, flags);
	}
}





#include "knotification.moc"



