/*
    kopete.h

    Kopete Instant Messenger Main Class

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_H
#define KOPETE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kuniqueapplication.h>

#include "kopetemessage.h"

class AppearanceConfig;
class KopeteEvent;
class KopeteNotifier;
class KopeteUserPreferencesConfig;
class KopeteWindow;
class Plugins;
class PreferencesDialog;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class Kopete : public KUniqueApplication
{
	Q_OBJECT

public:
	Kopete();
	~Kopete();

public slots:
	/**
	 * Only use notify event for system-wide messages
	 * and things like online notification
	 * Messages from specific contacts should use KopeteContact's
	 * incomingEvent signal
	 */
	void notifyEvent( KopeteEvent * );

	/**
	 * Cancel an event.
	 */
	void cancelEvent( KopeteEvent * );

signals:
	/**
	 * This signal is emitted whenever a message
	 * is about to be displayed by the KopeteChatWindow.
	 * Please remember that both messages sent and
	 * messages received will emit this signal!
	 * Plugins may connect to this signal to change
	 * the message contents before it's going to be displayed.
	 */
	void aboutToDisplay( KopeteMessage& );

	/**
	 * Plugins may connect to this signal
	 * to manipulate the contents of the
	 * message that is being sent.
	 */
	void aboutToSend( KopeteMessage& );

	void signalSettingsChanged();

private slots:
	void slotAddContact();
	void slotShowTransfers();

	/**
	 * Load all plugins
	 */
	void slotLoadPlugins();

private:
	Plugins *mPluginsModule;

	KopeteWindow *m_mainWindow;
	AppearanceConfig *mAppearance;

	//User preferences config module
	KopeteUserPreferencesConfig *mUserPreferencesConfig;
	KopeteNotifier *mNotifier;
};

#define kopeteapp (static_cast<Kopete*>(kapp))

#endif

// vim: set noet ts=4 sts=4 sw=4:

