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

#ifndef KOPETEAPPLICATION_H
#define KOPETEAPPLICATION_H

#include <qpointer.h>

#include <kuniqueapplication.h>

class KopeteWindow;
class QSessionManager;

namespace Kopete
{
	class MimeTypeHandler;
	class FileEngineHandler;
}

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class KopeteApplication : public KUniqueApplication
{
	Q_OBJECT

public:
	KopeteApplication();
	~KopeteApplication();

	/**
	 * Method to return whether or not we're shutting down
	 * or not at this point.
	 */
	bool isShuttingDown() const { return m_isShuttingDown; }

	virtual int newInstance();

public slots:
	/**
	 * Quit Kopete, closing all the windows, which causes application shutdown
	 * This method marks Kopete as 'shutting down' to avoid
	 * showing the message box that Kopete will be left running in the
	 * system tray before calling qApp->quit().
	 */
	void quitKopete();

	virtual void commitData( QSessionManager &sm );
	/**
	 * Load all plugins
	 */
	void slotLoadPlugins();

private slots:
	/**
	 * auto-connect
	 */
	void slotAllPluginsLoaded();
private:
	// The main window might get deleted behind our back (W_DestructiveClose),
	// so use a guarded pointer
	QPointer<KopeteWindow> m_mainWindow;
	bool m_isShuttingDown;
	Kopete::MimeTypeHandler *m_emoticonHandler;
	Kopete::FileEngineHandler *m_fileEngineHandler;
private:
	void handleURLArgs();
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

