/*
    kopeteaway.cpp  -  Kopete Away

    Copyright (c) 2002      by Hendrik vom Lehn       <hvl@linux-4-ever.de>
    Copyright (c) 2003      Olivier Goffart           <ogoffart@kde.org>

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

#include <config-kopete.h>

#include "kopeteaway.h"

#include <QTimer>
#include <QTime>

#include <kconfig.h>
#include <kapplication.h>
#include <QtDBus/QtDBus>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <ksettings/dispatcher.h>

#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopetebehaviorsettings.h"
#include "kopeteonlinestatus.h"
#include "kopeteonlinestatusmanager.h"
#include "kopetecontact.h"

//Included here so the file is generated at all, it will be installed
#include "ui_kopeteawaydialogbase.h"

#ifdef Q_WS_X11

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
// The following include is to make --enable-final work
#include <X11/Xutil.h>

#ifdef HAVE_XSCREENSAVER
#define HasScreenSaver
#include <X11/extensions/scrnsaver.h>
#include <QX11Info>
#endif
#endif // Q_WS_X11

// As this is an untested X extension we better leave it off
#undef HAVE_XIDLE
#undef HasXidle


struct KopeteAwayPrivate
{
	QString awayMessage;
	QString autoAwayMessage;
	bool useAutoAwayMessage;
	bool globalAway;
	QStringList awayMessageList;
	QTime idleTime;
	QTimer *timer;
	bool autoaway;
	bool goAvailable;
	int awayTimeout;
	bool useAutoAway;
	QList<Kopete::Account*> autoAwayAccounts;

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

Kopete::Away::Away() : QObject( kapp )
{
	setObjectName( "Kopete::Away" );

	int dummy = 0;
	dummy = dummy; // shut up

	d = new KopeteAwayPrivate;

	// Set up the away messages
	d->awayMessage.clear();
	d->autoAwayMessage.clear();
	d->useAutoAwayMessage = false;
	d->globalAway = false;
	d->autoaway = false;
	d->useAutoAway = true;

	// Empty the list
	d->awayMessageList.clear();

	// set the XAutoLock info
#ifdef Q_WS_X11
	Display *dsp = QX11Info::display();
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
	d->useXidle = XidleQueryExtension(QX11Info::display(), &dummy, &dummy);
#endif
#ifdef HasScreenSaver
	if(!d->useXidle)
		d->useMit = XScreenSaverQueryExtension(QX11Info::display(), &dummy, &dummy);
#endif
#ifdef Q_WS_X11
	d->xIdleTime = 0;
#endif
	kDebug(14010) << k_funcinfo << "Idle detection methods:" << endl;
	kDebug(14010) << k_funcinfo << "\tKScreensaverIface::isBlanked()" << endl;
#ifdef Q_WS_X11
	kDebug(14010) << k_funcinfo << "\tX11 XQueryPointer()" << endl;
#endif
	if (d->useXidle)
	{
		kDebug(14010) << k_funcinfo << "\tX11 Xidle extension" << endl;
	}
	if (d->useMit)
	{
		kDebug(14010) << k_funcinfo << "\tX11 MIT Screensaver extension" << endl;
	}

	load();
	KSettings::Dispatcher::self()->registerComponent( KGlobal::mainComponent(), this, SLOT( load() ) );
	// Set up the config object, and load the saved away messages
	KConfigGroup config( KGlobal::config(), "Away Messages" );

	// Away Messages
	if(config.hasKey("Messages"))
	{
		d->awayMessageList = config.readEntry("Messages", QStringList());
	}
	else
	{
		d->awayMessageList.append( i18n( "Sorry, I am busy right now" ) );
		d->awayMessageList.append( i18n( "I am gone right now, but I will be back later" ) );

		/* Save this list to disk */
		save();
	}

	d->autoAwayMessage = Kopete::BehaviorSettings::self()->autoAwayCustomMessage();

	// init the timer
	d->timer = new QTimer(this);
	d->timer->setObjectName("AwayTimer");

	connect(d->timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));
	d->timer->start(4000);

	//init the time and other
	setActive();
}

Kopete::Away::~Away()
{
	if(this == instance)
		instance = 0L;
	delete d;
}

QString Kopete::Away::message()
{
	return getInstance()->d->awayMessage;
}

QString Kopete::Away::autoAwayMessage()
{
	return getInstance()->d->autoAwayMessage;
}

void Kopete::Away::setGlobalAwayMessage(const QString &message)
{
	if( !message.isEmpty() )
	{
		kDebug(14010) << k_funcinfo <<
			"Setting global away message: " << message << endl;
		d->awayMessage = message;
	}
}

void Kopete::Away::setAutoAwayMessage(const QString &message)
{
	if( !message.isEmpty() )
	{
		kDebug(14010) << k_funcinfo <<
			"Setting auto away message: " << message << endl;
		d->autoAwayMessage = message;

		// Save the new auto away message to disk
		save();
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
	KConfigGroup config( KGlobal::config(), "Away Messages" );
	/* Set the away message settings in the Away Messages config group */
	config.writeEntry("Messages", d->awayMessageList);
	Kopete::BehaviorSettings::self()->setAutoAwayCustomMessage( d->autoAwayMessage );
	config.sync();

	emit( messagesChanged() );
}

void Kopete::Away::load()
{
	d->awayTimeout=Kopete::BehaviorSettings::self()->autoAwayTimeout();
	d->goAvailable=Kopete::BehaviorSettings::self()->autoAwayGoAvailable();
	d->useAutoAway=Kopete::BehaviorSettings::self()->useAutoAway();
	d->useAutoAwayMessage=Kopete::BehaviorSettings::self()->useCustomAwayMessage();
}

QStringList Kopete::Away::getMessages()
{
	return d->awayMessageList;
}

QString Kopete::Away::getMessage( uint messageNumber )
{
	if ( messageNumber < (unsigned int)d->awayMessageList.size() )
	{
		QString msg = d->awayMessageList.takeAt( messageNumber );
		d->awayMessageList.prepend( msg );
		save();
		return msg;
	}
	else
		return QString();
}

void Kopete::Away::addMessage(const QString &message)
{
	d->awayMessageList.prepend( message );
	if( (int)d->awayMessageList.count() > Kopete::BehaviorSettings::self()->awayMessageRemembered() )
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
	// Time to check whether we're active or autoaway.  We basically have two
	// bits of info to go on - KDE's screensaver status
	// (KScreenSaverIface::isBlanked()) and the X11 activity detection.
	//
	// Note that isBlanked() is a slight of a misnomer.  It returns true if we're:
	//  - using a non-locking screensaver, which is running, or
	//  - using a locking screensaver which is still locked, regardless of
	//    whether the user is trying to unlock it right now
	// Either way, it's only worth checking for activity if the screensaver
	// isn't blanked/locked, because activity while blanked is impossible and
	// activity while locked never matters (if there is any, it's probably just
	// the cleaner wiping the keyboard :).


	/* we should be able to respond to KDesktop queries to avoid a deadlock, so we allow the event loop to be called */
	static bool rentrency_protection=false;
	if(rentrency_protection)
		return;
	rentrency_protection=true;
#ifdef __GNUC__
#warning verify dcop call
#endif
	QDBusInterface caller("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
    //QDBusReply<bool> reply = caller.call("isBlanked");
    QDBusReply<bool> reply = caller.call("GetActive");
	if ( !reply.isValid() || !reply.value())
	{
		// DCOP failed, or returned something odd, or the screensaver is
		// inactive, so check for activity the X11 way.  It's only worth
		// checking for autoaway if there's no activity, and because
		// Screensaver blanking/locking implies autoAway activation (see
		// KopeteIface::KopeteIface()), only worth checking autoAway when the
		// screensaver isn't running.
		if (isActivity())
		{
			setActive();
		}
		else if (!d->autoaway && d->useAutoAway && idleTime() > d->awayTimeout)
		{
			setAutoAway();
		}
	}
}

bool Kopete::Away::isActivity()
{
	// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
	//
	// KDE screensaver engine
	//
	// This module is a heavily modified xautolock.
	// In fact as of KDE 2.0 this code is practically unrecognisable as xautolock.

	bool activity = false;

#ifdef Q_WS_X11
	Display *dsp = QX11Info::display();
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

	// =================================================================================

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

	// Only check idle time if we have some way of measuring it, otherwise if
	// we've neither Mit nor Xidle it'll still be zero and we'll always appear active.
	// FIXME: what problem does the 2000ms fudge solve?
	if (root_x != d->mouse_x || root_y != d->mouse_y || mask != d->mouse_mask
		|| ((d->useXidle || d->useMit) && xIdleTime < d->xIdleTime + 2000))
	{
		// -1 => just gone autoaway, ignore apparent activity this time round
		// anything else => genuine activity
		// See setAutoAway().
		if (d->mouse_x != -1)
		{
			activity = true;
		}
		d->mouse_x = root_x;
		d->mouse_y = root_y;
		d->mouse_mask = mask;
		d->xIdleTime = xIdleTime;
	}
#endif // Q_WS_X11
	// =================================================================================

	return activity;
}

void Kopete::Away::setActive()
{
//	kDebug(14010) << k_funcinfo << "Found activity on desktop, resetting away timer" << endl;
	d->idleTime.start();

	if(d->autoaway)
	{
		d->autoaway = false;
		emit activity();
		if (d->goAvailable)
		{
			QList<Kopete::Account*>::iterator it, itEnd = d->autoAwayAccounts.end();
			for( it = d->autoAwayAccounts.begin(); it != itEnd; ++it )
			{
				Kopete::Account* i = (*it);
				if(i->isConnected() && i->isAway())
				{
					i->setOnlineStatus( Kopete::OnlineStatusManager::self()->onlineStatus( i->protocol() ,
										Kopete::OnlineStatusManager::Online ) );
				}

				// removeAll() makes the next entry in the list the current one,
				// that's why we use current() above
				d->autoAwayAccounts.removeAll( i );
			}
		}
	}
}

void Kopete::Away::setAutoAway()
{
	// A value of -1 in mouse_x indicates to checkActivity() that next time it
	// fires it should ignore any apparent idle/mouse/keyboard changes.
	// I think the point of this is that if you manually start the screensaver
	// then there'll unavoidably be some residual mouse/keyboard activity
	// that should be ignored.
	d->mouse_x = -1;

//	kDebug(14010) << k_funcinfo << "Going AutoAway!" << endl;
	d->autoaway = true;

	// Set all accounts that are not away already to away.
	// We remember them so later we only set the accounts to
	// available that we set to away (and not the user).
	for(QListIterator<Kopete::Account *> it( Kopete::AccountManager::self()->accounts() ); it.hasNext(); )
	{
		Kopete::Account *i= it.next();
		if(i->myself()->onlineStatus().status() == Kopete::OnlineStatus::Online)
		{
			d->autoAwayAccounts.append(i);

			if(d->useAutoAwayMessage)
			{
			// Display a specific away message
			i->setOnlineStatus( Kopete::OnlineStatusManager::self()->onlineStatus( i->protocol() ,
				Kopete::OnlineStatusManager::Idle ) ,
								getInstance()->d->autoAwayMessage);
			}
			else
			{
			// Display the last away message used
			i->setOnlineStatus( Kopete::OnlineStatusManager::self()->onlineStatus( i->protocol() ,
				Kopete::OnlineStatusManager::Idle ) ,
								getInstance()->d->awayMessage);
			}
		}
	}
}

#include "kopeteaway.moc"
// vim: set et ts=4 sts=4 sw=4:
