/*
    kopete.h

    Kopete Instant Messenger Main Class

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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

class KopeteWindow;
class QSessionManager;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class Kopete : public KUniqueApplication
{
	Q_OBJECT

public:
	Kopete();
	~Kopete();
	/**
	 * Method to return whether or not we're shutting down
	 * or not at this point.
	 */
	 bool isShuttingDown() { return m_isShuttingDown; }
	 virtual void commitData( QSessionManager &sm );

private slots:
	/**
	 * Load all plugins
	 */
	void slotLoadPlugins();

	/**
	 * The main window got deleted
	 */
	void slotMainWindowDestroyed();

private:
	KopeteWindow *m_mainWindow;
	bool m_isShuttingDown;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

