/*
    kopeteaway.cpp  -  Kopete Away

    Copyright (c) 2002      by Hendrik vom Lehn       <hvl@linux-4-ever.de>
    Copyright (c) 2003      Olivier Goffart           <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kopeteaway.h"

#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopeteonlinestatus.h"
#include "kopeteonlinestatusmanager.h"
#include "kopetecontact.h"
#include "kopeteprefs.h"

#include <kconfig.h>
#include <qtimer.h>
#include <kapplication.h>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <ksettings/dispatcher.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
// The following include is to make --enable-final work
#include <X11/Xutil.h>

#ifdef HAVE_XSCREENSAVER
#define HasScreenSaver
#include <X11/extensions/scrnsaver.h>
#endif
#endif // Q_WS_X11

// As this is an untested X extension we better leave it off
#undef HAVE_XIDLE
#undef HasXidle


struct KopeteAwayPrivate
{
	QString awayMessage;
	bool globalAway;
	QStringList awayMessageList;
	QTime idleTime;
	QTimer *timer;
	bool autoaway;
	bool goAvailable;
	int awayTimeout;
	bool useAutoAway;
	QPtrList<Kopete::Account> autoAwayAccounts;

	int mouse_x;
	int mouse_y;
	unsigned int mouse_mask;
#ifdef Q_WS_X11
	Window    root;               /* root window the pointer is on */
	Screen*   screen;             /* screen the pointer is on      */

	Time xIdleTime;
#endif
	bool useXidle;
	bool useMit;
};

Kopete::Away *Kopete::Away::instance = 0L;

Kopete::Away::Away() : QObject( kapp , "Kopete::Away")
{
	int dummy = 0;
	dummy = dummy; // shut up

	d = new KopeteAwayPrivate;

	// Set up the away messages
	d->awayMessage = QString::null;
	d->globalAway = false;
	d->autoaway = false;
	d->useAutoAway = true;

	// Empty the list
	d->awayMessageList.clear();

	// set the XAutoLock info
#ifdef Q_WS_X11
	Display *dsp = qt_xdisplay();
#endif
	d->mouse_x = d->mouse_y=0;
	d->mouse_mask = 0;
#ifdef Q_WS_X11
	d->root = DefaultRootWindow (dsp);
	d->screen = ScreenOfDisplay (dsp, DefaultScreen (dsp));
#endif
	d->useXidle = false;
	d->useMit = false;
#ifdef HasXidle
	d->useXidle = XidleQueryExtension(qt_xdisplay(), &dummy, &dummy);
#endif
#ifdef HasScreenSaver
	if(!d->useXidle)
		d->useMit = XScreenSaverQueryExtension(qt_xdisplay(), &dummy, &dummy);
#endif
#ifdef Q_WS_X11
	d->xIdleTime = 0;
#endif
	if (d->useXidle)
		kdDebug(14010) << "using X11 Xidle extension" << endl;
	if(d->useMit)
		kdDebug(14010) << "using X11 MIT Screensaver extension" << endl;


	load();
	KSettings::Dispatcher::self()->registerInstance( KGlobal::instance(), this, SLOT( load() ) );
	// Set up the config object
	KConfig *config = KGlobal::config();
	/* Load the saved away messages */
	config->setGroup("Away Messages");

	if(config->hasKey("Messages"))
	{
		d->awayMessageList = config->readListEntry("Messages");
	}
	else if(config->hasKey("Titles"))  // Old config format
	{
		QStringList titles = config->readListEntry("Titles");  // Get the titles
		for(QStringList::iterator i = titles.begin(); i != titles.end(); ++i)
		{
			d->awayMessageList.append( config->readEntry(*i) ); // And add it to the list
		}

		/* Save this list to disk */
		save();
	}
	else
	{
		d->awayMessageList.append( i18n( "Sorry, I am busy right now" ) );
		d->awayMessageList.append( i18n( "I am gone right now, but I will be back later" ) );

		/* Save this list to disk */
		save();
	}

	// init the timer
	d->timer = new QTimer(this, "AwayTimer");
	connect(d->timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));
	d->timer->start(4000);

	//init the time and other
	setActivity();
}

Kopete::Away::~Away()
{
	delete d;
}

QString Kopete::Away::message()
{
	return getInstance()->d->awayMessage;
}

void Kopete::Away::setGlobalAwayMessage(const QString &message)
{
	if( !message.isEmpty() )
	{
		kdDebug(14010) << k_funcinfo <<
			"Setting global away message: " << message << endl;
		d->awayMessage = message;
	}
}

Kopete::Away *Kopete::Away::getInstance()
{
	if (!instance)
		instance = new Kopete::Away;
	return instance;
}

bool Kopete::Away::globalAway()
{
	return getInstance()->d->globalAway;
}

void Kopete::Away::setGlobalAway(bool status)
{
	getInstance()->d->globalAway = status;
}

void Kopete::Away::save()
{
	KConfig *config = KGlobal::config();
	/* Set the away message settings in the Away Messages config group */
	config->setGroup("Away Messages");
	config->writeEntry("Messages", d->awayMessageList);
	config->sync();

	emit( messagesChanged() );
}

void Kopete::Away::load()
{
	KConfig *config = KGlobal::config();
	config->setGroup("AutoAway");
	d->awayTimeout=config->readNumEntry("Timeout", 600);
	d->goAvailable=config->readBoolEntry("GoAvailable", true);
	d->useAutoAway=config->readBoolEntry("UseAutoAway", true);
}

QStringList Kopete::Away::getMessages()
{
	return d->awayMessageList;
}

QString Kopete::Away::getMessage( uint messageNumber )
{
	QStringList::iterator it = d->awayMessageList.at( messageNumber );
	if( it != d->awayMessageList.end() )
	{
		QString str = *it;
		d->awayMessageList.prepend( str );
		d->awayMessageList.remove( it );
		save();
		return str;
	}
	else
	{
		return QString::null;
	}
}

void Kopete::Away::addMessage(const QString &message)
{
	d->awayMessageList.prepend( message );
	if( (int)d->awayMessageList.count() > KopetePrefs::prefs()->rememberedMessages() )
		d->awayMessageList.pop_back();
	save();
}

long int Kopete::Away::idleTime()
{
	//FIXME: the time is reset to zero if more than 24 hours are elapsed
	// we can imagine someone who leave his PC for several weeks
	return (d->idleTime.elapsed() / 1000);
}

void Kopete::Away::slotTimerTimeout()
{
	// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
	//
	// KDE screensaver engine
	//
	// This module is a heavily modified xautolock.
	// In fact as of KDE 2.0 this code is practically unrecognisable as xautolock.

#ifdef Q_WS_X11
	Display *dsp = qt_xdisplay();
	Window           dummy_w;
	int              dummy_c;
	unsigned int     mask;               /* modifier mask                 */
	int              root_x;
	int              root_y;

	/*
	*  Find out whether the pointer has moved. Using XQueryPointer for this
	*  is gross, but it also is the only way never to mess up propagation
	*  of pointer events.
	*
	*  Remark : Unlike XNextEvent(), XPending () doesn't notice if the
	*           connection to the server is lost. For this reason, earlier
	*           versions of xautolock periodically called XNoOp (). But
	*           why not let XQueryPointer () do the job for us, since
	*           we now call that periodically anyway?
	*/
	if (!XQueryPointer (dsp, d->root, &(d->root), &dummy_w, &root_x, &root_y,
			&dummy_c, &dummy_c, &mask))
	{
		/*
		*  Pointer has moved to another screen, so let's find out which one.
		*/
		for (int i = 0; i < ScreenCount(dsp); i++)
		{
			if (d->root == RootWindow(dsp, i))
			{
				d->screen = ScreenOfDisplay (dsp, i);
				break;
			}
		}
	}
#endif
	// =================================================================================
#ifdef Q_WS_X11
	Time xIdleTime = 0; // millisecs since last input event

	#ifdef HasXidle
	if (d->useXidle)
	{
		XGetIdleTime(dsp, &xIdleTime);
	}
	else
	#endif /* HasXIdle */

	{
	#ifdef HasScreenSaver
		if(d->useMit)
		{
			static XScreenSaverInfo* mitInfo = 0;
			if (!mitInfo) mitInfo = XScreenSaverAllocInfo();
			XScreenSaverQueryInfo (dsp, d->root, mitInfo);
			xIdleTime = mitInfo->idle;
		}
	#endif /* HasScreenSaver */
	}

	// =================================================================================

	if (root_x != d->mouse_x || root_y != d->mouse_y || mask != d->mouse_mask || xIdleTime < d->xIdleTime+2000)
	{
		if(d->mouse_x!=-1) //we just gone autoaway, not activity this time
			setActivity();
		d->mouse_x = root_x;
		d->mouse_y = root_y;
		d->mouse_mask = mask;
		d->xIdleTime = xIdleTime;
	}
#endif // Q_WS_X11
	// =================================================================================

	if(!d->autoaway && d->useAutoAway && idleTime() > d->awayTimeout)
	{
		setAutoAway();
	}
}

void Kopete::Away::setActivity()
{
//	kdDebug(14010) << k_funcinfo << "Found activity on desktop, resetting away timer" << endl;
	d->idleTime.start();

	if(d->autoaway)
	{
		d->autoaway = false;
		emit activity();
		if (d->goAvailable)
		{
			d->autoAwayAccounts.setAutoDelete(false);
			for(Kopete::Account *i=d->autoAwayAccounts.first() ; i; i=d->autoAwayAccounts.current() )
			{
				if(i->isConnected() && i->isAway())
				{
					i->setOnlineStatus( Kopete::OnlineStatusManager::self()->onlineStatus( i->protocol() ,
										Kopete::OnlineStatusManager::Online ) ,
					getInstance()->d->awayMessage);	
				}

				// remove() makes the next entry in the list the current one,
				// that's why we use current() above
				d->autoAwayAccounts.remove();
			}
		}
	}
}

void Kopete::Away::setAutoAway()
{
	d->mouse_x=-1; //do not go availiable automaticaly after

//	kdDebug(14010) << k_funcinfo << "Going AutoAway!" << endl;
	d->autoaway = true;

	// Set all accounts that are not away already to away.
	// We remember them so later we only set the accounts to
	// available that we set to away (and not the user).
	QPtrList<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts();
	for(Kopete::Account *i=accounts.first() ; i; i=accounts.next()  )
	{
		if(i->myself()->onlineStatus().status() == Kopete::OnlineStatus::Online)
		{
			d->autoAwayAccounts.append(i);
			i->setOnlineStatus( Kopete::OnlineStatusManager::self()->onlineStatus( i->protocol() ,
													Kopete::OnlineStatusManager::Idle ) ,
								getInstance()->d->awayMessage);
		}
	}
}

#include "kopeteaway.moc"
// vim: set et ts=4 sts=4 sw=4:
